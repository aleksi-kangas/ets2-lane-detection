module;

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
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

using namespace xt::placeholders;

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

  [[nodiscard]] PostProcessResult PostProcess(std::vector<Ort::Value>&& outputs, const ImageInfo& image_info) const;

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
  const cv::Size size = image.size();
  cv::Rect crop{};
  crop.x = 0;
  crop.y = 0;
  crop.width = image.cols;
  crop.height = image.rows;
  image = image(crop);
  cv::resize(image, image, cv::Size{kInputWidth, kInputHeight});
  ColorPreProcess(image);
  return {.image = std::move(image),
          .image_info = {.size = size, .crop = crop},
          .duration = Duration(start_time, std::chrono::high_resolution_clock::now())};
}

ufld::PostProcessResult ufld::v1::LaneDetector::PostProcess(std::vector<Ort::Value>&& outputs,
                                                            const ImageInfo& image_info) const {
  const auto start_time = std::chrono::high_resolution_clock::now();

  const xt::xtensor<float, 3> location_output = xt::adapt(
      outputs[0].GetTensorData<float>(), outputs[0].GetTensorTypeAndShapeInfo().GetElementCount(), xt::no_ownership(),
      std::array<std::size_t, 3>{config::RowAnchorCellCount(config_), config::RowAnchorCount(config_), kLaneCount});

  // The last cell in each row anchor signifies the absence of a lane,
  // so store the argmax.
  const uint32_t kNotDetectedIndex = config::RowAnchorCellCount(config_) - 1;
  const auto&& location_argmax = xt::argmax(location_output, 0);  // e.g. [18, 4]

  // Using mathematical expectation, compute the lane location in each row anchor.
  auto locations_view = xt::view(location_output, xt::range(_, -1), xt::all(), xt::all());  // e.g. [200, 18, 4]
  auto probabilities = ufld::math::SoftMax<0>(locations_view);                              // e.g. [200, 18, 4]
  auto indices =
      xt::arange<std::size_t>(1, config::RowAnchorCellCount(config_)).reshape({-1, 1, 1});  // e.g. [200, 1, 1]
  auto&& locations = xt::eval(xt::sum(probabilities * xt::cast<float>(indices), 0));        // e.g. [18, 4]

  std::array<Lane, kLaneCount> lanes{};
  for (std::uint32_t lane_index = 0; lane_index < kLaneCount; ++lane_index) {
    const auto detected_count =
        xt::count_nonzero(xt::not_equal(xt::view(location_argmax, xt::all(), lane_index), kNotDetectedIndex))(0);
    if (detected_count <= config::RowAnchorCount(config_) / 2)
      continue;  // Skip if too few detections for this lane
    Lane& lane = lanes[lane_index];
    lane.reserve(detected_count);
    for (std::uint32_t row_anchor_index = 0; row_anchor_index < config::RowAnchorCount(config_); ++row_anchor_index) {
      if (location_argmax(row_anchor_index, lane_index) == kNotDetectedIndex)
        continue;  // Skip if not detected

      const auto anchor = static_cast<float>(config::RowAnchor(config_, row_anchor_index));
      const auto location = locations(row_anchor_index, lane_index);

      float y = anchor / static_cast<float>(kInputHeight);
      float x = location / static_cast<float>(config::RowAnchorCellCount(config_) - 1);

      y *= static_cast<float>(image_info.crop.height);
      x *= static_cast<float>(image_info.crop.width);

      y += static_cast<float>(image_info.crop.y);
      x += static_cast<float>(image_info.crop.x);

      lane.emplace_back(x, y);
    }
  }

  return {.lanes = std::move(lanes), .duration = Duration(start_time, std::chrono::high_resolution_clock::now())};
}

std::filesystem::path ufld::v1::LaneDetector::MakeModelPath(const std::filesystem::path& directory, Variant variant) {
  switch (variant) {
    case Variant::kCULane:
      return directory / "ufld_v1_culane_288x800.onnx";
    case Variant::kTuSimple:
      return directory / "ufld_v1_tusimple_288x800.onnx";
    default:
      throw std::invalid_argument{"Invalid model variant"};
  }
}
