#pragma once

#include <memory>
#include <mutex>
#include <thread>

#include "dx11/capture.h"
#include "dx11/common.h"
#include "ets2ld/arguments.h"
#include "ets2ld/event.h"
#include "ets2ld/lane_detection_result_buffer.h"
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

  // Threads
  Event stop_lane_detection_{};
  std::thread lane_detection_thread_{};

  LaneDetectionResultBuffer lane_detection_result_buffer_{};
  std::mutex lane_detection_result_mutex_{};
  Event lane_detection_result_available_{};

  void LaneDetectionThread();
};

}  // namespace ets2ld
