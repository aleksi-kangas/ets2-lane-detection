module;

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

export module ufld.v1;

import ufld.base;
import ufld.v1.config;

export namespace ufld::v1 {
/**
 *
 */
class LaneDetector final : public ILaneDetector {
 public:
  /**
   *
   * @param model_directory
   * @param variant
   */
  LaneDetector(const std::filesystem::path& model_directory,
               ufld::v1::Variant variant);

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

  std::unique_ptr<const IConfig> config_{nullptr};

  /**
   *
   * @param image
   * @return
   */
  [[nodiscard]] ufld::PreProcessResult PreProcess(const cv::Mat& image) const override;

  /**
   *
   * @param outputs
   * @param pre_process_result
   * @return
   */
  [[nodiscard]] ufld::PostProcessResult PostProcess(
      const std::vector<Ort::Value>& outputs,
      const PreProcessResult& pre_process_result) const override;
};
}  // namespace ufld::v1
