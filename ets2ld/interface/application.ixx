module;

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include <opencv2/opencv.hpp>

export module ets2ld.application;

import capture;
import ets2ld.ui;
import ufld;

export namespace ets2ld {
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
  capture::Settings capture_settings_{};
  ufld::Settings ufld_settings_{};
  UI ui_{};

  std::unique_ptr<ufld::ILaneDetector> lane_detector_{nullptr};
  std::atomic<bool> lane_detection_initializing_{false};
  std::atomic<bool> lane_detection_active_{false};
  std::atomic<bool> stop_lane_detection_signal_{false};
  std::thread lane_detection_thread_{};

  ufld::LaneDetectionResult lane_detection_result_{};
  std::atomic<bool> lane_detection_result_available_{false};
  std::mutex lane_detection_mutex_{};

  void LaneDetectorInitializerThread(capture::Settings capture_settings,
                                     ufld::Settings ufld_settings);
  void LaneDetectorThread(capture::Settings capture_settings);

  void HandleLaneDetectionEnableChanged(bool enable,
                                        capture::Settings capture_settings,
                                        ufld::Settings ufld_settings);
};

}  // namespace ets2ld
