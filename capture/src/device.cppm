module;

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

module capture:device;

import :output;

namespace capture {
class Device {
 public:
  explicit Device(CComPtr<IDXGIAdapter1> dxgi_adapter1);

  [[nodiscard]] ID3D11Device* D3D11Device() const;
  [[nodiscard]] ID3D11DeviceContext* D3D11DeviceContext() const;
  [[nodiscard]] ID3D11DeviceContext* D3D11DeviceImmediateContext() const;

  [[nodiscard]] uint64_t DXGIOutputCount() const;
  [[nodiscard]] Output& DXGIOutput(uint32_t index);

  [[nodiscard]] uint64_t DedicatedVideoMemory() const;
  [[nodiscard]] std::wstring Description() const;

 private:
  CComPtr<IDXGIAdapter1> dxgi_adapter1_{nullptr};
  CComPtr<ID3D11Device> d3d11_device_{nullptr};
  D3D_FEATURE_LEVEL d3d_feature_level_{};
  CComPtr<ID3D11DeviceContext> d3d11_device_context_{nullptr};
  CComPtr<ID3D11DeviceContext> d3d11_device_immediate_context_{nullptr};
  DXGI_ADAPTER_DESC1 dxgi_adapter_desc1_{};

  std::vector<Output> outputs_;

  [[nodiscard]] std::vector<CComPtr<IDXGIOutput1>> EnumerateDXGIOutputs();
};
}  // namespace capture

// -------- Implementation --------

capture::Device::Device(CComPtr<IDXGIAdapter1> dxgi_adapter1)
    : dxgi_adapter1_{std::move(dxgi_adapter1)} {
  dxgi_adapter1_->GetDesc1(&dxgi_adapter_desc1_);
  // D3D11CreateDevice requires an unknown driver type when a DXGI adapter is provided
  constexpr D3D_DRIVER_TYPE kDriverType = D3D_DRIVER_TYPE_UNKNOWN;
  constexpr std::array<D3D_FEATURE_LEVEL, 3> feature_levels = {
      {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1}};

#ifdef _DEBUG
  constexpr UINT kCreateDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
  constexpr UINT kCreateDeviceFlags = 0;
#endif

  const HRESULT hr = D3D11CreateDevice(
      dxgi_adapter1_, kDriverType, nullptr, kCreateDeviceFlags,
      feature_levels.data(), static_cast<UINT>(feature_levels.size()),
      D3D11_SDK_VERSION, &d3d11_device_, &d3d_feature_level_,
      &d3d11_device_context_);
  if (FAILED(hr)) {
    throw std::runtime_error{"TODO"};  // TODO
  }
  d3d11_device_->GetImmediateContext(&d3d11_device_immediate_context_);

  for (const auto& dxgi_output1 : EnumerateDXGIOutputs()) {
    outputs_.emplace_back(dxgi_output1);
  }
}

ID3D11Device* capture::Device::D3D11Device() const {
  return d3d11_device_;
}

ID3D11DeviceContext* capture::Device::D3D11DeviceContext() const {
  return d3d11_device_context_;
}

ID3D11DeviceContext* capture::Device::D3D11DeviceImmediateContext() const {
  return d3d11_device_immediate_context_;
}

uint64_t capture::Device::DXGIOutputCount() const {
  return outputs_.size();
}

capture::Output& capture::Device::DXGIOutput(uint32_t index) {
  return outputs_[index];
}

uint64_t capture::Device::DedicatedVideoMemory() const {
  return dxgi_adapter_desc1_.DedicatedVideoMemory;
}

std::wstring capture::Device::Description() const {
  return {dxgi_adapter_desc1_.Description,
          dxgi_adapter_desc1_.Description +
              wcslen(dxgi_adapter_desc1_.Description)};
}

std::vector<CComPtr<IDXGIOutput1>> capture::Device::EnumerateDXGIOutputs() {
  std::vector<CComPtr<IDXGIOutput1>> dxgi_outputs;
  CComPtr<IDXGIOutput> dxgi_output{nullptr};
  for (uint32_t i = 0;
       dxgi_adapter1_->EnumOutputs(i, &dxgi_output) != DXGI_ERROR_NOT_FOUND;
       ++i) {
    CComQIPtr<IDXGIOutput1> dxgi_output1{dxgi_output};
    if (!dxgi_output1) {
      throw std::runtime_error{"TODO"};  // TODO
    }
    dxgi_output.Release();
    dxgi_outputs.push_back(dxgi_output1);
  }
  return dxgi_outputs;
}
