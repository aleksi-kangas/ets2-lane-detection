#include "dx11/camera.h"

#include <cassert>

namespace dx11 {

Camera::Camera(Device& device, Output& output, std::optional<cv::Rect> region,
               uint32_t frame_buffer_capacity)
    : device_{device},
      output_{output},
      duplicator_{device, output},
      surface_{device, output},
      region_{
          region.value_or(cv::Rect{0, 0, output_.Width(), output_.Height()})},
      frame_buffer_{frame_buffer_capacity} {}

void Camera::StartCapture(int32_t target_fps, std::optional<cv::Rect> region) {
  if (!region.has_value()) {
    region = region_;
  }
  capture_thread_ = std::thread(
      [this, target_fps, region]() { Capture(target_fps, region.value()); });
}

void Camera::StopCapture() {
  stop_capture_.Set();
  capture_thread_.join();
  frame_available_.Clear();
  stop_capture_.Clear();
}

cv::Mat Camera::GetLatestFrame() {
  frame_available_.Wait();
  std::lock_guard<std::mutex> lock{mutex_};
  const auto frame = frame_buffer_.GetNewestFrame();
  // This should always pass, as we wait for a frame to be available
  assert(frame.has_value());
  frame_available_.Clear();
  return frame.value_or(cv::Mat{output_.Height(), output_.Width(), CV_8UC4});
}

void Camera::Capture(int32_t target_fps, const cv::Rect& region) {
  Timer timer{target_fps};
  while (!stop_capture_.IsSet()) {
    timer.Wait();
    const std::optional<cv::Mat> frame = Grab(region);
    if (frame) {
      std::lock_guard<std::mutex> lock{mutex_};
      frame_buffer_.AddFrame(frame.value());
      frame_available_.Set();
    }
  }
}

std::optional<cv::Mat> Camera::Grab(const cv::Rect& region) {
  const bool is_frame_updated = duplicator_.UpdateFrame();
  if (!is_frame_updated) {
    return std::nullopt;
  }
  device_.D3D11DeviceImmediateContext()->CopyResource(
      surface_.D3D11Texture2D(), duplicator_.D3D11Texture2D());
  duplicator_.ReleaseFrame();
  const auto rect = surface_.Map();
  const auto [width, height] = output_.Resolution();
  cv::Mat frame{height, width, CV_8UC4, rect.pBits,
                static_cast<size_t>(rect.Pitch)};
  surface_.UnMap();

  auto Crop = [](cv::Mat& src, cv::Rect region) {
    cv::Mat cropped_ref(src, region);
    cv::Mat cropped;
    cropped_ref.copyTo(cropped);
    return cropped;
  };

  // Crop if the region differs from output resolution
  if (region != cv::Rect{0, 0, width, height}) {
    return Crop(frame, region);
  }
  return frame;
}

}  // namespace dx11
