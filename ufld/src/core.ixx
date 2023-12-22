module;

#include <array>
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

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

export module ufld:core;

import :draw;

namespace ufld {
export template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
export template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

/**
 * Ultra-Fast-Lane-Detection version.
 */
export enum class Version { kV1 };

namespace v1 {
/**
 * Variant of Ultra-Fast-Lane-Detection V1:
 *    CULane -> ufld_v1_culane_288x800.onnx
 *    TuSimple -> ufld_v1_tusimple_288x800.onnx
 */
export enum class Variant { kCULane, kTuSimple };
}  // namespace v1

/**
 * Ultra-Fast-Lane-Detection settings.
 */
export struct Settings {
  std::filesystem::path model_directory{};
  Version model_version{Version::kV1};
  std::variant<v1::Variant> model_variant{v1::Variant::kCULane};
};

/**
 * A single lane (lane divider) is represented as a vector of points.
 */
export using Lane = std::vector<cv::Point2f>;

/**
 * Performance statistics for lane detection.
 */
export struct LaneDetectionStatistics {
  std::chrono::milliseconds pre_process_duration{0};
  std::chrono::milliseconds inference_duration{0};
  std::chrono::milliseconds post_process_duration{0};
};

/**
 * Lane detection result.
 */
export struct LaneDetectionResult {
  std::array<Lane, 4> lanes{};
  std::optional<cv::Mat> preview{std::nullopt};
  LaneDetectionStatistics statistics{};
};

struct ImageInfo {
  cv::Size size{};
  cv::Rect crop{};
};

struct PreProcessResult {
  cv::Mat image{};
  ImageInfo image_info{};
  std::chrono::milliseconds duration{0};
};

struct InferenceResult {
  std::vector<Ort::Value> outputs{};
  std::chrono::milliseconds duration{0};
};

struct PostProcessResult {
  std::array<Lane, 4> lanes{};
  std::chrono::milliseconds duration{0};
};

export template <class C>
class LaneDetector {
 public:
  [[nodiscard]] LaneDetectionResult Detect(cv::Mat&& image, bool preview = true);

  [[nodiscard]] std::filesystem::path ModelDirectory() const;
  [[nodiscard]] Version ModelVersion() const;
  [[nodiscard]] std::variant<v1::Variant> ModelVariant() const;

 protected:
  LaneDetector(std::filesystem::path model_path, Version model_version, std::variant<v1::Variant> model_variant);

  [[nodiscard]] PreProcessResult PreProcess(cv::Mat&& image) const;

  [[nodiscard]] InferenceResult Inference(const cv::Mat& image);

  [[nodiscard]] PostProcessResult PostProcess(std::vector<Ort::Value>&& outputs, const ImageInfo& image_info) const;

  static void ColorPreProcess(cv::Mat& image);
  [[nodiscard]] static std::chrono::milliseconds Duration(const std::chrono::high_resolution_clock::time_point& start,
                                                          const std::chrono::high_resolution_clock::time_point& end);

 private:
  std::filesystem::path model_directory_{};
  std::string cache_directory_string_{};  // String for lifetime
  Version model_version_{};
  std::variant<v1::Variant> model_variant_{};

  Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "UFLD"};
  Ort::Session session_{nullptr};
  Ort::AllocatorWithDefaultOptions allocator_{};

  // Input (always 1 image)
  std::string input_name_{};
  std::vector<std::int64_t> input_dimensions_{};
  std::int64_t input_tensor_size_{};

  // Output(s) (variable based on version and variant)
  std::vector<std::string> output_names_{};
  std::vector<std::vector<std::int64_t>> output_dimensions_{};
  std::vector<std::int64_t> output_tensor_sizes_{};

  [[nodiscard]] Ort::Value MakeInputTensor(cv::Mat& image) const;
  [[nodiscard]] std::vector<Ort::Value> MakeOutputTensors() const;

  C& Underlying() { return static_cast<C&>(*this); }
  const C& Underlying() const { return static_cast<const C&>(*this); }

  void InitializeSession(std::filesystem::path&& model_path);
  void InitializeInput();
  void InitializeOutputs();

  friend C;
};

template <class C>
LaneDetectionResult LaneDetector<C>::Detect(cv::Mat&& image, bool preview) {
  LaneDetectionResult result{};
  if (preview) {
    result.preview = image.clone();
  }
  auto pre_process_result = PreProcess(std::move(image));
  auto inference_result = Inference(pre_process_result.image);
  auto post_process_result = PostProcess(std::move(inference_result.outputs), pre_process_result.image_info);
  if (result.preview.has_value()) {
    draw::DrawLanes(post_process_result.lanes, result.preview.value());
    draw::DrawInputArea(pre_process_result.image_info.crop, result.preview.value());
  }
  result.lanes = post_process_result.lanes;
  result.statistics = {.pre_process_duration = pre_process_result.duration,
                       .inference_duration = inference_result.duration,
                       .post_process_duration = post_process_result.duration};
  return result;
}

template <class C>
std::filesystem::path LaneDetector<C>::ModelDirectory() const {
  return model_directory_;
}

