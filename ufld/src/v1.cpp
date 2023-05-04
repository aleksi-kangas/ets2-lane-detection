#include "ufld/v1.h"

#include <array>
#include <cassert>
#include <iostream>
#include <span>
#include <stdexcept>

#include "ufld/utils.h"

namespace ufld::v1 {

LaneDetector::LaneDetector(const std::filesystem::path& model_directory,
                           Variant variant)
    : ILaneDetector{ConstructModelPath(model_directory, variant), Version::kV1,
                    variant} {
  config_ = [=]() -> std::unique_ptr<IConfig> {
    switch (variant) {
      case Variant::kCULane:
        return std::make_unique<CULaneConfig>();
      case Variant::kTuSimple:
        return std::make_unique<TuSimpleConfig>();
      default:
        throw std::invalid_argument{"Invalid model type"};
    }
  }();
}

cv::Mat LaneDetector::Preprocess(const cv::Mat& image) {
  cv::Mat resized_image{};
  cv::resize(image, resized_image, cv::Size{kInputWidth, kInputHeight});
  return ColorPreprocess(resized_image);
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
    const std::filesystem::path& directory, ufld::v1::Variant variant) {
  switch (variant) {
    case ufld::v1::Variant::kCULane:
      return directory / kCULaneModelFile;
    case ufld::v1::Variant::kTuSimple:
      return directory / kTuSimpleModelFile;
    default:
      throw std::runtime_error("Invalid model type");
  }
}

}  // namespace ufld::v1
