module;

#include <cstdint>
#include <memory>
#include <optional>

#include <atlbase.h>
#include <dxgi1_2.h>
#include <opencv2/opencv.hpp>

export module capture:camera;

import :device;
import :duplicator;
import :output;
import :surface;
import :timer;
import :utils;

namespace capture {
export struct Settings {
  std::uint32_t device_index{0};
  std::uint32_t output_index{0};
  std::int32_t target_fps{10};
  cv::Rect region{0, 0, utils::QueryPrimaryMonitorResolution().first, utils::QueryPrimaryMonitorResolution().second};
};

export class Camera {
 public:
  explicit Camera(const Settings& settings);

  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;

  Camera(Camera&&) = delete;
  Camera& operator=(Camera&&) = delete;

  [[nodiscard]] std::optional<cv::Mat> CaptureFrame(const cv::Rect& region) const;

 private:
  static std::once_flag initialize_once_flag_;
  static std::vector<CComPtr<IDXGIAdapter4>> dxgi_adapters_;
  static std::vector<Device> devices_;
  static void InitializeAdaptersAndDevices();

  Device* device_{nullptr};
  Output* output_{nullptr};
  std::unique_ptr<Duplicator> duplicator_{nullptr};
  std::unique_ptr<Surface> surface_{nullptr};
  cv::Rect region_{0, 0, 1, 1};
};
}  // namespace capture

std::once_flag capture::Camera::initialize_once_flag_{};
std::vector<CComPtr<IDXGIAdapter4>> capture::Camera::dxgi_adapters_{};
std::vector<capture::Device> capture::Camera::devices_{};

void capture::Camera::InitializeAdaptersAndDevices() {
  dxgi_adapters_ = utils::EnumerateDXGIAdapters();
  for (const auto& dxgi_adapter : dxgi_adapters_) {
    Device device{dxgi_adapter};
    if (device.DXGIOutputCount() == 0)
      continue;
    devices_.emplace_back(device);
  }
}

capture::Camera::Camera(const Settings& settings)
    : region_{settings.region} {
  std::call_once(initialize_once_flag_, InitializeAdaptersAndDevices);
  if (settings.device_index >= devices_.size())
    throw std::out_of_range{"Device index out of range"};
  device_ = &devices_[settings.device_index];
  if (settings.output_index >= devices_[settings.device_index].DXGIOutputCount())
    throw std::out_of_range{"Output index is out of range."};
  output_ = &devices_[settings.device_index].DXGIOutput(settings.output_index);
  duplicator_ = std::make_unique<Duplicator>(*device_, *output_);
  surface_ = std::make_unique<Surface>(*device_, *output_);
  region_ = settings.region;
}

std::optional<cv::Mat> capture::Camera::CaptureFrame(const cv::Rect& region) const {
  if (!duplicator_->UpdateFrame())
    return std::nullopt;
  device_->D3D11DeviceImmediateContext()->CopyResource(surface_->D3D11Texture2D(), duplicator_->D3D11Texture2D());
  duplicator_->ReleaseFrame();
  const auto rect = surface_->Map();
  const auto [width, height] = output_->Resolution();
  cv::Mat frame_bgra{height, width, CV_8UC4, rect.pBits, static_cast<std::size_t>(rect.Pitch)};
  if (region != cv::Rect{0, 0, width, height}) {
    frame_bgra = frame_bgra(region);
  }
  cv::Mat frame_rgb{frame_bgra.rows, frame_bgra.cols, CV_8UC4};
  cv::cvtColor(frame_bgra, frame_rgb, cv::COLOR_BGRA2RGB);
  surface_->UnMap();
  return frame_rgb;
}
