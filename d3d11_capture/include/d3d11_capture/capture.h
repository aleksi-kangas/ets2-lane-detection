#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

#include <atlbase.h>
#include <dxgi1_2.h>

#include "d3d11_capture/camera.h"
#include "d3d11_capture/common.h"
#include "d3d11_capture/device.h"
#include "d3d11_capture/region.h"

namespace d3d11_capture {

class Capture {
 public:
  Capture();
  ~Capture();

  Camera* Start(uint32_t device_index, uint32_t output_index,
                std::optional<Region> region = std::nullopt);
  void Stop();

 private:
  static std::mutex mutex_;
  static std::once_flag once_flag_;
  static std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters_;
  static std::vector<Device> devices_;
  static std::unique_ptr<Camera> camera_;
};

}  // namespace d3d11_capture
