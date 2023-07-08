module;

#include <cstdint>
#include <filesystem>
#include <string>
#include <variant>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

export module ufld.base;

export namespace ufld {
enum class Version { kV1 };
namespace v1 {
enum class Variant { kCULane, kTuSimple };
}  // namespace v1

using Lane = std::vector<cv::Point2f>;

struct PreprocessInfo {
  cv::Mat preprocessed_image{};
  cv::Size original_size{};
  cv::Rect crop_area{};
};

class ILaneDetector {
 public:
  virtual ~ILaneDetector() = default;

  ILaneDetector(const ILaneDetector&) = delete;
  ILaneDetector& operator=(const ILaneDetector&) = delete;

  ILaneDetector(ILaneDetector&&) = delete;
  ILaneDetector& operator=(ILaneDetector&&) = delete;

  /**
   *
   * @param image
   * @param preview
   * @return
   */
  [[nodiscard]] std::vector<Lane> Detect(const cv::Mat& image,
                                         cv::Mat* preview = nullptr);

  /**
   *
   * @return
   */
  [[nodiscard]] std::filesystem::path ModelDirectory() const;

  /**
   *
   * @return
   */
  [[nodiscard]] Version ModelVersion() const;

  /**
   *
   * @return
   */
  [[nodiscard]] std::variant<v1::Variant> ModelVariant() const;

 protected:
  /**
   *
   * @param model_path
   * @param version
   * @param variant
   */
  explicit ILaneDetector(const std::filesystem::path& model_path,
                         Version version, std::variant<v1::Variant> variant);

  std::filesystem::path model_directory_{};
  Version version_{};
  std::variant<v1::Variant> variant_{};

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

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] virtual PreprocessInfo Preprocess(const cv::Mat& image) = 0;

  /**
   *
   * @param image
   * @param input_aspect_ratio
   * @return
   */
  [[nodiscard]] static cv::Rect ComputeCenterCrop(const cv::Mat& image,
                                                  float input_aspect_ratio);

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] static cv::Mat ColorPreprocess(const cv::Mat& image);

  [[nodiscard]] virtual std::vector<Lane> PredictionsToLanes(
      const std::vector<Ort::Value>& outputs,
      const PreprocessInfo& preprocess_info) = 0;

 private:
  /**
   *
   * @param model_path
   */
  void InitializeSession(const std::filesystem::path& model_path);

  /**
   *
   */
  void InitializeInput();

  /**
   *
   */
  void InitializeOutputs();

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] Ort::Value InitializeInputTensor(const cv::Mat& image);

  /**
   *
   * @return
   */
  [[nodiscard]] std::vector<Ort::Value> InitializeOutputTensors();

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] std::vector<Ort::Value> Inference(const cv::Mat& image);
};

}  // namespace ufld
