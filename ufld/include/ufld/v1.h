#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include "ufld.h"

namespace ufld::v1 {

enum class ModelType { kCULane, kTuSimple };

struct IConfig {
 public:
  int32_t griding_num;  // The number of cells in a grid row, where the last cell indicates no lane found in the row
  int32_t cls_num_per_lane;  // Number of classes (= row anchors) per lane
  std::vector<int32_t> row_anchors;

 protected:
  IConfig(int32_t griding_num, int32_t cls_num_per_lane, std::vector<int32_t> row_anchors)
      : griding_num{griding_num}, cls_num_per_lane{cls_num_per_lane}, row_anchors{std::move(row_anchors)} {}
};

struct CULaneConfig : public IConfig {
  CULaneConfig()
      : IConfig{200 + 1,
                18,
                {121, 131, 141, 150, 160, 170, 180, 189, 199, 209, 219, 228, 238, 248, 258, 267, 277, 287}} {}
};

struct TuSimpleConfig : public IConfig {
  TuSimpleConfig()
      : IConfig{100 + 1, 56, {64,  68,  72,  76,  80,  84,  88,  92,  96,  100, 104, 108, 112, 116,
                              120, 124, 128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172,
                              176, 180, 184, 188, 192, 196, 200, 204, 208, 212, 216, 220, 224, 228,
                              232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284}} {}
};

class LaneDetector final : public ILaneDetector {
 public:
  explicit LaneDetector(ModelType model_type);

  LaneDetector(const LaneDetector&) = delete;
  LaneDetector& operator=(const LaneDetector&) = delete;

  LaneDetector(LaneDetector&&) = delete;
  LaneDetector& operator=(LaneDetector&&) = delete;

 private:
  static constexpr int32_t kInputWidth = 800;
  static constexpr int32_t kInputHeight = 288;
  static constexpr int32_t kLaneCount = 4;

  std::unique_ptr<const IConfig> config_{nullptr};

  [[nodiscard]] Ort::Value Preprocess(const cv::Mat& image) override;

  [[nodiscard]] Ort::Value Inference(const Ort::Value& input) override;

  [[nodiscard]] std::vector<Lane> PredictionsToLanes(const Ort::Value& predictions, int32_t image_width,
                                                     int32_t image_height) override;

  void InitSession(const std::filesystem::path& model_path);

  void InitModelInfo();
};

}  // namespace ufld::v1
