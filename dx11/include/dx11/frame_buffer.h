#pragma once

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <vector>

#include <opencv2/opencv.hpp>

namespace dx11 {
class FrameBuffer {
 public:
  explicit FrameBuffer(uint32_t capacity);
  FrameBuffer(const FrameBuffer&) = delete;
  FrameBuffer& operator=(const FrameBuffer&) = delete;
  FrameBuffer(FrameBuffer&&) = delete;
  FrameBuffer& operator=(FrameBuffer&&) = delete;

  void Push(cv::Mat frame);
  [[nodiscard]] cv::Mat GetNewest();

 private:
  uint32_t capacity_{16};
  std::vector<cv::Mat> frames_{};
  bool is_full_{false};
  uint32_t head_{0};
  uint32_t tail_{0};

  std::mutex mutex_{};
  std::condition_variable cv_{};
};
}  // namespace dx11
