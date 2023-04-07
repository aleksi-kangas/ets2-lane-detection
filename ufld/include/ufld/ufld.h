#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
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
  virtual ~ILaneDetector() = default;

  ILaneDetector(const ILaneDetector&) = delete;
  ILaneDetector& operator=(const ILaneDetector&) = delete;

  ILaneDetector(ILaneDetector&&) = delete;
  ILaneDetector& operator=(ILaneDetector&&) = delete;

  [[nodiscard]] std::vector<Lane> Detect(const cv::Mat& image);

 protected:
  explicit ILaneDetector(const std::filesystem::path& model_path);

  Ort::Env env_{ORT_LOGGING_LEVEL_WARNING, "UFLD"};
  Ort::Session session_{nullptr};
  Ort::AllocatorWithDefaultOptions allocator_{};
  std::mutex detection_mutex_{};

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

  [[nodiscard]] virtual cv::Mat Preprocess(const cv::Mat& image) = 0;
  static void ColorPreprocess(cv::Mat& image);

  [[nodiscard]] virtual std::vector<Lane> PredictionsToLanes(
      const std::vector<Ort::Value>& outputs, int32_t image_width,
      int32_t image_height) = 0;

 private:
  void InitializeSession(const std::filesystem::path& model_path);
  void InitializeInput();
  void InitializeOutputs();

  [[nodiscard]] Ort::Value InitializeInputTensor(const cv::Mat& image);
  [[nodiscard]] std::vector<Ort::Value> InitializeOutputTensors();
  [[nodiscard]] std::vector<Ort::Value> Inference(const cv::Mat& image);
};

}  // namespace ufld
