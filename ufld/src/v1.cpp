#include "ufld/v1.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <stdexcept>

namespace {

std::vector<float> Softmax_0(const std::vector<float>& input, const std::array<int32_t, 3>& dimensions) {
  std::vector<float> softmax(input.size());
  const auto dim1 = dimensions[0];
  const auto dim2 = dimensions[1];
  const auto dim3 = dimensions[2];

  for (int32_t d2 = 0; d2 < dim2; ++d2) {
    for (int32_t d3 = 0; d3 < dim3; ++d3) {
      float sum = 0;
      auto index = [=](auto d1) {
        return d1 * dim2 * dim3 + d2 * dim3 + d3;
      };

      for (int32_t d1 = 0; d1 < dim1; ++d1) {
        sum += std::exp(input[index(d1)]);
      }
      for (int32_t d1 = 0; d1 < dim1; ++d1) {
        softmax[index(d1)] = std::exp(input[index(d1)]) / sum;
      }
    }
  }
  return softmax;
}

std::vector<int32_t> ArgMax_0(const std::vector<float>& input, const std::array<int32_t, 3>& dimensions) {
  const auto dim1 = dimensions[0];
  const auto dim2 = dimensions[1];
  const auto dim3 = dimensions[2];
  std::vector<int32_t> argmax(dim2 * dim3);
  for (int32_t d2 = 0; d2 < dim2; ++d2) {
    for (int32_t d3 = 0; d3 < dim3; ++d3) {
      float max = input[d2 * dim3 + d3];
      int32_t argmax_index = 0;
      auto index = [=](auto d1) {
        return d1 * dim2 * dim3 + d2 * dim3 + d3;
      };

      for (int32_t d1 = 1; d1 < dim1; ++d1) {
        if (input[index(d1)] > max) {
          max = input[index(d1)];
          argmax_index = d1;
        }
      }
      argmax[d2 * dim3 + d3] = argmax_index;
    }
  }
  return argmax;
}

}  // namespace

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

  if (!std::filesystem::exists(model_path)) {
    std::cerr << "Model file not found: " << model_path << std::endl;
    throw std::invalid_argument{"Model file not found"};
  }

  config_ = [=]() -> std::unique_ptr<IConfig> {
    switch (model_type) {
      case ModelType::kCULane:
        return std::make_unique<CULaneConfig>();
      case ModelType::kTuSimple:
        return std::make_unique<TuSimpleConfig>();
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

  cv::Scalar mean{0.485, 0.456, 0.406};
  cv::Scalar std{0.229, 0.224, 0.225};
  input_image = (input_image - mean) / std;

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
  const char* input_name = input_name_.c_str();
  const char* output_name = output_name_.c_str();
  session_.Run(Ort::RunOptions{nullptr}, &input_name, &input, 1, &output_name, &output, 1);
  return output;
}

std::vector<Lane> LaneDetector::PredictionsToLanes(const Ort::Value& predictions, int32_t image_width,
                                                   int32_t image_height) {
  // https://github.com/cfzd/Ultra-Fast-Lane-Detection/blob/master/demo.py
  std::vector<float> predictions_raw(
      predictions.GetTensorData<float>(),
      predictions.GetTensorData<float>() + predictions.GetTensorTypeAndShapeInfo().GetElementCount());
  std::vector<float> probabilities =
      Softmax_0(predictions_raw, {config_->griding_num, config_->cls_num_per_lane, kLaneCount});
  std::vector<int32_t> predicted_cells =
      ArgMax_0(probabilities, {config_->griding_num, config_->cls_num_per_lane, kLaneCount});
  std::vector<Lane> lanes;
  for (int32_t lane_index = 0; lane_index < kLaneCount; ++lane_index) {
    Lane lane;
    for (int32_t class_index = 0; class_index < config_->cls_num_per_lane; ++class_index) {
      const int32_t kIndexOfNoLane = config_->griding_num - 1;
      const auto index = class_index * kLaneCount + lane_index;
      if (predicted_cells[index] == kIndexOfNoLane) {
        continue;
      }

      // TODO This is strictly not correct. Should use linspace.
      const float kGridCellWidth = (kInputWidth) / static_cast<float>(config_->griding_num);
      const auto x = static_cast<float>(predicted_cells[index]) * kGridCellWidth * static_cast<float>(image_width) /
                     static_cast<float>(kInputWidth);
      const auto y = static_cast<float>(config_->row_anchors[class_index]) * static_cast<float>(image_height) /
                     static_cast<float>(kInputHeight);
      lane.emplace_back(x, y);
    }
    lanes.push_back(lane);
  }

  return lanes;
}

void LaneDetector::InitSession(const std::filesystem::path& model_path) {
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
    std::cerr << "Failed to create OnnxRuntime session: " << e.what() << std::endl;
    throw;
  }

  assert(session_.GetInputCount() == 1);
  assert(session_.GetOutputCount() == 1);
}

void LaneDetector::InitModelInfo() {
  assert(session_.GetInputCount() == 1);
  assert(session_.GetOutputCount() == 1);

  const auto allocated_input_name = session_.GetInputNameAllocated(0, allocator_);
  input_name_ = std::string{allocated_input_name.get()};

  Ort::TypeInfo input_type_info = session_.GetInputTypeInfo(0);
  assert(input_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
  input_dimensions_ = input_type_info.GetTensorTypeAndShapeInfo().GetShape();
  input_tensor_size_ = std::accumulate(input_dimensions_.begin(), input_dimensions_.end(), 1, std::multiplies<>());

  const auto allocated_output_name = session_.GetOutputNameAllocated(0, allocator_);
  output_name_ = std::string{allocated_output_name.get()};
  Ort::TypeInfo output_type_info = session_.GetOutputTypeInfo(0);
  assert(output_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
  output_dimensions_ = output_type_info.GetTensorTypeAndShapeInfo().GetShape();
  output_tensor_size_ = std::accumulate(output_dimensions_.begin(), output_dimensions_.end(), 1, std::multiplies<>());
}

}  // namespace ufld::v1
