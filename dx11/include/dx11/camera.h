#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <opencv2/opencv.hpp>

#include "dx11/common.h"
#include "dx11/device.h"
#include "dx11/duplicator.h"
#include "dx11/frame_buffer.h"
#include "dx11/output.h"
#include "dx11/surface.h"
#include "dx11/timer.h"

namespace dx11 {

class Camera {
 public:
  Camera(Device& device, Output& output, uint32_t frame_buffer_capacity = 16,
         std::optional<cv::Rect> region = std::nullopt);

  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  Camera(Camera&&) = delete;
  Camera& operator=(Camera&&) = delete;

  void StartCapture(int32_t target_fps = 60,
                    std::optional<cv::Rect> region = std::nullopt);

  void StopCapture();

  [[nodiscard]] cv::Mat GetNewestFrame();

 private:
  Device& device_;
  Output& output_;
  Duplicator duplicator_;
  Surface surface_;

  cv::Rect region_;

  FrameBuffer frame_buffer_;

  std::thread capture_thread_{};
  std::atomic<bool> stop_capture_{false};

  void Capture(int32_t target_fps, const cv::Rect& region);

  std::optional<cv::Mat> Grab(const cv::Rect& region);
};

}  // namespace dx11
