#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <vector>

#include <opencv2/opencv.hpp>

#include "dx11/camera.h"
#include "dx11/common.h"
#include "dx11/device.h"

namespace dx11 {

class Capture {
 public:
  Capture();
  ~Capture();

  Camera* Start(uint32_t device_index, uint32_t output_index,
                uint32_t frame_buffer_capacity = 16,
                std::optional<cv::Rect> region = std::nullopt);
  void Stop();

 private:
  static std::mutex mutex_;
  static std::once_flag once_flag_;
  static std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters_;
  static std::vector<Device> devices_;
  static std::unique_ptr<Camera> camera_;
};

}  // namespace dx11
