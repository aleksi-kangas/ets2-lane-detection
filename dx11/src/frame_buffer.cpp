#include "dx11/frame_buffer.h"

namespace dx11 {
FrameBuffer::FrameBuffer(uint32_t capacity) : capacity_{capacity} {
  frames_.resize(capacity_);
}

void FrameBuffer::Push(cv::Mat frame) {
  std::unique_lock lock{mutex_};
  frames_[head_] = std::move(frame);
  if (is_full_) {
    tail_ = (tail_ + 1) % capacity_;
  }
  head_ = (head_ + 1) % capacity_;
  is_full_ = head_ == tail_;
  cv_.notify_one();
}

cv::Mat FrameBuffer::GetNewest() {
  std::unique_lock lock{mutex_};
  cv_.wait(lock, [this] { return is_full_; });
  return frames_[tail_];
}

}  // namespace dx11
