#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include "ufld.h"

namespace ufld::v1 {

enum class ModelType { kCULane, kTuSimple };

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

  [[nodiscard]] Ort::Value Preprocess(const cv::Mat& image) override;

  [[nodiscard]] Ort::Value Inference(const Ort::Value& input) override;

  [[nodiscard]] std::vector<Lane> PredictionsToLanes(const Ort::Value& predictions) override;

  void InitSession(const std::filesystem::path& model_path);

  void InitModelInfo();
};

}  // namespace ufld::v1
