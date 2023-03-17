#include "d3d11_capture/duplicator.h"

namespace d3d11_capture {

Duplicator::Duplicator(Device& device, Output& output) {
  output.DXGIOutput1()->DuplicateOutput(device.D3D11Device(),
                                        &dxgi_output_duplication_);
}

bool Duplicator::UpdateFrame() {
  DXGI_OUTDUPL_FRAME_INFO frame_info;
  CComPtr<IDXGIResource> dxgi_resource{nullptr};
  HRESULT hr = dxgi_output_duplication_->AcquireNextFrame(0, &frame_info,
                                                          &dxgi_resource);
  if (hr == DXGI_ERROR_WAIT_TIMEOUT || hr == DXGI_ERROR_ACCESS_LOST) {
    return false;
  }
  if (FAILED(hr)) {
    throw std::runtime_error{"Failed to acquire next frame"};
  }
  CComQIPtr<ID3D11Texture2D> d3d11_texture{dxgi_resource};
  if (!d3d11_texture) {
    throw std::runtime_error{"Failed to get texture"};
  }
  d3d11_texture2d_ = d3d11_texture;
  return true;
}

void Duplicator::ReleaseFrame() {
  dxgi_output_duplication_->ReleaseFrame();
}

}  // namespace d3d11_capture
