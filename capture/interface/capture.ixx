module;

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include <atlbase.h>
#include <dxgi1_2.h>

#include <opencv2/opencv.hpp>

export module capture;

export import capture.camera;
import capture.device;
export import capture.utils;

export namespace capture {
struct Settings {
  std::uint32_t device_index{0};
  std::uint32_t output_index{0};
  std::uint32_t frame_buffer_capacity{16};
  cv::Rect region{0, 0, utils::QueryPrimaryMonitorResolution().first,
                  utils::QueryPrimaryMonitorResolution().second};
};

class CaptureManager {
 public:
  CaptureManager();
  ~CaptureManager();

  Camera* Start(const Settings& settings);
  void Stop();

 private:
  static std::mutex mutex_;
  static std::once_flag once_flag_;
  static std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters_;
  static std::vector<Device> devices_;
  static std::unique_ptr<Camera> camera_;
};
}  // namespace capture
