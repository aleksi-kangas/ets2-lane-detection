#include "d3d11_capture/camera.h"

#include <cassert>

namespace d3d11_capture {

Camera::Camera(Device& device, Output& output, std::optional<Region> region, int32_t frame_buffer_capacity)
    : device_{device},
      output_{output},
      stage_surface_{device, output},
      duplicator_{device, output},
      region_{region.value_or(Region{0, 0, output_.Width(), output_.Height()})},
      frame_buffer_{frame_buffer_capacity} {}

void Camera::StartCapture(std::optional<Region> region) {
  if (!region.has_value()) {
    region = region_;
  }
  capture_thread_ = std::thread([this, region]() { Capture(region.value()); });
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
  assert(frame.has_value());  // This should always pass, as we wait for a frame to be available
  frame_available_.Clear();
  return frame.value_or(cv::Mat{output_.Height(), output_.Width(), CV_8UC4});
}

void Camera::Capture(const Region& region) {
  // TODO Target FPS

  while (!stop_capture_.IsSet()) {
    // TODO Target FPS

    // Get a frame from the duplicator
    const std::optional<cv::Mat> frame = Grab(region);
    if (frame) {
      std::lock_guard<std::mutex> lock{mutex_};
      frame_buffer_.AddFrame(frame.value());
      frame_available_.Set();
    }
  }
}

std::optional<cv::Mat> Camera::Grab(const Region& region) {
  const bool is_frame_updated = duplicator_.UpdateFrame();
  if (!is_frame_updated) {
    return std::nullopt;
  }
  device_.D3D11DeviceImmediateContext()->CopyResource(stage_surface_.D3D11Texture2D(), duplicator_.D3D11Texture2D());
  duplicator_.ReleaseFrame();
  const auto rect = stage_surface_.Map();
  const auto [width, height] = output_.Resolution();
  cv::Mat frame{height, width, CV_8UC4, rect.pBits, static_cast<size_t>(rect.Pitch)};
  stage_surface_.UnMap();

  auto Crop = [](cv::Mat& src, Region region) {
    cv::Mat cropped_ref(src, cv::Rect{region.left, region.top, region.right, region.bottom});
    cv::Mat cropped;
    cropped_ref.copyTo(cropped);
    return cropped;
  };

  // Crop if the region differs from output resolution
  if (region.left != 0 || region.top != 0 || region.right != width || region.bottom != height) {
    return Crop(frame, region);
  }
  return frame;
}

}  // namespace d3d11_capture
