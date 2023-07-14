module;

#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

#include "onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"

module ufld:base;

import ufld.ld;

import :utils;

namespace ufld {

struct PreProcessResult {
  cv::Mat processed_image{};
  cv::Size original_size{};
  cv::Rect crop_area{};
  std::chrono::milliseconds duration{0};
};

struct InferenceResult {
  std::vector<Ort::Value> outputs{};
  std::chrono::milliseconds duration{0};
};

struct PostProcessResult {
  std::vector<Lane> lanes{};
  std::chrono::milliseconds duration{0};
};

class LaneDetectorBase : public ILaneDetector {
 public:
  virtual ~LaneDetectorBase() = default;

  LaneDetectorBase(const LaneDetectorBase&) = delete;
  LaneDetectorBase& operator=(const LaneDetectorBase&) = delete;

  LaneDetectorBase(LaneDetectorBase&&) = delete;
  LaneDetectorBase& operator=(LaneDetectorBase&&) = delete;

  /**
   *
   * @param image
   * @param preview
   * @return
   */
  [[nodiscard]] LaneDetectionResult Detect(
      const cv::Mat& image,
      std::optional<cv::Mat> preview = std::nullopt) override;

  /**
   *
   * @return
   */
  [[nodiscard]] std::filesystem::path ModelDirectory() const override;

  /**
   *
   * @return
   */
  [[nodiscard]] Version ModelVersion() const override;

  /**
   *
   * @return
   */
  [[nodiscard]] std::variant<v1::Variant> ModelVariant() const override;

 protected:
  /**
   *
   * @param model_path
   * @param version
   * @param variant
   */
  LaneDetectorBase(const std::filesystem::path& model_path, Version version,
                   std::variant<v1::Variant> variant);

  std::filesystem::path model_directory_{};
  std::string cache_directory_string_{};  // String for lifetime
  Version version_{};
  std::variant<v1::Variant> variant_{};

  Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "UFLD"};
  Ort::Session session_{nullptr};
  Ort::AllocatorWithDefaultOptions allocator_{};
  std::mutex detection_mutex_{};

  // Input (always 1 image)
  std::string input_name_{};
  std::vector<int64_t> input_dimensions_{};
  int64_t input_tensor_size_{};
  std::vector<float> input_tensor_data_{};

  // Output(s)
  std::vector<std::string> output_names_{};
  std::vector<std::vector<int64_t>> output_dimensions_{};
  std::vector<int64_t> output_tensor_sizes_{};
  std::vector<std::vector<float>> output_tensor_data_{};

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] virtual PreProcessResult PreProcess(
      const cv::Mat& image) const = 0;

  /**
   *
   * @param outputs
   * @param pre_process_result
   * @return
   */
  [[nodiscard]] virtual PostProcessResult PostProcess(
      const std::vector<Ort::Value>& outputs,
      const PreProcessResult& pre_process_result) const = 0;

  /**
   *
   * @param image
   * @param input_aspect_ratio
   * @return
   */
  [[nodiscard]] static cv::Rect ComputeCenterCrop(const cv::Mat& image,
                                                  float input_aspect_ratio);

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] static cv::Mat ColorPreProcess(const cv::Mat& image);

 private:
  /**
   *
   * @param model_path
   */
  void InitializeSession(const std::filesystem::path& model_path);

  /**
   *
   */
  void InitializeInput();

  /**
   *
   */
  void InitializeOutputs();

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] Ort::Value InitializeInputTensor(const cv::Mat& image);

  /**
   *
   * @return
   */
  [[nodiscard]] std::vector<Ort::Value> InitializeOutputTensors();

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] InferenceResult Inference(const cv::Mat& image);
};

}  // namespace ufld

// -------- Implementation --------

ufld::LaneDetectionResult ufld::LaneDetectorBase::Detect(
    const cv::Mat& image, std::optional<cv::Mat> preview) {
  std::lock_guard<std::mutex> lock{detection_mutex_};
  const auto pre_process_result = PreProcess(image);
  const auto inference_result = Inference(pre_process_result.processed_image);
  auto post_process_result =
      PostProcess(inference_result.outputs, pre_process_result);

  if (preview.has_value()) {
    ufld::utils::DrawLanes(post_process_result.lanes, preview.value());
    ufld::utils::DrawInputArea(pre_process_result.crop_area, preview.value());
  }

  ufld::LaneDetectionStatistics statistics{
      .pre_process_duration = pre_process_result.duration,
      .inference_duration = inference_result.duration,
      .post_process_duration = post_process_result.duration};

  return {.lanes = std::move(post_process_result.lanes),
          .preview = std::move(preview.value()),
          .statistics = std::move(statistics)};
}

