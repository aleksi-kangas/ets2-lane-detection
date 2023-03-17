#pragma once

#include <stdexcept>

#include "dx11/common.h"
#include "dx11/device.h"
#include "dx11/output.h"

namespace dx11 {

class Duplicator {
 public:
  Duplicator(Device& device, Output& output);

  bool UpdateFrame();

  void ReleaseFrame();

  [[nodiscard]] ID3D11Texture2D* D3D11Texture2D() const {
    return d3d11_texture2d_;
  }

 private:
  CComPtr<IDXGIOutputDuplication> dxgi_output_duplication_{nullptr};
  CComQIPtr<ID3D11Texture2D> d3d11_texture2d_{nullptr};
};

}  // namespace dx11
