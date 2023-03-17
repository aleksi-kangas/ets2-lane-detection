#pragma once

#include <cstdint>

#include "dx11/common.h"
#include "dx11/device.h"
#include "dx11/output.h"

namespace dx11 {

class Surface {
 public:
  Surface(Device& device, Output& output);

  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  Surface(Surface&&) = delete;
  Surface& operator=(Surface&&) = delete;

  DXGI_MAPPED_RECT Map();
  void UnMap();

  [[nodiscard]] ID3D11Texture2D* D3D11Texture2D() const;

 private:
  Device& device_;
  Output& output_;
  int32_t width_{0};
  int32_t height_{0};
  DXGI_FORMAT dxgi_format_{DXGI_FORMAT_B8G8R8A8_UNORM};
  D3D11_TEXTURE2D_DESC d3d11_texture2d_desc_{};
  CComPtr<ID3D11Texture2D> d3d11_texture2d_{nullptr};

  void Rebuild();
};

}  // namespace dx11