std::filesystem::path ufld::LaneDetectorBase::ModelDirectory() const {
  return model_directory_;
}

ufld::Version ufld::LaneDetectorBase::ModelVersion() const {
  return version_;
}

std::variant<ufld::v1::Variant> ufld::LaneDetectorBase::ModelVariant() const {
  return variant_;
}

ufld::LaneDetectorBase::LaneDetectorBase(
    const std::filesystem::path& model_path, Version version,
    std::variant<v1::Variant> variant)
    : model_directory_{model_path.parent_path()},
      cache_directory_string_{(model_directory_ / "cache").string()},
      version_{version},
      variant_{variant} {
  InitializeSession(model_path);
  InitializeInput();
  InitializeOutputs();
}

cv::Rect ufld::LaneDetectorBase::ComputeCenterCrop(const cv::Mat& image,
                                                   float input_aspect_ratio) {
  const auto image_aspect_ratio =
      static_cast<float>(image.cols) / static_cast<float>(image.rows);
  if (image_aspect_ratio > input_aspect_ratio) {  // Crop in X
    //  |- -------------- -|
    //  |    | x    x |    |
    //  |    | x    x |    |
    //  |    | x    x |    |  <- Keep center
    //  |    | x    x |    |
    //  |    | x    x |    |
    //  |- -------------- -|
    const auto pixels_to_crop = static_cast<int32_t>(
        static_cast<float>(image.rows) * input_aspect_ratio);
    const auto x_offset = (image.cols - pixels_to_crop) / 2;
    cv::Rect crop_area{};
    crop_area.x = x_offset;
    crop_area.y = 0;
    crop_area.width = image.cols - 2 * x_offset;
    crop_area.height = image.rows;
    return crop_area;
  } else {  // Crop in Y
    // |- -------------- -|
    // |                  |
    // |------------------|
    // | x  x  x  x  x  x |  <- Keep center
    // |------------------|
    // |                  |
    // |- -------------- -|
    const auto pixels_to_crop = static_cast<int32_t>(
        static_cast<float>(image.cols) / input_aspect_ratio);
    const auto y_offset = (image.rows - pixels_to_crop) / 2;
    cv::Rect crop_area{};
    crop_area.x = 0;
    crop_area.y = y_offset;
    crop_area.width = image.cols;
    crop_area.height = image.rows - 2 * y_offset;
    return crop_area;
  }
}

cv::Mat ufld::LaneDetectorBase::ColorPreProcess(const cv::Mat& image) {
  cv::Mat converted_image;
  cv::cvtColor(image, converted_image, cv::COLOR_RGBA2RGB);
  converted_image.convertTo(converted_image, CV_32FC3, 1.0f / 255.0f);
  // ResNet
  const cv::Scalar mean{0.485, 0.456, 0.406};
  const cv::Scalar std{0.229, 0.224, 0.225};
  return (converted_image - mean) / std;
}

void ufld::LaneDetectorBase::InitializeSession(
    const std::filesystem::path& model_path) {
  if (!std::filesystem::exists(model_path))
    throw std::invalid_argument{"Model file does not exist: " +
                                model_path.string()};

  Ort::SessionOptions session_options{};
  session_options.SetIntraOpNumThreads(1);

  try {  // TensorRT
    OrtTensorRTProviderOptions tensorrt_options{};
    tensorrt_options.device_id = 0;
    // "TensorRT option trt_max_partition_iterations must be a positive integer value. Set it to 1000"
    tensorrt_options.trt_max_partition_iterations = 1000;
    // "TensorRT option trt_min_subgraph_size must be a positive integer value. Set it to 1"
    tensorrt_options.trt_min_subgraph_size = 1;
    // "TensorRT option trt_max_workspace_size must be a positive integer value. Set it to 1073741824 (1GB)"
    tensorrt_options.trt_max_workspace_size = 1073741824;
    // Enable DLA
    tensorrt_options.trt_dla_enable = 1;
    // Enable engine caching (note that cache must be cleared when ORT version, TensorRT or hardware changes)
    tensorrt_options.trt_engine_cache_enable = 1;
    tensorrt_options.trt_engine_cache_path = cache_directory_string_.c_str();
    session_options.AppendExecutionProvider_TensorRT(tensorrt_options);
  } catch (const Ort::Exception& e) {
    std::cerr << "TensorRT is not available: " << e.what() << std::endl;
  }

  try {  // CUDA
    OrtCUDAProviderOptions cuda_options{};
    cuda_options.device_id = 0;
    session_options.AppendExecutionProvider_CUDA(cuda_options);
  } catch (const Ort::Exception& e) {
    std::cerr << "CUDA is not available: " << e.what() << std::endl;
  }

  try {
    session_ = Ort::Session(env_, model_path.c_str(), session_options);
  } catch (const Ort::Exception& e) {
    std::cerr << "Failed to create OnnxRuntime session: " << e.what()
              << std::endl;
    throw;
  }
}

