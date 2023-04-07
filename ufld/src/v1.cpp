#include "ufld/v1.h"

#include <array>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <span>
#include <stdexcept>

#include "ufld/utils.h"

namespace ufld::v1 {

ModelType ModelTypeFromString(const std::string& model_type) {
  std::string model_type_lower = model_type;
  std::transform(model_type_lower.begin(), model_type_lower.end(),
                 model_type_lower.begin(),
                 [](auto c) { return static_cast<char>(std::tolower(c)); });
  if (model_type_lower == "culane") {
    return ModelType::kCULane;
  } else if (model_type_lower == "tusimple") {
    return ModelType::kTuSimple;
  } else {
    throw std::invalid_argument{"Invalid model type"};
  }
}

LaneDetector::LaneDetector(const std::filesystem::path& model_directory, ModelType model_type) {
  std::filesystem::path model_path = [=]() {
    std::filesystem::path base_path{model_directory};
    switch (model_type) {
      case ModelType::kCULane:
        return base_path.append(kCULaneModelFile);
      case ModelType::kTuSimple:
        return base_path.append(kTuSimpleModelFile);
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
  std::copy(preprocessed_image.begin<float>(), preprocessed_image.end<float>(),
            input_tensor_data_.begin());

  Ort::MemoryInfo memory_info =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  return Ort::Value::CreateTensor<float>(
      memory_info, input_tensor_data_.data(), input_tensor_data_.size(),
      input_dimensions_.data(), input_dimensions_.size());
}

std::vector<Ort::Value> LaneDetector::Inference(const Ort::Value& input) {
  output_tensor_data_.clear();
  Ort::MemoryInfo memory_info =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  std::vector<Ort::Value> output_tensors;
  for (auto output_index = 0; output_index < session_.GetOutputCount();
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

  const char* input_name = input_name_.c_str();

  // Ort::Session::Run expects an array of const char* for the output names
  auto OutputNamesToRaw = [&]() {
    std::vector<const char*> output_names;
    for (const auto& output_name : output_names_) {
      output_names.push_back(output_name.c_str());
    }
    return output_names;
  };

  session_.Run(Ort::RunOptions{nullptr}, &input_name, &input, 1,
               OutputNamesToRaw().data(), output_tensors.data(),
               output_tensors.size());
  return output_tensors;
}

std::vector<Lane> LaneDetector::PredictionsToLanes(
    const std::vector<Ort::Value>& outputs, int32_t image_width,
    int32_t image_height) {
  // https://github.com/cfzd/Ultra-Fast-Lane-Detection/blob/master/demo.py

  // We have exactly 1 output
  const auto& predictions = outputs[0];

  const std::array<uint32_t, 3> k3DShape{
      {config_->griding_num, config_->cls_num_per_lane, kLaneCount}};

  const auto tensor_type_and_shape_info =
      predictions.GetTensorTypeAndShapeInfo();
  const auto shape = tensor_type_and_shape_info.GetShape();
  assert(shape.size() == 4);
  assert(shape[0] == 1 && shape[1] == k3DShape[0] && shape[2] == k3DShape[1] &&
         shape[3] == k3DShape[2]);

  const std::span<const float> predictions_raw(
      predictions.GetTensorData<float>(),
      tensor_type_and_shape_info.GetElementCount());
  const std::vector<float> probabilities =
      utils::Softmax_0(predictions_raw, k3DShape);
  const std::vector<uint32_t> predicted_cells =
      utils::ArgMax_0(std::span{probabilities}, k3DShape);

  std::vector<Lane> lanes;
  for (int32_t lane_index = 0; lane_index < kLaneCount; ++lane_index) {
    Lane lane;
    for (uint32_t class_index = 0; class_index < config_->cls_num_per_lane;
         ++class_index) {
      const uint32_t kIndexOfNoLane = config_->griding_num - 1;
      const auto index = class_index * kLaneCount + lane_index;
      if (predicted_cells[index] == kIndexOfNoLane) {
        continue;
      }

      const float kGridCellWidth = static_cast<float>(kInputWidth) /
                                   static_cast<float>(config_->griding_num);
      const float kWidthScale =
          static_cast<float>(image_width) / static_cast<float>(kInputWidth);
      const float kHeightScale =
          static_cast<float>(image_height) / static_cast<float>(kInputHeight);

      const auto x =
          (static_cast<float>(predicted_cells[index]) * kGridCellWidth +
           kGridCellWidth * 0.5) *
          kWidthScale;
      const auto y =
          static_cast<float>(config_->row_anchors[class_index]) * kHeightScale;
      lane.emplace_back(static_cast<int32_t>(x), static_cast<int32_t>(y));
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
    std::cerr << "Failed to create OnnxRuntime session: " << e.what()
              << std::endl;
    throw;
  }

  assert(session_.GetInputCount() == 1);
  assert(session_.GetOutputCount() == 1);
}

void LaneDetector::InitModelInfo() {
  assert(session_.GetInputCount() == 1);
  assert(session_.GetOutputCount() == 1);

  const auto allocated_input_name =
      session_.GetInputNameAllocated(0, allocator_);
  input_name_ = std::string{allocated_input_name.get()};

  Ort::TypeInfo input_type_info = session_.GetInputTypeInfo(0);
  assert(input_type_info.GetONNXType() == ONNX_TYPE_TENSOR);
  input_dimensions_ = input_type_info.GetTensorTypeAndShapeInfo().GetShape();
  input_tensor_size_ =
      std::accumulate(input_dimensions_.begin(), input_dimensions_.end(), 1LL,
                      std::multiplies<>());

  // Realistically, we have exactly one output, but does not hurt to be generic.
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

}  // namespace ufld::v1
