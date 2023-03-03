#pragma once

#include <cstdint>

#include <d3d11.h>

#include "d3d11_capture/device.h"
#include "d3d11_capture/output.h"

#include "d3d11_capture/common.h"

namespace d3d11_capture {

class StageSurface {
 public:
  StageSurface(Device& device, Output& output);

  DXGI_MAPPED_RECT Map();
  void UnMap();

  [[nodiscard]] ID3D11Texture2D* D3D11Texture2D() const { return d3d11_texture2d_; }

 private:
  Device& device_;
  Output& output_;
  uint32_t width_{0};
  uint32_t height_{0};
  uint32_t dxgi_format{DXGI_FORMAT_B8G8R8A8_UNORM};
  D3D11_TEXTURE2D_DESC d3d11_texture2d_desc_{};
  CComPtr<ID3D11Texture2D> d3d11_texture2d_{nullptr};

  void Rebuild();
};

}  // namespace d3d11_capture
