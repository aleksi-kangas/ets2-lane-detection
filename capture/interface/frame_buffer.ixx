module;

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <vector>

#include <opencv2/opencv.hpp>

export module capture.frame_buffer;

export namespace capture {
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
