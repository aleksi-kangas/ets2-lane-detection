module;

#include <chrono>
#include <filesystem>
#include <optional>
#include <variant>
#include <vector>

#include <opencv2/opencv.hpp>

export module ufld.ld;

export namespace ufld {

/**
 *
 */
enum class Version { kV1 };

namespace v1 {
/**
 *
 */
enum class Variant { kCULane, kTuSimple };
}  // namespace v1

/**
 *
 */
using Lane = std::vector<cv::Point2f>;

/**
 *
 */
struct LaneDetectionStatistics {
  std::chrono::milliseconds pre_process_duration{0};
  std::chrono::milliseconds inference_duration{0};
  std::chrono::milliseconds post_process_duration{0};
};

/**
 *
 */
struct LaneDetectionResult {
  std::vector<Lane> lanes{};
  std::optional<cv::Mat> preview{std::nullopt};
  LaneDetectionStatistics statistics{};
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
  [[nodiscard]] virtual LaneDetectionResult Detect(
      const cv::Mat& image, std::optional<cv::Mat> preview) = 0;

  /**
   *
   * @return
   */
  [[nodiscard]] virtual std::filesystem::path ModelDirectory() const = 0;

  /**
   *
   * @return
   */
  [[nodiscard]] virtual Version ModelVersion() const = 0;

  /**
   *
   * @return
   */
  [[nodiscard]] virtual std::variant<v1::Variant> ModelVariant() const = 0;

 protected:
  ILaneDetector() = default;
};

}  // namespace ufld
