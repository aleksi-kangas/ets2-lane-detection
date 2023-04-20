#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "dx11/capture.h"
#include "dx11/common.h"
#include "ets2ld/arguments.h"
#include "ets2ld/ui.h"
#include "ufld/ufld.h"

namespace ets2ld {
class Application {
 public:
  explicit Application(Arguments arguments);
  ~Application();

  void Run();

 private:
  dx11::Capture capture_{};
  dx11::Camera* camera_{nullptr};  // Owned by capture_, holds current camera
  std::unique_ptr<ufld::ILaneDetector> lane_detector_{nullptr};
  UI ui_{};

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
};

}  // namespace ets2ld
