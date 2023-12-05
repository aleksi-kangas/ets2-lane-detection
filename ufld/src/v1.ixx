module;

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <span>
#include <stdexcept>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

export module ufld:v1;

import :core;
import :math;
import :v1.config;

namespace ufld::v1 {
/**
 * Ultra-Fast-Lane-Detection-V1
 */
export class LaneDetector final : public ufld::LaneDetector<LaneDetector> {
 public:
  LaneDetector(const std::filesystem::path& model_directory, Variant variant);

  LaneDetector(const LaneDetector&) = delete;
  LaneDetector& operator=(const LaneDetector&) = delete;

  LaneDetector(LaneDetector&&) = default;
  LaneDetector& operator=(LaneDetector&&) = default;

  [[nodiscard]] PreProcessResult PreProcess(cv::Mat&& image) const;

  [[nodiscard]] PostProcessResult PostProcess(const std::vector<Ort::Value>& outputs, cv::Rect crop_area,
                                              cv::Size original_size) const;

 private:
  static constexpr std::uint32_t kInputWidth{800};
  static constexpr std::uint32_t kInputHeight{288};
  static constexpr float kInputAspectRatio = static_cast<float>(kInputWidth) / static_cast<float>(kInputHeight);
  static constexpr std::uint32_t kLaneCount{4};

  config::config_t config_{};

  [[nodiscard]] static std::filesystem::path MakeModelPath(const std::filesystem::path& directory, Variant variant);
};
}  // namespace ufld::v1

ufld::v1::LaneDetector::LaneDetector(const std::filesystem::path& model_directory, Variant variant)
    : ufld::LaneDetector<LaneDetector>{MakeModelPath(model_directory, variant), Version::kV1, variant} {
  switch (variant) {
    case Variant::kCULane:
      config_ = config::CULaneConfig{};
      break;
    case Variant::kTuSimple:
      config_ = config::TuSimpleConfig{};
      break;
    default:
      throw std::invalid_argument{"Invalid model variant"};
  }
}

ufld::PreProcessResult ufld::v1::LaneDetector::PreProcess(cv::Mat&& image) const {
  const auto start_time = std::chrono::high_resolution_clock::now();
  const cv::Size original_size = image.size();
  const cv::Rect crop_area = CenterCropArea(image, kInputAspectRatio);
  image = image(crop_area);
  cv::resize(image, image, cv::Size{kInputWidth, kInputHeight});
  ColorPreProcess(image);
  return {
      .processed_image = std::move(image),
      .original_size = original_size,
      .crop_area = crop_area,
      .duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time),
  };
}

ufld::PostProcessResult ufld::v1::LaneDetector::PostProcess(const std::vector<Ort::Value>& outputs, cv::Rect crop_area,
                                                            cv::Size original_size) const {
  const auto start_time = std::chrono::high_resolution_clock::now();

  const std::uint32_t row_anchor_count = config::RowAnchorCount(config_);
  const std::uint32_t row_anchor_cell_count = config::RowAnchorCellCount(config_);

  const xt::xtensor<float, 3> location_output =
      xt::adapt(outputs[0].GetTensorData<float>(), outputs[0].GetTensorTypeAndShapeInfo().GetElementCount(),
                xt::no_ownership(), std::array<std::size_t, 3>{row_anchor_cell_count, row_anchor_count, kLaneCount});

  // The last cell in each row anchor signifies the absence of a lane,
  // so store the argmax.
  const uint32_t kNotDetectedIndex = row_anchor_cell_count - 1;
  const auto&& location_argmax = xt::argmax(location_output, 0);  // e.g. [18, 4]

  // Using mathematical expectation, compute the lane location in each row anchor.
  auto locations_view =
      xt::view(location_output, xt::range(xt::placeholders::_, -1), xt::all(), xt::all());  // e.g. [200, 18, 4]
  auto probabilities = ufld::math::SoftMax<0>(locations_view);                              // e.g. [200, 18, 4]
  auto indices = xt::arange<std::size_t>(1, row_anchor_cell_count).reshape({-1, 1, 1});     // e.g. [200, 1, 1]
  auto&& locations = xt::eval(xt::sum(probabilities * xt::cast<float>(indices), 0));        // e.g. [18, 4]

  std::array<Lane, 4> lanes{};
  for (std::uint32_t lane_index = 0; lane_index < kLaneCount; ++lane_index) {
    const auto detected_count =
        xt::count_nonzero(xt::not_equal(xt::view(location_argmax, xt::all(), lane_index), kNotDetectedIndex))(0);
    if (detected_count <= row_anchor_count / 2)
      continue;  // Skip if too few detections for this lane
    Lane& lane = lanes[lane_index];
    lane.reserve(detected_count);
    for (std::uint32_t row_anchor_index = 0; row_anchor_index < row_anchor_count; ++row_anchor_index) {
      if (location_argmax(row_anchor_index, lane_index) == kNotDetectedIndex)
        continue;  // Skip if not detected

      const auto anchor = static_cast<float>(config::RowAnchor(config_, row_anchor_index));
      const auto location = locations(row_anchor_index, lane_index);

      // 1st) Coordinates in the input image (800 x 288)
      float y = anchor;
      float x = location / static_cast<float>(row_anchor_cell_count - 1) * static_cast<float>(kInputWidth);

      // 2nd) Coordinates in the cropped image before resizing
      y *= static_cast<float>(crop_area.height) / static_cast<float>(kInputHeight);
      x *= static_cast<float>(crop_area.width) / static_cast<float>(kInputWidth);

      // 3rd) Coordinates in the original image
      y += (static_cast<float>(original_size.height) - static_cast<float>(crop_area.height)) / 2.0f;
      x += (static_cast<float>(original_size.width) - static_cast<float>(crop_area.width)) / 2.0f;

      lane.emplace_back(x, y);
    }
  }

  return {
      .lanes = std::move(lanes),
      .duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time)};
}

std::filesystem::path ufld::v1::LaneDetector::MakeModelPath(const std::filesystem::path& directory, Variant variant) {
  switch (variant) {
    case Variant::kCULane: {
      constexpr auto kCULaneModelFile = "ufld_v1_culane_288x800.onnx";
      return directory / kCULaneModelFile;
    }
    case Variant::kTuSimple: {
      constexpr auto kTuSimpleModelFile = "ufld_v1_tusimple_288x800.onnx";
      return directory / kTuSimpleModelFile;
    }
    default:
      throw std::invalid_argument{"Invalid model variant"};
  }
}
