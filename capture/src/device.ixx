module;

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_6.h>

export module capture:device;

import :output;

namespace capture {
class Device {
 public:
  explicit Device(CComPtr<IDXGIAdapter4> dxgi_adapter4);

  Device(const Device&) = default;
  Device& operator=(const Device&) = default;

  Device(Device&&) = default;
  Device& operator=(Device&&) = default;

  [[nodiscard]] ID3D11Device* D3D11Device() const;
  [[nodiscard]] ID3D11DeviceContext* D3D11DeviceContext() const;
  [[nodiscard]] ID3D11DeviceContext* D3D11DeviceImmediateContext() const;

  [[nodiscard]] std::uint64_t DXGIOutputCount() const;
  [[nodiscard]] Output& DXGIOutput(std::uint32_t index);

  [[nodiscard]] std::uint64_t DedicatedVideoMemory() const;
  [[nodiscard]] std::wstring Description() const;

 private:
  CComPtr<IDXGIAdapter4> dxgi_adapter4_{nullptr};
  CComPtr<ID3D11Device> d3d11_device_{nullptr};
  D3D_FEATURE_LEVEL d3d_feature_level_{};
  CComPtr<ID3D11DeviceContext> d3d11_device_context_{nullptr};
  CComPtr<ID3D11DeviceContext> d3d11_device_immediate_context_{nullptr};
  DXGI_ADAPTER_DESC3 dxgi_adapter_desc3_{};

  std::vector<Output> outputs_{};

  [[nodiscard]] std::vector<CComPtr<IDXGIOutput6>> EnumerateDXGIOutputs() const;
};
}  // namespace capture

capture::Device::Device(CComPtr<IDXGIAdapter4> dxgi_adapter4) : dxgi_adapter4_{std::move(dxgi_adapter4)} {
  if (FAILED(dxgi_adapter4_->GetDesc3(&dxgi_adapter_desc3_)))
    throw std::runtime_error{"GetDesc3: Failure"};
  // D3D11CreateDevice requires an unknown driver type when a DXGI adapter is provided
  constexpr D3D_DRIVER_TYPE kDriverType{D3D_DRIVER_TYPE_UNKNOWN};
  constexpr std::array<D3D_FEATURE_LEVEL, 3> feature_levels{
      {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1}};

#ifdef _DEBUG
  constexpr UINT kCreateDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
  constexpr UINT kCreateDeviceFlags = 0;
#endif

  const HRESULT hr = D3D11CreateDevice(dxgi_adapter4_, kDriverType, nullptr, kCreateDeviceFlags, feature_levels.data(),
                                       static_cast<UINT>(feature_levels.size()), D3D11_SDK_VERSION, &d3d11_device_,
                                       &d3d_feature_level_, &d3d11_device_context_);
  if (FAILED(hr)) {
    throw std::runtime_error{"D3D11CreateDevice: Failure"};
  }
  d3d11_device_->GetImmediateContext(&d3d11_device_immediate_context_);

  for (const auto& dxgi_output6 : EnumerateDXGIOutputs()) {
    outputs_.emplace_back(dxgi_output6);
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

capture::Output& capture::Device::DXGIOutput(std::uint32_t index) {
  return outputs_[index];
}

uint64_t capture::Device::DedicatedVideoMemory() const {
  return dxgi_adapter_desc3_.DedicatedVideoMemory;
}

std::wstring capture::Device::Description() const {
  return {dxgi_adapter_desc3_.Description, dxgi_adapter_desc3_.Description + wcslen(dxgi_adapter_desc3_.Description)};
}

std::vector<CComPtr<IDXGIOutput6>> capture::Device::EnumerateDXGIOutputs() const {
  std::vector<CComPtr<IDXGIOutput6>> dxgi_outputs{};
  CComPtr<IDXGIOutput> dxgi_output{nullptr};
  for (std::uint32_t i = 0; dxgi_adapter4_->EnumOutputs(i, &dxgi_output) != DXGI_ERROR_NOT_FOUND; ++i) {
    CComQIPtr<IDXGIOutput6> dxgi_output6{dxgi_output};
    if (!dxgi_output6)
      throw std::runtime_error{"No DXGIOutput6"};
    dxgi_output.Release();
    dxgi_outputs.emplace_back(dxgi_output6);
  }
  return dxgi_outputs;
}
