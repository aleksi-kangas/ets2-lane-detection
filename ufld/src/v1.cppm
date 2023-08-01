module;

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

module ufld:v1;

import :base;
import :math;
import :v1_config;

namespace ufld::v1 {
/**
 * UFLD V1
 */
class LaneDetector final : public LaneDetectorBase {
 public:
  /**
   * Constructor
   * @param model_directory model directory
   * @param variant         UFLD V1 variant
   */
  LaneDetector(const std::filesystem::path& model_directory, ufld::v1::Variant variant);

  LaneDetector(const LaneDetector&) = delete;
  LaneDetector& operator=(const LaneDetector&) = delete;

  LaneDetector(LaneDetector&&) = delete;
  LaneDetector& operator=(LaneDetector&&) = delete;

 private:
  static constexpr int32_t kInputWidth = 800;
  static constexpr int32_t kInputHeight = 288;
  static constexpr float kInputAspectRatio = static_cast<float>(kInputWidth) / static_cast<float>(kInputHeight);
  static constexpr uint32_t kLaneCount = 4;

  std::unique_ptr<const IConfig> config_{nullptr};

  /**
   * Pre-process the input image.
   * Pre-processing includes cropping, resizing and color conversion.
   * @param image   input image
   * @return        pre-process result
   */
  [[nodiscard]] ufld::PreProcessResult PreProcess(const cv::Mat& image) const override;

  /**
   * Post-process the output of the model.
   * @param outputs             model outputs
   * @param pre_process_result  pre-process result
   * @return                    post-process result
   */
  [[nodiscard]] ufld::PostProcessResult PostProcess(const std::vector<Ort::Value>& outputs,
                                                    const PreProcessResult& pre_process_result) const override;
};
}  // namespace ufld::v1

// -------- Implementation --------

/**
 *
 * @param directory
 * @param variant
 * @return
 */
[[nodiscard]] std::filesystem::path ConstructModelPath(const std::filesystem::path& directory,
                                                       ufld::v1::Variant variant);

