module;

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <vector>

#include <opencv2/opencv.hpp>

module capture.frame_buffer;

capture::FrameBuffer::FrameBuffer(std::uint32_t capacity) : capacity_{capacity} {
  frames_.resize(capacity_);
}

void capture::FrameBuffer::Push(cv::Mat frame) {
  std::unique_lock lock{mutex_};
  frames_[head_] = std::move(frame);
  if (is_full_) {
    tail_ = (tail_ + 1) % capacity_;
  }
  head_ = (head_ + 1) % capacity_;
  is_full_ = head_ == tail_;
  cv_.notify_one();
}

cv::Mat capture::FrameBuffer::GetNewest() {
  std::unique_lock lock{mutex_};
  cv_.wait(lock, [this] { return is_full_; });
  return frames_[tail_];
}
