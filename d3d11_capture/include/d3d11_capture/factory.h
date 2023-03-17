#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

#include <dxgi1_2.h>

#include "d3d11_capture/camera.h"
#include "d3d11_capture/common.h"
#include "d3d11_capture/device.h"
#include "d3d11_capture/output.h"

namespace d3d11_capture {

class Factory {
 public:
  Factory();
  ~Factory() = default;

  Camera& Create(int32_t device_index = 0,
                 std::optional<int32_t> output_index = std::nullopt,
                 std::optional<Region> region = std::nullopt);

 private:
  static std::mutex mutex_;
  static std::once_flag once_flag_;
  static std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters_;
  static std::vector<Device> devices_;

  using CameraId = std::pair<int32_t, int32_t>;  // (device_index, output_index)
  static std::map<CameraId, Camera> cameras_;
};

}  // namespace d3d11_capture