ufld::v1::LaneDetector::LaneDetector(const std::filesystem::path& model_directory, Variant variant)
    : LaneDetectorBase{ConstructModelPath(model_directory, variant), Version::kV1, variant} {
  config_ = [=]() -> std::unique_ptr<ufld::v1::IConfig> {
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

ufld::PreProcessResult ufld::v1::LaneDetector::PreProcess(const cv::Mat& image) const {
  const auto start_time = std::chrono::high_resolution_clock::now();

  const cv::Rect crop_area = ComputeCenterCrop(image, kInputAspectRatio);
  cv::Mat cropped_image = image(crop_area);
  cv::Mat resized_image{};
  cv::resize(cropped_image, resized_image, cv::Size{kInputWidth, kInputHeight});
  return {
      .processed_image = ColorPreProcess(resized_image),
      .original_size = image.size(),
      .crop_area = crop_area,
      .duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time),
  };
}

ufld::PostProcessResult ufld::v1::LaneDetector::PostProcess(const std::vector<Ort::Value>& outputs,
                                                            const PreProcessResult& pre_process_result) const {
  const auto start_time = std::chrono::high_resolution_clock::now();

  // We have exactly 1 output
  assert(outputs.size() == 1);
  const auto& output0 = outputs[0];  // e.g. [1, 201, 18, 4]
  assert(output0.IsTensor());
  assert(output0.GetTensorTypeAndShapeInfo().GetDimensionsCount() == 4);
  assert(output0.GetTensorTypeAndShapeInfo().GetElementType() ==
         ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT);
  assert(output0.GetTensorTypeAndShapeInfo().GetShape()[0] == 1);
  assert(output0.GetTensorTypeAndShapeInfo().GetShape()[1] == config_->row_anchor_cell_count);
  assert(output0.GetTensorTypeAndShapeInfo().GetShape()[2] == config_->row_anchor_count);
  assert(output0.GetTensorTypeAndShapeInfo().GetShape()[3] == kLaneCount);

  const std::span<const float> output0_raw(output0.GetTensorData<float>(),
                                           output0.GetTensorTypeAndShapeInfo().GetElementCount());
  // Interpret 4D tensor as 3D tensor with shape e.g. [201, 18, 4]
  const xt::xtensor<float, 3> output_tensor =
      xt::adapt(output0_raw.data(), output0_raw.size(), xt::no_ownership(),
                std::array<std::size_t, 3>{config_->row_anchor_cell_count, config_->row_anchor_count, kLaneCount});

  // As the last cell in each row signifies the absence of a lane,
  // we should store the argmax of each row.
  const xt::xtensor<std::size_t, 2> output_argmax = xt::argmax(output_tensor, 0);  // e.g. [18, 4]

  // Ideally, we would not use a temporary tensor here,
  // but I don't know how to write the ufls::utils::SoftMax() in a way
  // to accept both tensors and views.
  const xt::xtensor<float, 3> output_tensor_grid =
      xt::view(output_tensor, xt::range(xt::placeholders::_, -1), xt::all(),
               xt::all());  // e.g. [200, 18, 4]

  // Using mathematical expectation, we compute the lane locations in each row.
  auto probabilities = ufld::math::SoftMax<0>(output_tensor_grid);  // e.g. [200, 18, 4]
  auto indices = xt::arange<float>(1.0f, static_cast<float>(config_->row_anchor_cell_count))
                     .reshape({-1, 1, 1});               // e.g. [200, 1, 1]
  auto locations = xt::sum(probabilities * indices, 0);  // e.g. [18, 4]

  std::vector<Lane> lanes(kLaneCount);
  for (uint32_t lane_index = 0; lane_index < kLaneCount; ++lane_index) {
    const uint32_t kNotDetectedIndex = config_->row_anchor_cell_count - 1;
    // Skip if the lane is not detected
    const xt::xarray<std::size_t> detected_count =
        xt::count_nonzero(xt::not_equal(xt::view(output_argmax, xt::all(), lane_index), kNotDetectedIndex));
    if (detected_count(0) <= 2)
      continue;

    Lane& lane = lanes[lane_index];
    lane.reserve(detected_count(0));

    for (uint32_t class_index = 0; class_index < config_->row_anchor_count; ++class_index) {
      if (output_argmax(class_index, lane_index) == kNotDetectedIndex)
        continue;

      // 1st) Coordinates in the input image (800 x 288)
      auto y = static_cast<float>(config_->row_anchors[class_index]);
      auto x = locations(class_index, lane_index) / static_cast<float>(config_->row_anchor_cell_count) * kInputWidth;

      // 2nd) Coordinates in the cropped image before resizing
      const auto y_scale = static_cast<float>(pre_process_result.crop_area.height) / static_cast<float>(kInputHeight);
      const auto x_scale = static_cast<float>(pre_process_result.crop_area.width) / static_cast<float>(kInputWidth);
      y *= y_scale;
      x *= x_scale;

      // 3rd) Coordinates in the original image
      const auto crop_offset_y = (static_cast<float>(pre_process_result.original_size.height) -
                                  static_cast<float>(pre_process_result.crop_area.height)) /
                                 2.0f;
      const auto crop_offset_x = (static_cast<float>(pre_process_result.original_size.width) -
                                  static_cast<float>(pre_process_result.crop_area.width)) /
                                 2.0f;
      y += crop_offset_y;
      x += crop_offset_x;

      lane.emplace_back(x, y);
    }
  }

  ufld::PostProcessResult result{};
  result.duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);
  result.lanes = std::move(lanes);
  return result;
}

std::filesystem::path ConstructModelPath(const std::filesystem::path& directory, ufld::v1::Variant variant) {
  switch (variant) {
    case ufld::v1::Variant::kCULane: {
      constexpr auto kCULaneModelFile = "ufld_v1_culane_288x800.onnx";
      return directory / kCULaneModelFile;
    }
    case ufld::v1::Variant::kTuSimple: {
      constexpr auto kTuSimpleModelFile = "ufld_v1_tusimple_288x800.onnx";
      return directory / kTuSimpleModelFile;
    }
    default:
      throw std::invalid_argument{"Invalid model type"};
  }
}
