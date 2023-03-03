#pragma once

#include <stdexcept>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "d3d11_capture/common.h"
#include "d3d11_capture/device.h"
#include "d3d11_capture/output.h"

namespace d3d11_capture {

class Duplicator {
 public:
  Duplicator(Device& device, Output& output);

  bool UpdateFrame();

  void ReleaseFrame();

  [[nodiscard]] ID3D11Texture2D* D3D11Texture2D() const { return d3d11_texture2d_; }

 private:
  CComPtr<IDXGIOutputDuplication> dxgi_output_duplication_{nullptr};
  CComQIPtr<ID3D11Texture2D> d3d11_texture2d_{nullptr};
};

}  // namespace d3d11_capture
