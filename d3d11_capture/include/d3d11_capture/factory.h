#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include <dxgi1_2.h>

#include "d3d11_capture/camera.h"
#include "d3d11_capture/common.h"
#include "d3d11_capture/device.h"
#include "d3d11_capture/output.h"

namespace d3d11_capture {

/**
 * A singleton class for creating cameras.
 */
class Factory {
 public:
  ~Factory() = default;

  Factory(const Factory&) = delete;
  Factory& operator=(const Factory&) = delete;

  static Factory& Instance() {
    static Factory instance;
    return instance;
  }

  Camera& Create(int32_t device_index = 0, std::optional<int32_t> output_index = std::nullopt,
                 std::optional<Region> region = std::nullopt);

 private:
  Factory();

  std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters_{};
  std::vector<Device> devices_{};

  using CameraId = std::pair<int32_t, int32_t>;  // (device_index, output_index)
  std::map<CameraId, Camera> cameras_{};
};

}  // namespace d3d11_capture
