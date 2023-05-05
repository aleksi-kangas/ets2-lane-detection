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

PreprocessInfo LaneDetector::Preprocess(const cv::Mat& image) {
  cv::Mat cropped_image = CenterCrop(image, kInputAspectRatio);
  cv::Mat resized_image{};
  cv::resize(cropped_image, resized_image, cv::Size{kInputWidth, kInputHeight});
  return {
      .preprocessed_image = ColorPreprocess(resized_image),
      .original_size = image.size(),
      .cropped_size = cropped_image.size(),
  };
}

std::vector<Lane> LaneDetector::PredictionsToLanes(
    const std::vector<Ort::Value>& outputs,
    const PreprocessInfo& preprocess_info) {
  // https://github.com/cfzd/Ultra-Fast-Lane-Detection/blob/master/demo.py
  const std::vector<uint32_t> predicted_cells = PredictedCells(outputs);

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

      // 1st) Coordinates in the input image (800 x 288)
      const float kGridCellWidth = static_cast<float>(kInputWidth) /
                                   static_cast<float>(config_->griding_num);
      auto y = static_cast<int32_t>(config_->row_anchors[class_index]);
      auto x = static_cast<int32_t>(static_cast<float>(predicted_cells[index]) *
                                        kGridCellWidth +
                                    kGridCellWidth * 0.5f);

      // 2nd) Coordinates in the cropped image before resizing
      const auto y_scale =
          static_cast<float>(preprocess_info.cropped_size.height) /
          static_cast<float>(kInputHeight);
      const auto x_scale =
          static_cast<float>(preprocess_info.cropped_size.width) /
          static_cast<float>(kInputWidth);
      y = static_cast<int32_t>(static_cast<float>(y) * y_scale);
      x = static_cast<int32_t>(static_cast<float>(x) * x_scale);

      // 3rd) Coordinates in the original image
      const auto crop_offset_y = (preprocess_info.original_size.height -
                                  preprocess_info.cropped_size.height) /
                                 2;
      const auto crop_offset_x = (preprocess_info.original_size.width -
                                  preprocess_info.cropped_size.width) /
                                 2;
      y += crop_offset_y;
      x += crop_offset_x;

      lane.emplace_back(static_cast<int32_t>(x), static_cast<int32_t>(y));
    }
    lanes.push_back(lane);
  }

  return lanes;
}

std::vector<uint32_t> LaneDetector::PredictedCells(
    const std::vector<Ort::Value>& outputs) const {
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
  return utils::ArgMax_1(std::span{probabilities}, kShape);
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
