#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

namespace ufld {

using Lane = std::vector<cv::Point>;

void VisualizeLanes(const std::vector<Lane>& lanes, cv::Mat& image);

class ILaneDetector {
 public:
  ILaneDetector() = default;
  virtual ~ILaneDetector() {
    if (input_name_) {
      free(input_name_);
    }
    if (output_name_) {
      free(output_name_);
    }
  }

  ILaneDetector(const ILaneDetector&) = delete;
  ILaneDetector& operator=(const ILaneDetector&) = delete;

  ILaneDetector(ILaneDetector&&) = delete;
  ILaneDetector& operator=(ILaneDetector&&) = delete;

  [[nodiscard]] std::vector<Lane> Detect(const cv::Mat& image) {
    auto input = Preprocess(image);
    auto predictions = Inference(input);
    return PredictionsToLanes(predictions, image.cols, image.rows);
  }

 protected:
  Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "UFLD"};
  Ort::Session session_{nullptr};
  Ort::AllocatorWithDefaultOptions allocator_{};

  // Input
  char* input_name_{nullptr};
  std::vector<int64_t> input_dimensions_{};
  int64_t input_tensor_size_{};
  std::vector<float> input_tensor_data_{};

  // Output
  char* output_name_{nullptr};
  std::vector<int64_t> output_dimensions_{};
  int64_t output_tensor_size_{};
  std::vector<float> output_tensor_data_{};

  [[nodiscard]] virtual Ort::Value Preprocess(const cv::Mat& image) = 0;

  [[nodiscard]] virtual Ort::Value Inference(const Ort::Value& input) = 0;

  [[nodiscard]] virtual std::vector<Lane> PredictionsToLanes(const Ort::Value& predictions, int32_t image_width,
                                                             int32_t image_height) = 0;
};

}  // namespace ufld
