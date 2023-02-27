#include "ufld/v1.h"

#include <cassert>
#include <cstring>
#include <filesystem>
#include <numeric>
#include <stdexcept>

namespace ufld::v1 {

LaneDetector::LaneDetector(ModelType model_type) {
  std::filesystem::path model_path = [=]() {
    std::filesystem::path base_path{"models"};
    switch (model_type) {
      case ModelType::kCULane:
        return base_path.append("ufld_v1_culane_288x800.onnx");
      case ModelType::kTuSimple:
        return base_path.append("TODO");  // TODO
      default:
        throw std::invalid_argument{"Invalid model type"};
    }
  }();

  InitSession(model_path);
  InitModelInfo();
}

Ort::Value LaneDetector::Preprocess(const cv::Mat& image) {
  cv::Mat input_image;
  cv::resize(image, input_image, cv::Size{kInputWidth, kInputHeight});
  cv::cvtColor(input_image, input_image, cv::COLOR_BGR2RGB);
  input_image.convertTo(input_image, CV_32FC3, 1.0 / 255.0);
  // TODO Normalization

  cv::Mat preprocessed_image;
  cv::dnn::blobFromImage(input_image, preprocessed_image);

  input_tensor_data_.resize(static_cast<size_t>(input_tensor_size_));
  std::copy(preprocessed_image.begin<float>(), preprocessed_image.end<float>(), input_tensor_data_.begin());

  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  return Ort::Value::CreateTensor<float>(memory_info, input_tensor_data_.data(), input_tensor_data_.size(),
                                         input_dimensions_.data(), input_dimensions_.size());
}

Ort::Value LaneDetector::Inference(const Ort::Value& input) {
  output_tensor_data_.resize(static_cast<size_t>(output_tensor_size_));
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  auto output = Ort::Value::CreateTensor<float>(memory_info, output_tensor_data_.data(), output_tensor_data_.size(),
                                                output_dimensions_.data(), output_dimensions_.size());
  session_.Run(Ort::RunOptions{nullptr}, &input_name_, &input, 1, &output_name_, &output, 1);
  return output;
}

std::vector<Lane> LaneDetector::PredictionsToLanes(const Ort::Value& predictions) {
  return {};
}

void LaneDetector::InitSession(const std::filesystem::path& model_path) {
  Ort::SessionOptions session_options{};
  session_options.SetIntraOpNumThreads(1);

  // TensorRT
  OrtTensorRTProviderOptions tensorrt_options{};
  tensorrt_options.device_id = 0;
  session_options.AppendExecutionProvider_TensorRT(tensorrt_options);

  session_ = Ort::Session(env_, model_path.c_str(), session_options);
  assert(session_ != nullptr);

  assert(session_.GetInputCount() == 1);
  assert(session_.GetOutputCount() == 1);
}

void LaneDetector::InitModelInfo() {
  assert(session_.GetInputCount() == 1);
  assert(session_.GetOutputCount() == 1);

  const auto allocated_input_name = session_.GetInputNameAllocated(0, allocator_);
  input_name_ = _strdup(allocated_input_name.get());

  Ort::TypeInfo input_type_info = session_.GetInputTypeInfo(0);
  assert(input_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
  input_dimensions_ = input_type_info.GetTensorTypeAndShapeInfo().GetShape();
  input_tensor_size_ = std::accumulate(input_dimensions_.begin(), input_dimensions_.end(), 1, std::multiplies<>());

  const auto allocated_output_name = session_.GetOutputNameAllocated(0, allocator_);
  output_name_ = _strdup(allocated_output_name.get());
  Ort::TypeInfo output_type_info = session_.GetOutputTypeInfo(0);
  assert(output_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
  output_dimensions_ = output_type_info.GetTensorTypeAndShapeInfo().GetShape();
  output_tensor_size_ = std::accumulate(output_dimensions_.begin(), output_dimensions_.end(), 1, std::multiplies<>());
}

}  // namespace ufld::v1
