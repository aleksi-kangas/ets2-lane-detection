#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

namespace ufld {

enum class Version { kV1 = 1 };

using Lane = std::vector<cv::Point>;

void VisualizeLanes(const std::vector<Lane>& lanes, cv::Mat& image);

class ILaneDetector {
 public:
  ILaneDetector() = default;
  virtual ~ILaneDetector() = default;

  ILaneDetector(const ILaneDetector&) = delete;
  ILaneDetector& operator=(const ILaneDetector&) = delete;

  ILaneDetector(ILaneDetector&&) = delete;
  ILaneDetector& operator=(ILaneDetector&&) = delete;

  [[nodiscard]] std::vector<Lane> Detect(const cv::Mat& image);

 protected:
  Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "UFLD"};
  Ort::Session session_{nullptr};
  Ort::AllocatorWithDefaultOptions allocator_{};

  // Input (always 1 image)
  std::string input_name_{};
  std::vector<int64_t> input_dimensions_{};
  int64_t input_tensor_size_{};
  std::vector<float> input_tensor_data_{};

  // Output(s)
  std::vector<std::string> output_names_{};
  std::vector<std::vector<int64_t>> output_dimensions_{};
  std::vector<int64_t> output_tensor_sizes_{};
  std::vector<std::vector<float>> output_tensor_data_{};

  [[nodiscard]] virtual Ort::Value Preprocess(const cv::Mat& image) = 0;

  [[nodiscard]] virtual std::vector<Ort::Value> Inference(
      const Ort::Value& input) = 0;

  [[nodiscard]] virtual std::vector<Lane> PredictionsToLanes(
      const std::vector<Ort::Value>& outputs, int32_t image_width,
      int32_t image_height) = 0;
};

}  // namespace ufld
