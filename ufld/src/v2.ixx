module;

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include <onnxruntime_cxx_api.h>

#include <opencv2/opencv.hpp>
#include <variant>
#include <xtensor/xadapt.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

export module ufld:v2;

import :core;
import :math;
import :v2.config;

namespace ufld::v2 {
/**
 * Ultra-Fast-Lane-Detection-V2
 */
export class LaneDetector final : public ufld::LaneDetector<LaneDetector> {
 public:
  LaneDetector(const std::filesystem::path& model_directory, Variant variant);

  LaneDetector(const LaneDetector&) = delete;
  LaneDetector& operator=(const LaneDetector&) = delete;

  LaneDetector(LaneDetector&&) = default;
  LaneDetector& operator=(LaneDetector&&) = default;

  [[nodiscard]] PreProcessResult PreProcess(cv::Mat&& image) const;

  [[nodiscard]] PostProcessResult PostProcess(const std::vector<Ort::Value>& outputs,
                                              const ImageInfo& image_info) const;

 private:
  static constexpr std::uint32_t kLaneCount{4};
  config::config_t config_{};

  [[nodiscard]] static std::filesystem::path MakeModelPath(const std::filesystem::path& directory, Variant variant);
};
}  // namespace ufld::v2

ufld::v2::LaneDetector::LaneDetector(const std::filesystem::path& model_directory, Variant variant)
    : ufld::LaneDetector<LaneDetector>{MakeModelPath(model_directory, variant), Version::kV2, variant} {
  switch (variant) {
    case Variant::kCULane18: {
      config_ = config::CULane18Config{};
    } break;
    case Variant::kCULane34: {
      config_ = config::CULane34Config{};
    } break;
    case Variant::kCurveLanes18: {
      config_ = config::CurveLanes18Config{};
    } break;
    case Variant::kCurveLanes34: {
      config_ = config::CurveLanes34Config{};
    } break;
    case Variant::kTuSimple18: {
      config_ = config::TuSimple18Config{};
    } break;
    case Variant::kTuSimple34: {
      config_ = config::TuSimple34Config{};
    } break;
    default:
      throw std::invalid_argument{"Invalid model variant"};
  }
}

ufld::PreProcessResult ufld::v2::LaneDetector::PreProcess(cv::Mat&& image) const {
  const auto start_time = std::chrono::high_resolution_clock::now();
  const cv::Size size = image.size();
  cv::Rect crop{};
  crop.x = 0;
  crop.y = 0;
  crop.width = image.cols;
  crop.height = image.rows;
  if (std::holds_alternative<config::CULane18Config>(config_)) {
    crop.y = static_cast<std::int32_t>(static_cast<float>(image.rows) * 0.4f);
  }
  if (std::holds_alternative<config::CULane18Config>(config_)) {
    crop.height = static_cast<std::int32_t>(static_cast<float>(image.rows) * 0.5f);
  }
  image = image(crop);
  cv::resize(image, image, cv::Size{config::InputWidth(config_), config::InputHeight(config_)});
  ColorPreProcess(image);
  return {.image = std::move(image),
          .image_info = {.size = size, .crop = crop},
          .duration = Duration(start_time, std::chrono::high_resolution_clock::now())};
}