template <class C>
Version LaneDetector<C>::ModelVersion() const {
  return model_version_;
}

template <class C>
std::variant<v1::Variant> LaneDetector<C>::ModelVariant() const {
  return model_variant_;
}

template <class C>
LaneDetector<C>::LaneDetector(std::filesystem::path model_path, Version model_version,
                              std::variant<v1::Variant> model_variant)

    : model_directory_{model_path.parent_path()},
      cache_directory_string_{(model_path.parent_path() / "cache").string()},
      model_version_{model_version},
      model_variant_{model_variant} {
  InitializeSession(std::move(model_path));
  InitializeInput();
  InitializeOutputs();
}

template <class C>
PreProcessResult LaneDetector<C>::PreProcess(cv::Mat&& image) const {
  return Underlying().PreProcess(std::move(image));
}

template <class C>
InferenceResult LaneDetector<C>::Inference(const cv::Mat& image) {
  const auto start_time = std::chrono::high_resolution_clock::now();

  cv::Mat input_image{};
  cv::dnn::blobFromImage(image, input_image);
  const auto input_tensor = MakeInputTensor(input_image);
  auto output_tensors = MakeOutputTensors();
  const char* input_name = input_name_.c_str();

  // Ort::Session::Run expects an array of const char* for the output names
  auto OutputNamesToRaw = [&]() {
    std::vector<const char*> output_names;
    for (const auto& output_name : output_names_) {
      output_names.push_back(output_name.c_str());
    }
    return output_names;
  };

  session_.Run(Ort::RunOptions{nullptr}, &input_name, &input_tensor, 1, OutputNamesToRaw().data(),
               output_tensors.data(), output_tensors.size());

  return {.outputs = std::move(output_tensors), .duration = Duration(start_time, std::chrono::high_resolution_clock::now())};
}

template <class C>
PostProcessResult LaneDetector<C>::PostProcess(std::vector<Ort::Value>&& outputs, const ImageInfo& image_info) const {
  return Underlying().PostProcess(std::move(outputs), image_info);
}

template <class C>
void LaneDetector<C>::ColorPreProcess(cv::Mat& image) {
  cv::cvtColor(image, image, cv::COLOR_RGBA2RGB);
  image.convertTo(image, CV_32FC3, 1.0f / 255.0f);
  // ResNet
  const cv::Scalar mean{0.485, 0.456, 0.406};
  const cv::Scalar std{0.229, 0.224, 0.225};
  image -= mean;
  image /= std;
}

template <class C>
std::chrono::milliseconds LaneDetector<C>::Duration(const std::chrono::high_resolution_clock::time_point& start,
                                                    const std::chrono::high_resolution_clock::time_point& end) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

template <class C>
Ort::Value LaneDetector<C>::MakeInputTensor(cv::Mat& image) const {
  const auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  return Ort::Value::CreateTensor<float>(memory_info, image.ptr<float>(), image.total(), input_dimensions_.data(),
                                         input_dimensions_.size());
}

template <class C>
std::vector<Ort::Value> LaneDetector<C>::MakeOutputTensors() const {
  const auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::vector<Ort::Value> output_tensors{};
  output_tensors.reserve(session_.GetOutputCount());
  for (std::size_t output_index = 0; output_index < session_.GetOutputCount(); ++output_index) {
    output_tensors.emplace_back(Ort::Value::CreateTensor<float>(allocator_, output_dimensions_[output_index].data(),
                                                                output_dimensions_[output_index].size()));
  }
  return output_tensors;
}

template <class C>
void LaneDetector<C>::InitializeSession(std::filesystem::path&& model_path) {
  if (!std::filesystem::exists(model_path))
    throw std::invalid_argument{"Model file does not exist: " + model_path.string()};

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
    std::cerr << "Failed to create OnnxRuntime session: " << e.what() << std::endl;
    throw;
  }
}

template <class C>
void LaneDetector<C>::InitializeInput() {
  const auto allocated_input_name = session_.GetInputNameAllocated(0, allocator_);
  input_name_ = std::string{allocated_input_name.get()};
  const auto input_type_info = session_.GetInputTypeInfo(0);
  input_dimensions_ = input_type_info.GetTensorTypeAndShapeInfo().GetShape();
  input_tensor_size_ = std::accumulate(input_dimensions_.begin(), input_dimensions_.end(), 1LL, std::multiplies());
}

template <class C>
void LaneDetector<C>::InitializeOutputs() {
  for (auto output_index = 0; output_index < session_.GetOutputCount(); ++output_index) {
    const auto allocated_output_name = session_.GetOutputNameAllocated(output_index, allocator_);
    output_names_.emplace_back(allocated_output_name.get());

    const auto output_type_info = session_.GetOutputTypeInfo(output_index);
    output_dimensions_.emplace_back(output_type_info.GetTensorTypeAndShapeInfo().GetShape());

    const auto& output_dimensions = output_dimensions_.back();
    output_tensor_sizes_.emplace_back(
        std::accumulate(output_dimensions.begin(), output_dimensions.end(), 1LL, std::multiplies()));
  }
}

}  // namespace ufld
