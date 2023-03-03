#include "d3d11_capture/frame_buffer.h"

namespace d3d11_capture {

FrameBuffer::FrameBuffer(int32_t capacity) : capacity_{capacity} {}

void FrameBuffer::AddFrame(cv::Mat frame) {
  if (frames_.size() >= capacity_) {
    frames_.pop_back();
  }
  frames_.push_front(std::move(frame));
}

[[nodiscard]] std::optional<cv::Mat> FrameBuffer::GetNewestFrame() {
  if (frames_.empty()) {
    return std::nullopt;
  }
  return frames_.front();
}

}  // namespace d3d11_capture
