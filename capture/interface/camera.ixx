module;

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>

#include <opencv2/opencv.hpp>

export module capture.camera;

import capture.device;
import capture.duplicator;
import capture.frame_buffer;
import capture.output;
import capture.surface;

export namespace capture {
class Camera {
 public:
  Camera(Device& device, Output& output, std::uint32_t frame_buffer_capacity = 16,
         std::optional<cv::Rect> region = std::nullopt);

  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  Camera(Camera&&) = delete;
  Camera& operator=(Camera&&) = delete;

  void StartCapture(std::int32_t target_fps = 60,
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

  void Capture(std::int32_t target_fps, const cv::Rect& region);

  std::optional<cv::Mat> Grab(const cv::Rect& region);
};
}  // namespace capture
