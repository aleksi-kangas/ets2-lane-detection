#include "d3d11_capture/factory.h"

namespace d3d11_capture {

Factory::Factory() : dxgi_adapters_{EnumerateDXGIAdapters()} {
  for (const auto& dxgi_adapter : dxgi_adapters_) {
    Device device{dxgi_adapter};
    if (device.DXGIOutputCount() == 0)
      continue;
    devices_.emplace_back(device);
  }
}

Camera& Factory::Create(int32_t device_index, std::optional<int32_t> output_index, std::optional<Region> region) {
  Device& device = devices_[device_index];
  if (!output_index.has_value()) {
    output_index = 0;  // For now use the first output
  }
  Output& output = device.DXGIOutput(output_index.value());

  auto camera_id = std::make_pair(device_index, output_index.value());
  const auto existing_camera = cameras_.find(camera_id);
  if (existing_camera != cameras_.end()) {
    return existing_camera->second;
  }

  cameras_.emplace(std::piecewise_construct, std::forward_as_tuple(camera_id),
                   std::forward_as_tuple(device, output, region));
  return cameras_.at(camera_id);
}

}  // namespace d3d11_capture
