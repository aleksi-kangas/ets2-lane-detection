#include "dx11/camera.h"

#include <cassert>

namespace dx11 {

Camera::Camera(Device& device, Output& output, uint32_t frame_buffer_capacity,
               std::optional<cv::Rect> region)
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
  stop_capture_ = true;
  capture_thread_.join();
  stop_capture_ = false;
}

cv::Mat Camera::GetNewestFrame() {
  return frame_buffer_.GetNewest();
}

void Camera::Capture(int32_t target_fps, const cv::Rect& region) {
  Timer timer{target_fps};
  while (!stop_capture_) {
    timer.Wait();
    const std::optional<cv::Mat> frame = Grab(region);
    if (frame) {
      frame_buffer_.Push(frame.value());
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
