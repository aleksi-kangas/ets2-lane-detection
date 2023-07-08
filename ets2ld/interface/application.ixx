module;

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include <opencv2/opencv.hpp>

export module ets2ld.application;

import ets2ld.settings;
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
  Settings settings_{};
  UI ui_{settings_};

  std::unique_ptr<ufld::ILaneDetector> lane_detector_{nullptr};
  std::atomic<bool> lane_detection_initializing_{false};
  std::atomic<bool> lane_detection_active_{false};
  std::atomic<bool> stop_lane_detection_signal_{false};
  std::thread lane_detection_thread_{};

  ufld::LaneDetectionResult lane_detection_result_{};
  std::atomic<bool> lane_detection_result_available_{false};
  std::mutex lane_detection_mutex_{};

  void InitializeAndStartLaneDetector(const Settings& settings);
  void LaneDetectionThread(CaptureSettings capture_settings);

  void HandleLaneDetectionEnableChanged();
};

}  // namespace ets2ld