ufld::PostProcessResult ufld::v2::LaneDetector::PostProcess(const std::vector<Ort::Value>& outputs,
                                                            const ImageInfo& image_info) const {
  const auto start_time = std::chrono::high_resolution_clock::now();

  const xt::xtensor<float, 3> location_output_row = xt::adapt(  // e.g. [200, 72, 4]
      outputs[0].GetTensorData<float>(), outputs[0].GetTensorTypeAndShapeInfo().GetElementCount(), xt::no_ownership(),
      std::array<std::size_t, 3>{config::RowAnchorCellCount(config_), config::RowAnchorCount(config_),
                                 config::LaneCount(config_)});
  const xt::xtensor<float, 3> location_output_column = xt::adapt(  // e.g. [100, 81, 4]
      outputs[1].GetTensorData<float>(), outputs[1].GetTensorTypeAndShapeInfo().GetElementCount(), xt::no_ownership(),
      std::array<std::size_t, 3>{config::ColumnAnchorCellCount(config_), config::ColumnAnchorCount(config_),
                                 config::LaneCount(config_)});
  const xt::xtensor<float, 3> existence_output_row =  // e.g. [2, 72, 4]
      xt::adapt(outputs[2].GetTensorData<float>(), outputs[2].GetTensorTypeAndShapeInfo().GetElementCount(),
                xt::no_ownership(),
                std::array<std::size_t, 3>{2, config::RowAnchorCount(config_), config::LaneCount(config_)});
  const xt::xtensor<float, 3> existence_output_column =  // e.g. [2, 81, 4]
      xt::adapt(outputs[3].GetTensorData<float>(), outputs[3].GetTensorTypeAndShapeInfo().GetElementCount(),
                xt::no_ownership(),
                std::array<std::size_t, 3>{2, config::ColumnAnchorCount(config_), config::LaneCount(config_)});

  const auto&& peak_indices_row = xt::argmax(location_output_row, 0);        // e.g. [72, 4]
  const auto&& existence_row = xt::argmax(existence_output_row, 0);          // e.g. [72, 4]
  const auto&& peak_indices_column = xt::argmax(location_output_column, 0);  // e.g. [81, 4]
  const auto&& existence_column = xt::argmax(existence_output_column, 0);    // e.g. [81, 4]

  // Because of crop ratio in certain model configs, we need to adjust the y-coordinate
  constexpr auto AdjustYAccordingToCropRatio = [](float y, std::int32_t height, float crop_ratio) -> float {
    const float multiplier = static_cast<float>(height) / crop_ratio;
    return (y * multiplier - (multiplier - static_cast<float>(height))) / static_cast<float>(height);
  };

  std::array<Lane, kLaneCount> lanes{};
  const auto GetLane = [&](std::uint32_t lane_index) -> Lane& {
    if (std::holds_alternative<config::CurveLanes18Config>(config_) || std::holds_alternative<config::CurveLanes34Config>(config_)) {
      // In CurveLanes, we only use the 4 middle lanes {3, 4, 5, 6} out of the total 10 lanes.
      switch (lane_index) {
        case 3:
          return lanes[1];
        case 4:
          return lanes[0];
        case 5:
          return lanes[3];
        case 6:
          return lanes[2];
        default:
          throw std::logic_error{"Unexpected lane index"};
      }
    }
    return lanes[lane_index];
  };

  for (std::uint32_t lane_index : config::RowLaneIndices(config_)) {  // Row Anchors
    const auto detected_count = xt::count_nonzero(xt::view(existence_row, xt::all(), lane_index))(0);
    if (detected_count <= config::RowAnchorCount(config_) / 4)
      continue;  // Skip if too few detections for this lane
    Lane& lane = GetLane(lane_index);
    lane.reserve(detected_count);
    for (std::uint32_t row_anchor_index = 0; row_anchor_index < config::RowAnchorCount(config_); ++row_anchor_index) {
      if (existence_row(row_anchor_index, lane_index) == 0)
        continue;  // Skip if not detected
      const std::size_t peak = peak_indices_row(row_anchor_index, lane_index);
      const std::size_t indices_begin = std::max(1ULL, peak) - 1ULL;
      const std::size_t indices_end = std::min(config::RowAnchorCellCount(config_) - 1ULL, peak + 1ULL) + 1ULL;
      const auto indices = xt::arange<std::size_t>(indices_begin, indices_end);
      auto probabilities = math::SoftMax<0>(
          xt::view(location_output_row, xt::range(indices_begin, indices_end), row_anchor_index, lane_index));
      auto expectation = xt::sum(probabilities * xt::cast<float>(indices));
      const float location = xt::eval(expectation)(0) + 0.5f;

      float y = config::RowAnchor(config_, row_anchor_index);
      float x = location / static_cast<float>(config::RowAnchorCellCount(config_) - 1);

      y = AdjustYAccordingToCropRatio(y, config::InputHeight(config_), config::CropRatio(config_));

      y *= static_cast<float>(image_info.crop.height);
      x *= static_cast<float>(image_info.crop.width);

      y += static_cast<float>(image_info.crop.y);
      x += static_cast<float>(image_info.crop.x);

      lane.emplace_back(x, y);
    }
  }

  for (std::uint32_t lane_index : config::ColumnLaneIndices(config_)) {  // Column Anchors
    const auto detected_count = xt::count_nonzero(xt::view(existence_column, xt::all(), lane_index))(0);
    if (detected_count <= config::ColumnAnchorCount(config_) / 4)
      continue;  // Skip if too few detections for this lane
    Lane& lane = GetLane(lane_index);
    lane.reserve(detected_count);
    for (std::uint32_t column_anchor_index = 0; column_anchor_index < config::ColumnAnchorCount(config_);
         ++column_anchor_index) {
      if (existence_column(column_anchor_index, lane_index) == 0)
        continue;  // Skip if not detected
      const std::size_t peak = peak_indices_column(column_anchor_index, lane_index);
      const std::size_t indices_begin = std::max(1ULL, peak) - 1ULL;
      const std::size_t indices_end = std::min(config::ColumnAnchorCellCount(config_) - 1ULL, peak + 1ULL) + 1ULL;
      const auto indices = xt::arange<std::size_t>(indices_begin, indices_end);
      auto probabilities = math::SoftMax<0>(
          xt::view(location_output_column, xt::range(indices_begin, indices_end), column_anchor_index, lane_index));
      auto expectation = xt::sum(probabilities * xt::cast<float>(indices));
      const float location = xt::eval(expectation)(0) + 0.5f;

      float x = config::ColumnAnchor(config_, column_anchor_index);
      float y = location / static_cast<float>(config::ColumnAnchorCellCount(config_) - 1);

      y = AdjustYAccordingToCropRatio(y, config::InputHeight(config_), config::CropRatio(config_));

      y *= static_cast<float>(image_info.crop.height);
      x *= static_cast<float>(image_info.crop.width);

      y += static_cast<float>(image_info.crop.y);
      x += static_cast<float>(image_info.crop.x);

      lane.emplace_back(x, y);
    }
  }

  return {.lanes = std::move(lanes), .duration = Duration(start_time, std::chrono::high_resolution_clock::now())};
}

std::filesystem::path ufld::v2::LaneDetector::MakeModelPath(const std::filesystem::path& directory, Variant variant) {
  switch (variant) {
    case Variant::kCULane18:
      return directory / "ufld_v2_culane_res18_320x1600.onnx";
    case Variant::kCULane34:
      return directory / "ufld_v2_culane_res34_320x1600.onnx";
    case Variant::kCurveLanes18:
      return directory / "ufld_v2_curvelanes_res18_800x1600.onnx";
    case Variant::kCurveLanes34:
      return directory / "ufld_v2_curvelanes_res34_800x1600.onnx";
    case Variant::kTuSimple18:
      return directory / "ufld_v2_tusimple_res18_320x800.onnx";
    case Variant::kTuSimple34:
      return directory / "ufld_v2_tusimple_res34_320x800.onnx";
    default:
      throw std::invalid_argument{"Invalid model variant"};
  }
}
