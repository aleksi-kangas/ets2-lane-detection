#include "d3d11_capture/device.h"

#include <array>
#include <stdexcept>
#include <utility>

namespace d3d11_capture {

Device::Device(CComPtr<IDXGIAdapter1> dxgi_adapter1) : dxgi_adapter1_{std::move(dxgi_adapter1)} {
  dxgi_adapter1_->GetDesc1(&dxgi_adapter_desc1_);
  // D3D11CreateDevice requires an unknown driver type when a DXGI adapter is provided
  constexpr D3D_DRIVER_TYPE kDriverType = D3D_DRIVER_TYPE_UNKNOWN;
  constexpr std::array<D3D_FEATURE_LEVEL, 3> feature_levels = {
      {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1}};
  const HRESULT hr = D3D11CreateDevice(dxgi_adapter1_, kDriverType, nullptr, 0, feature_levels.data(),
                                       static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &d3d11_device_,
                                       &d3d_feature_level_, &d3d11_device_context_);
  if (FAILED(hr)) {
    throw std::runtime_error{"TODO"};  // TODO
  }
  d3d11_device_->GetImmediateContext(&d3d11_device_immediate_context_);
}

std::vector<CComPtr<IDXGIOutput1>> Device::EnumerateDXGIOutputs() {
  std::vector<CComPtr<IDXGIOutput1>> dxgi_outputs;
  CComPtr<IDXGIOutput> dxgi_output{nullptr};
  for (uint32_t i = 0; dxgi_adapter1_->EnumOutputs(i, &dxgi_output) != DXGI_ERROR_NOT_FOUND; ++i) {
    CComQIPtr<IDXGIOutput1> dxgi_output1{dxgi_output};
    if (!dxgi_output1) {
      throw std::runtime_error{"TODO"};  // TODO
    }
    dxgi_output.Release();
    dxgi_outputs.push_back(dxgi_output1);
  }
  return dxgi_outputs;
}

ID3D11Device* Device::D3D11Device() const {
  return d3d11_device_;
}

ID3D11DeviceContext* Device::D3D11DeviceContext() const {
  return d3d11_device_context_;
}

ID3D11DeviceContext* Device::D3D11DeviceImmediateContext() const {
  return d3d11_device_immediate_context_;
}

uint64_t Device::DedicatedVideoMemory() const {
  return dxgi_adapter_desc1_.DedicatedVideoMemory;
}

std::wstring Device::Description() const {
  return {dxgi_adapter_desc1_.Description, dxgi_adapter_desc1_.Description + wcslen(dxgi_adapter_desc1_.Description)};
}

}  // namespace d3d11_capture
