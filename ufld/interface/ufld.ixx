module;

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

#include <opencv2/opencv.hpp>

export module ufld;

export import :math;

export namespace ufld {
/**
 * UFLD version
 */
enum class Version { kV1 };

namespace v1 {
/**
 * UFLD V1 variant
 */
enum class Variant { kCULane, kTuSimple };
}  // namespace v1

/**
 * UFLD settings
 */
struct Settings {
  std::filesystem::path model_directory{};
  Version version{Version::kV1};
  std::variant<v1::Variant> variant{v1::Variant::kCULane};
};

/**
 * A single lane (lane divider) is represented as a vector of points.
 */
using Lane = std::vector<cv::Point2f>;

/**
 * Duration statistics for lane detection.
 */
struct LaneDetectionStatistics {
  std::chrono::milliseconds pre_process_duration{0};
  std::chrono::milliseconds inference_duration{0};
  std::chrono::milliseconds post_process_duration{0};
};

/**
 * Lane detection result.
 */
struct LaneDetectionResult {
  std::vector<Lane> lanes{};
  std::optional<cv::Mat> preview{std::nullopt};
  LaneDetectionStatistics statistics{};
};

/**
 * Lane detector interface.
 */
class ILaneDetector {
 public:
  virtual ~ILaneDetector() = default;

  ILaneDetector(const ILaneDetector&) = delete;
  ILaneDetector& operator=(const ILaneDetector&) = delete;

  ILaneDetector(ILaneDetector&&) = delete;
  ILaneDetector& operator=(ILaneDetector&&) = delete;

  /**
   * Detect lanes in the given image.
   * @param image   input image
   * @param preview optional preview image (visualization)
   * @return        lane detection result
   */
  [[nodiscard]] virtual LaneDetectionResult Detect(
      const cv::Mat& image, std::optional<cv::Mat> preview) = 0;

  /**
   * Get the model directory.
   * @return    model directory
   */
  [[nodiscard]] virtual std::filesystem::path ModelDirectory() const = 0;

  /**
   * Get the model version.
   * @return    model version
   */
  [[nodiscard]] virtual Version ModelVersion() const = 0;

  /**
   * Get the model variant.
   * @return    model variant
   */
  [[nodiscard]] virtual std::variant<v1::Variant> ModelVariant() const = 0;

 protected:
  ILaneDetector() = default;
};

/**
 * Make a lane detector.
 * @param settings  lane detector settings
 * @return          lane detector
 */
[[nodiscard]] std::unique_ptr<ufld::ILaneDetector> MakeLaneDetector(
    const Settings& settings);

}  // namespace ufld
