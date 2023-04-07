#include "ufld/v1.h"

#include <array>
#include <cassert>
#include <iostream>
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

LaneDetector::LaneDetector(const std::filesystem::path& model_directory,
                           ModelType model_type)
    : ILaneDetector{ConstructModelPath(model_directory, model_type)} {
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
}

Ort::Value LaneDetector::Preprocess(const cv::Mat& image) {
  cv::Mat input_image;
  cv::resize(image, input_image, cv::Size{kInputWidth, kInputHeight});

  ColorPreprocess(input_image);

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

  const std::array<uint32_t, 4> kShape{
      {1, config_->griding_num, config_->cls_num_per_lane, kLaneCount}};

  const auto tensor_type_and_shape_info =
      predictions.GetTensorTypeAndShapeInfo();
  const auto shape = tensor_type_and_shape_info.GetShape();
  assert(shape.size() == 4);
  assert(shape[0] == kShape[0] && shape[1] == kShape[1] &&
         shape[2] == kShape[2] && shape[3] == kShape[3]);

  const std::span<const float> predictions_raw(
      predictions.GetTensorData<float>(),
      tensor_type_and_shape_info.GetElementCount());
  const std::vector<float> probabilities =
      utils::Softmax_1(predictions_raw, kShape);
  const std::vector<uint32_t> predicted_cells =
      utils::ArgMax_1(std::span{probabilities}, kShape);

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

std::filesystem::path LaneDetector::ConstructModelPath(
    const std::filesystem::path& model_directory,
    ufld::v1::ModelType model_type) {
  switch (model_type) {
    case ufld::v1::ModelType::kCULane:
      return model_directory / kCULaneModelFile;
    case ufld::v1::ModelType::kTuSimple:
      return model_directory / kTuSimpleModelFile;
    default:
      throw std::runtime_error("Invalid model type");
  }
}

}  // namespace ufld::v1
