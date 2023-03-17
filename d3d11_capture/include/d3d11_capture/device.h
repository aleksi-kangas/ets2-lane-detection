#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "d3d11_capture/common.h"
#include "d3d11_capture/output.h"

namespace d3d11_capture {

class Device {
 public:
  explicit Device(CComPtr<IDXGIAdapter1> dxgi_adapter1);

  [[nodiscard]] ID3D11Device* D3D11Device() const;
  [[nodiscard]] ID3D11DeviceContext* D3D11DeviceContext() const;
  [[nodiscard]] ID3D11DeviceContext* D3D11DeviceImmediateContext() const;

  [[nodiscard]] int32_t DXGIOutputCount() const;
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

}  // namespace d3d11_capture
