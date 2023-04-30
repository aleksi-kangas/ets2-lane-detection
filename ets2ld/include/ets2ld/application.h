#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "dx11/common.h"
#include "ets2ld/settings.h"
#include "ets2ld/ui.h"
#include "ufld/ufld.h"

namespace ets2ld {
class Application {
 public:
  Application() = default;
  ~Application();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;
  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  void Run();

 private:
  Settings settings_{};
  UI ui_{settings_};

  std::unique_ptr<ufld::ILaneDetector> lane_detector_{nullptr};
  std::atomic<bool> lane_detection_initializing_{false};
  std::atomic<bool> lane_detection_active_{false};
  std::atomic<bool> stop_lane_detection_signal_{false};
  std::thread lane_detection_thread_{};

  struct LaneDetectionResult {
    std::vector<ufld::Lane> lanes;
    cv::Mat frame;
    cv::Mat preview;
  };
  LaneDetectionResult lane_detection_result_{};
  std::atomic<bool> lane_detection_result_available_{false};
  std::mutex lane_detection_mutex_{};

  void LaneDetectionThread(CaptureSettings capture_settings);

  void HandleLaneDetectionEnableChanged();
  void HandleModelSettingsChanged();
};

}  // namespace ets2ld
