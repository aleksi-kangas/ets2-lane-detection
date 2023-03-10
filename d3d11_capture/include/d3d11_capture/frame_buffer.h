#pragma once

#include <cstdint>
#include <deque>
#include <optional>

#include <opencv2/opencv.hpp>

namespace d3d11_capture {

/**
 * Frame buffer for storing frames. Note, this class is not thread-safe.
 * Thread-safety is the responsibility of the caller.
 */
class FrameBuffer {
 public:
  explicit FrameBuffer(int32_t capacity = 64);

  FrameBuffer(const FrameBuffer&) = delete;
  FrameBuffer& operator=(const FrameBuffer&) = delete;

  void AddFrame(cv::Mat frame);

  [[nodiscard]] std::optional<cv::Mat> GetNewestFrame();

 private:
  int32_t capacity_;
  std::deque<cv::Mat> frames_{};  // TODO Use a vector for contiguous memory?
};

}  // namespace d3d11_capture
