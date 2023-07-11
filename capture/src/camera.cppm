module;

#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>

#include <opencv2/opencv.hpp>

module capture.camera;

import capture.device;
import capture.duplicator;
import capture.frame_buffer;
import capture.output;
import capture.surface;
import capture.timer;

capture::Camera::Camera(capture::Device& device, capture::Output& output,
                        std::uint32_t frame_buffer_capacity,
                        std::optional<cv::Rect> region)
    : device_{device},
      output_{output},
      duplicator_{device, output},
      surface_{device, output},
      region_{
          region.value_or(cv::Rect{0, 0, output_.Width(), output_.Height()})},
      frame_buffer_{frame_buffer_capacity} {}

void capture::Camera::StartCapture(std::int32_t target_fps,
                                   std::optional<cv::Rect> region) {
  if (!region.has_value()) {
    region = region_;
  }
  capture_thread_ = std::thread(
      [this, target_fps, region]() { Capture(target_fps, region.value()); });
}

void capture::Camera::StopCapture() {
  stop_capture_ = true;
  capture_thread_.join();
  stop_capture_ = false;
}

cv::Mat capture::Camera::GetNewestFrame() {
  return frame_buffer_.GetNewest();
}

void capture::Camera::Capture(std::int32_t target_fps, const cv::Rect& region) {
  capture::Timer timer{target_fps};
  while (!stop_capture_) {
    timer.Wait();
    const std::optional<cv::Mat> frame = Grab(region);
    if (frame) {
      frame_buffer_.Push(frame.value());
    }
  }
}

std::optional<cv::Mat> capture::Camera::Grab(const cv::Rect& region) {
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

  // BGRA -> RGBA
  cv::Mat frame_rgba{};
  cv::cvtColor(frame, frame_rgba, cv::COLOR_BGRA2RGBA);

  auto Crop = [](cv::Mat& src, cv::Rect region) {
    cv::Mat cropped_ref(src, region);
    cv::Mat cropped;
    cropped_ref.copyTo(cropped);
    return cropped;
  };

  // Crop if the region differs from output resolution
  if (region != cv::Rect{0, 0, width, height}) {
    return Crop(frame_rgba, region);
  }

  return frame_rgba;
}