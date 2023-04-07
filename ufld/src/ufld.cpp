#include "ufld/ufld.h"

#include <array>
#include <cassert>
#include <numeric>

namespace ufld {

void VisualizeLanes(const std::vector<Lane>& lanes, cv::Mat& image) {
  assert(lanes.size() <= 4);
  const std::array<cv::Scalar, 4> kLaneColors{
      {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}}};
  for (uint32_t i = 0; i < lanes.size(); ++i) {
    const auto& lane = lanes[i];
    const auto& color = kLaneColors[i];
    constexpr auto kRadius = 5;
    constexpr auto kThickness = -1;  // Fill
    for (const auto j : lane) {
      cv::circle(image, j, kRadius, color, kThickness);
    }
  }
}

ILaneDetector::ILaneDetector(const std::filesystem::path& model_path) {
    InitializeSession(model_path);
    InitializeInput();
    InitializeOutputs();
}

std::vector<Lane> ILaneDetector::Detect(const cv::Mat& image) {
  const auto input = Preprocess(image);
  const auto outputs = Inference(input);
  return PredictionsToLanes(outputs, image.cols, image.rows);
}

void ILaneDetector::ColorPreprocess(cv::Mat& image) {
  cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
  image.convertTo(image, CV_32FC3, 1.0f / 255.0f);
  const cv::Scalar mean{0.485, 0.456, 0.406};
  const cv::Scalar std{0.229, 0.224, 0.225};
  image = (image - mean) / std;
}

void ILaneDetector::InitializeSession(const std::filesystem::path& model_path) {
  if (!std::filesystem::exists(model_path)) {
    std::cerr << "Model file not found: " << model_path << std::endl;
    throw std::invalid_argument{"Model file not found"};
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

}  // namespace ufld