void ufld::LaneDetectorBase::InitializeInput() {
  assert(session_.GetInputCount() == 1);

  const auto allocated_input_name =
      session_.GetInputNameAllocated(0, allocator_);
  input_name_ = std::string{allocated_input_name.get()};

  Ort::TypeInfo input_type_info = session_.GetInputTypeInfo(0);
  assert(input_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
  input_dimensions_ = input_type_info.GetTensorTypeAndShapeInfo().GetShape();

  input_tensor_size_ =
      std::accumulate(input_dimensions_.begin(), input_dimensions_.end(), 1LL,
                      std::multiplies<>());
}

void ufld::LaneDetectorBase::InitializeOutputs() {
  for (auto output_index = 0; output_index < session_.GetOutputCount();
       ++output_index) {
    const auto allocated_output_name =
        session_.GetOutputNameAllocated(output_index, allocator_);
    output_names_.emplace_back(allocated_output_name.get());

    Ort::TypeInfo output_type_info = session_.GetOutputTypeInfo(output_index);
    assert(output_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
    output_dimensions_.emplace_back(
        output_type_info.GetTensorTypeAndShapeInfo().GetShape());

    const auto& output_dimensions = output_dimensions_.back();
    output_tensor_sizes_.emplace_back(
        std::accumulate(output_dimensions.begin(), output_dimensions.end(), 1LL,
                        std::multiplies<>()));
  }
}

Ort::Value ufld::LaneDetectorBase::InitializeInputTensor(const cv::Mat& image) {
  cv::Mat preprocessed_image;
  cv::dnn::blobFromImage(image, preprocessed_image);

  input_tensor_data_.resize(static_cast<size_t>(input_tensor_size_));
  std::copy(preprocessed_image.begin<float>(), preprocessed_image.end<float>(),
            input_tensor_data_.begin());

  Ort::MemoryInfo memory_info =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  return Ort::Value::CreateTensor<float>(
      memory_info, input_tensor_data_.data(), input_tensor_data_.size(),
      input_dimensions_.data(), input_dimensions_.size());
}

std::vector<Ort::Value> ufld::LaneDetectorBase::InitializeOutputTensors() {
  output_tensor_data_.clear();
  Ort::MemoryInfo memory_info =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::vector<Ort::Value> output_tensors;
  for (size_t output_index = 0; output_index < session_.GetOutputCount();
       ++output_index) {
    output_tensor_data_.emplace_back();
    auto& output_tensor_data = output_tensor_data_.back();
    output_tensor_data.resize(
        static_cast<size_t>(output_tensor_sizes_[output_index]));
    auto& output_dimensions = output_dimensions_[output_index];
    output_tensors.emplace_back(Ort::Value::CreateTensor<float>(
        memory_info, output_tensor_data.data(), output_tensor_data.size(),
        output_dimensions.data(), output_dimensions.size()));
  }
  return output_tensors;
}

ufld::InferenceResult ufld::LaneDetectorBase::Inference(const cv::Mat& image) {
  const auto start_time = std::chrono::high_resolution_clock::now();

  auto input_tensor = InitializeInputTensor(image);
  auto output_tensors = InitializeOutputTensors();

  const char* input_name = input_name_.c_str();

  // Ort::Session::Run expects an array of const char* for the output names
  auto OutputNamesToRaw = [&]() {
    std::vector<const char*> output_names;
    for (const auto& output_name : output_names_) {
      output_names.push_back(output_name.c_str());
    }
    return output_names;
  };

  session_.Run(Ort::RunOptions{nullptr}, &input_name, &input_tensor, 1,
               OutputNamesToRaw().data(), output_tensors.data(),
               output_tensors.size());

  ufld::InferenceResult result{};
  result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_time);
  result.outputs = std::move(output_tensors);
  return result;
}
