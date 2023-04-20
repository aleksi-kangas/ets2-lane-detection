#include "dx11/capture.h"

#include <cassert>

namespace dx11 {

std::mutex Capture::mutex_{};
std::once_flag Capture::once_flag_{};
std::vector<CComPtr<IDXGIAdapter1>> Capture::dxgi_adapters_{};
std::vector<Device> Capture::devices_{};
std::unique_ptr<Camera> Capture::camera_{nullptr};

Capture::Capture() {
  auto Initialize = []() {
    dxgi_adapters_ = EnumerateDXGIAdapters();
    for (const auto& dxgi_adapter : dxgi_adapters_) {
      Device device{dxgi_adapter};
      if (device.DXGIOutputCount() == 0)
        continue;
      devices_.emplace_back(device);
    }
  };
  std::call_once(once_flag_, Initialize);
}

Capture::~Capture() {
  std::lock_guard<std::mutex> lock{mutex_};
  if (camera_ != nullptr) {
    camera_->StopCapture();
    camera_ = nullptr;
  }
}

Camera* Capture::Start(uint32_t device_index, uint32_t output_index,
                       std::optional<cv::Rect> region) {
  std::lock_guard<std::mutex> lock{mutex_};

  auto Initialize = []() {
    dxgi_adapters_ = EnumerateDXGIAdapters();
    for (const auto& dxgi_adapter : dxgi_adapters_) {
      Device device{dxgi_adapter};
      if (device.DXGIOutputCount() == 0)
        continue;
      devices_.emplace_back(device);
    }
  };
  std::call_once(once_flag_, Initialize);

  if (camera_ != nullptr) {
    assert(false);
    return nullptr;
  }
  if (device_index >= devices_.size()) {
    assert(false);
    return nullptr;
  }
  if (output_index >= devices_[device_index].DXGIOutputCount()) {
    assert(false);
    return nullptr;
  }
  camera_ = std::make_unique<Camera>(
      devices_[device_index], devices_[device_index].DXGIOutput(output_index),
      region);
  camera_->StartCapture();
  return camera_.get();
}

void Capture::Stop() {
  std::lock_guard<std::mutex> lock{mutex_};
  if (camera_ == nullptr) {
    assert(false);
    return;
  }
  camera_->StopCapture();
  camera_ = nullptr;
}

}  // namespace dx11
