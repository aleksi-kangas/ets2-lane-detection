module;

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <vector>

#include <opencv2/opencv.hpp>

module capture:frame_buffer;

namespace capture {
class FrameBuffer {
 public:
  explicit FrameBuffer(std::uint32_t capacity);
  FrameBuffer(const FrameBuffer&) = delete;
  FrameBuffer& operator=(const FrameBuffer&) = delete;
  FrameBuffer(FrameBuffer&&) = delete;
  FrameBuffer& operator=(FrameBuffer&&) = delete;

  void Push(cv::Mat frame);
  [[nodiscard]] cv::Mat GetNewest();

 private:
  std::uint32_t capacity_{16};
  std::vector<cv::Mat> frames_{};
  bool is_full_{false};
  std::uint32_t head_{0};
  std::uint32_t tail_{0};

  std::mutex mutex_{};
  std::condition_variable cv_{};
};
}  // namespace capture

// -------- Implementation --------

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
