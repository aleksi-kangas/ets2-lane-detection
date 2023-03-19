#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <opencv2/opencv.hpp>

#include "dx11/common.h"
#include "dx11/device.h"
#include "dx11/duplicator.h"
#include "dx11/event.h"
#include "dx11/frame_buffer.h"
#include "dx11/output.h"
#include "dx11/region.h"
#include "dx11/surface.h"
#include "dx11/timer.h"

namespace dx11 {

class Camera {
 public:
  Camera(Device& device, Output& output,
         std::optional<Region> region = std::nullopt,
         uint32_t frame_buffer_capacity = 64);

  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  Camera(Camera&&) = delete;
  Camera& operator=(Camera&&) = delete;

  void StartCapture(int32_t target_fps = 60, std::optional<Region> region = std::nullopt);

  void StopCapture();

  [[nodiscard]] cv::Mat GetLatestFrame();

 private:
  Device& device_;
  Output& output_;
  Duplicator duplicator_;
  Surface surface_;

  Region region_;

  FrameBuffer frame_buffer_;

  std::mutex mutex_{};
  std::thread capture_thread_{};
  Event stop_capture_{};
  Event frame_available_{};

  void Capture(int32_t target_fps, const Region& region);

  std::optional<cv::Mat> Grab(const Region& region);
};

}  // namespace dx11
