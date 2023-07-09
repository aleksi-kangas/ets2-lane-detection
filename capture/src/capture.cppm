module;

#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include <atlbase.h>
#include <dxgi1_2.h>

#include <opencv2/opencv.hpp>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

module capture;

import capture.camera;
import capture.device;
import capture.utils;

std::mutex capture::CaptureManager::mutex_{};
std::once_flag capture::CaptureManager::once_flag_{};
std::vector<CComPtr<IDXGIAdapter1>> capture::CaptureManager::dxgi_adapters_{};
std::vector<capture::Device> capture::CaptureManager::devices_{};
std::unique_ptr<capture::Camera> capture::CaptureManager::camera_{nullptr};

capture::CaptureManager::CaptureManager() {
  auto Initialize = []() {
    dxgi_adapters_ = capture::utils::EnumerateDXGIAdapters();
    for (const auto& dxgi_adapter : dxgi_adapters_) {
      Device device{dxgi_adapter};
      if (device.DXGIOutputCount() == 0)
        continue;
      devices_.emplace_back(device);
    }
  };
  std::call_once(once_flag_, Initialize);
}

capture::CaptureManager::~CaptureManager() {
  std::lock_guard<std::mutex> lock{mutex_};
  if (camera_ != nullptr) {
    camera_->StopCapture();
    camera_ = nullptr;
  }
}

capture::Camera* capture::CaptureManager::Start(
    const capture::Settings& settings) {
  std::lock_guard<std::mutex> lock{mutex_};

  auto Initialize = []() {
    dxgi_adapters_ = capture::utils::EnumerateDXGIAdapters();
    for (const auto& dxgi_adapter : dxgi_adapters_) {
      Device device{dxgi_adapter};
      if (device.DXGIOutputCount() == 0)
        continue;
      devices_.emplace_back(device);
    }
  };
  std::call_once(once_flag_, Initialize);

  if (camera_ != nullptr)
    throw std::logic_error{"Camera is already started."};
  if (settings.device_index >= devices_.size())
    throw std::out_of_range{"Device index is out of range."};
  if (settings.output_index >=
      devices_[settings.device_index].DXGIOutputCount())
    throw std::out_of_range{"Output index is out of range."};

  camera_ = std::make_unique<capture::Camera>(
      devices_[settings.device_index],
      devices_[settings.device_index].DXGIOutput(settings.output_index),
      settings.frame_buffer_capacity, settings.region);
  camera_->StartCapture();
  return camera_.get();
}

void capture::CaptureManager::Stop() {
  std::lock_guard<std::mutex> lock{mutex_};
  if (camera_ == nullptr)
    throw std::logic_error{"Camera is not started."};
  camera_->StopCapture();
  camera_ = nullptr;
}
