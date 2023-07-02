#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include "ufld.h"

namespace ufld::v1 {
struct IConfig {
 public:
  // Number of cells in a single row anchor, i.e. the number of columns in the grid
  uint32_t row_anchor_cell_count;
  // Number of row anchors (= classes) per lane
  std::uint32_t row_anchor_count;
  // Each lane consist of row anchors, i.e. rows of cells
  std::vector<uint32_t> row_anchors;

 protected:
  IConfig(uint32_t row_anchor_cell_count, std::vector<uint32_t> row_anchors)
      : row_anchor_cell_count{row_anchor_cell_count},
        row_anchor_count{static_cast<uint32_t>(row_anchors.size())},
        row_anchors{std::move(row_anchors)} {}
};

struct CULaneConfig : public IConfig {
  CULaneConfig()
      : IConfig{200 + 1,
                {121, 131, 141, 150, 160, 170, 180, 189, 199, 209, 219, 228,
                 238, 248, 258, 267, 277, 287}} {}
};

struct TuSimpleConfig : public IConfig {
  TuSimpleConfig()
      : IConfig{100 + 1,
                {64,  68,  72,  76,  80,  84,  88,  92,  96,  100, 104, 108,
                 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156,
                 160, 164, 168, 172, 176, 180, 184, 188, 192, 196, 200, 204,
                 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
                 256, 260, 264, 268, 272, 276, 280, 284}} {}
};

class LaneDetector final : public ILaneDetector {
 public:
  LaneDetector(const std::filesystem::path& model_directory, Variant variant);

  LaneDetector(const LaneDetector&) = delete;
  LaneDetector& operator=(const LaneDetector&) = delete;

  LaneDetector(LaneDetector&&) = delete;
  LaneDetector& operator=(LaneDetector&&) = delete;

 private:
  static constexpr int32_t kInputWidth = 800;
  static constexpr int32_t kInputHeight = 288;
  static constexpr float kInputAspectRatio =
      static_cast<float>(kInputWidth) / static_cast<float>(kInputHeight);
  static constexpr uint32_t kLaneCount = 4;
  static constexpr auto kCULaneModelFile = "ufld_v1_culane_288x800.onnx";
  static constexpr auto kTuSimpleModelFile = "ufld_v1_tusimple_288x800.onnx";

  std::unique_ptr<const IConfig> config_{nullptr};

  [[nodiscard]] PreprocessInfo Preprocess(const cv::Mat& image) override;

  [[nodiscard]] std::vector<Lane> PredictionsToLanes(
      const std::vector<Ort::Value>& outputs,
      const PreprocessInfo& preprocess_info) override;

  [[nodiscard]] static std::filesystem::path ConstructModelPath(
      const std::filesystem::path& directory, Variant variant);
};

}  // namespace ufld::v1
