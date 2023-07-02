#include "ufld/ufld.h"

#include <array>
#include <cassert>
#include <numeric>

namespace {
void DrawLanePoints(const std::vector<ufld::Lane>& lanes,
                    std::array<bool, 4> lanes_to_draw, cv::Mat& image) {
  assert(lanes.size() <= 4);
  const std::array<cv::Scalar, 4> kLaneColors{
      {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 0, 255}}};
  for (uint32_t lane_index = 0; lane_index < lanes.size(); ++lane_index) {
    if (!lanes_to_draw[lane_index])
      continue;
    const auto& lane = lanes[lane_index];
    constexpr auto kRadius = 5;
    constexpr auto kThickness = -1;  // Fill
    for (const auto j : lane) {
      cv::circle(image, j, kRadius, kLaneColors[lane_index], kThickness);
    }
  }
}

void DrawCenterLaneMask(const std::vector<ufld::Lane>& lanes, cv::Mat& image) {
  assert(lanes.size() <= 4);
  const auto& left_divider = lanes[1];
  const auto& right_divider = lanes[2];
  if (left_divider.empty() || right_divider.empty())
    return;
  cv::Mat mask = image.clone();
  std::vector<cv::Point> lane_polygon{};
  lane_polygon.reserve(left_divider.size() + right_divider.size());
  lane_polygon.insert(lane_polygon.end(), left_divider.begin(),
                      left_divider.end());
  lane_polygon.insert(lane_polygon.end(), right_divider.rbegin(),
                      right_divider.rend());
  cv::fillPoly(mask, {lane_polygon}, cv::Scalar{255, 191, 0, 255});
  cv::addWeighted(mask, 0.3, image, 0.7, 0, image);
}
}  // namespace

namespace ufld {
std::vector<Lane> ILaneDetector::Detect(const cv::Mat& image,
                                        cv::Mat* preview) {
  std::lock_guard<std::mutex> lock{detection_mutex_};
  const auto preprocess_info = Preprocess(image);
  const auto outputs = Inference(preprocess_info.preprocessed_image);
  auto lanes = PredictionsToLanes(outputs, preprocess_info);
  if (preview) {
    DrawLanes(lanes, *preview);
    DrawInputArea(preprocess_info.crop_area, *preview);
  }
  return lanes;
}

std::filesystem::path ILaneDetector::ModelDirectory() const {
  return model_directory_;
}

Version ILaneDetector::ModelVersion() const {
  return version_;
}

std::variant<v1::Variant> ILaneDetector::ModelVariant() const {
  return variant_;
}

ILaneDetector::ILaneDetector(const std::filesystem::path& model_path,
                             Version version, std::variant<v1::Variant> variant)
    : model_directory_{model_path.parent_path()},
      version_{version},
      variant_{variant} {
  InitializeSession(model_path);
  InitializeInput();
  InitializeOutputs();
}

cv::Rect ILaneDetector::ComputeCenterCrop(const cv::Mat& image,
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

cv::Mat ILaneDetector::ColorPreprocess(const cv::Mat& image) {
  cv::Mat converted_image;
  cv::cvtColor(image, converted_image, cv::COLOR_RGBA2RGB);
  converted_image.convertTo(converted_image, CV_32FC3, 1.0f / 255.0f);
  // ResNet
  const cv::Scalar mean{0.485, 0.456, 0.406};
  const cv::Scalar std{0.229, 0.224, 0.225};
  return (converted_image - mean) / std;
}

void ILaneDetector::InitializeSession(const std::filesystem::path& model_path) {
  if (!std::filesystem::exists(model_path)) {
    throw std::invalid_argument{"Model file does not exist: " +
                                model_path.string()};
  }

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
    session_options.AppendExecutionProvider_TensorRT(tensorrt_options);
  } catch (const Ort::Exception& e) {
    std::cerr << "TensorRT is not available: " << e.what() << std::endl;
  }

  try {
    session_ = Ort::Session(env_, model_path.c_str(), session_options);
  } catch (const Ort::Exception& e) {
    std::cerr << "Failed to create OnnxRuntime session: " << e.what()
              << std::endl;
    throw;
  }
}

void ILaneDetector::InitializeInput() {
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

void ILaneDetector::InitializeOutputs() {
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

Ort::Value ILaneDetector::InitializeInputTensor(const cv::Mat& image) {
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

std::vector<Ort::Value> ILaneDetector::InitializeOutputTensors() {
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

std::vector<Ort::Value> ILaneDetector::Inference(const cv::Mat& image) {
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
  return output_tensors;
}

void ILaneDetector::DrawLanes(const std::vector<Lane>& lanes, cv::Mat& image) {
  assert(lanes.size() <= 4);
  DrawLanePoints(lanes, {false, true, true, false}, image);
  DrawCenterLaneMask(lanes, image);
}

void ILaneDetector::DrawInputArea(cv::Rect inputArea, cv::Mat& image) {
  cv::rectangle(image, inputArea, cv::Scalar{255, 0, 0, 255}, /* thickness=*/5);
}

}  // namespace ufld
