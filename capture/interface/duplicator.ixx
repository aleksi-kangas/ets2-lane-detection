module;

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

export module capture.duplicator;

import capture.device;
import capture.output;

export namespace capture {
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
}  // namespace capture
