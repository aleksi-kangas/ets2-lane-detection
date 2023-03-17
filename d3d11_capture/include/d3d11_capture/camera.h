#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <opencv2/opencv.hpp>

#include "d3d11_capture/common.h"
#include "d3d11_capture/device.h"
#include "d3d11_capture/duplicator.h"
#include "d3d11_capture/event.h"
#include "d3d11_capture/frame_buffer.h"
#include "d3d11_capture/output.h"
#include "d3d11_capture/region.h"
#include "d3d11_capture/surface.h"

namespace d3d11_capture {

class Camera {
 public:
  Camera(Device& device, Output& output,
         std::optional<Region> region = std::nullopt,
         int32_t frame_buffer_capacity = 64);

  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  Camera(Camera&&) = delete;
  Camera& operator=(Camera&&) = delete;

  void StartCapture(std::optional<Region> region = std::nullopt);

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

  void Capture(const Region& region);

  std::optional<cv::Mat> Grab(const Region& region);
};

}  // namespace d3d11_capture
