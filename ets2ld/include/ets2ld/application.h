#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "dx11/capture.h"
#include "dx11/common.h"
#include "ets2ld/settings.h"
#include "ets2ld/ui.h"
#include "ufld/ufld.h"

namespace ets2ld {
class Application {
 public:
  explicit Application(Settings settings);
  ~Application();

  void Run();

 private:
  Settings settings_;
  UI ui_{settings_};

  // Capture
  dx11::Capture capture_{};
  dx11::Camera* camera_{nullptr};  // Owned by capture_, holds current camera

  // Lane Detection
  bool lane_detection_enabled_{false};
  std::atomic<bool> lane_detection_initializing_{false};
  std::unique_ptr<ufld::ILaneDetector> lane_detector_{nullptr};

  // Threads
  std::atomic<bool> stop_lane_detection_{false};
  std::thread lane_detection_thread_{};

  struct LaneDetectionResult {
    std::vector<ufld::Lane> lanes;
    cv::Mat frame;
    cv::Mat preview;
  };
  LaneDetectionResult lane_detection_result_{};
  std::atomic<bool> lane_detection_result_available_{false};
  std::mutex lane_detection_mutex_{};

  void LaneDetectionThread();

  void HandleChangeInLaneDetectionEnabled();
};

}  // namespace ets2ld
