module;

#include <stdexcept>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>

export module capture:duplicator;

import :device;
import :output;

namespace capture {
class Duplicator {
 public:
  Duplicator(const Device& device, const Output& output);

  Duplicator(const Duplicator&) = delete;
  Duplicator& operator=(const Duplicator&) = delete;

  Duplicator(Duplicator&&) = delete;
  Duplicator& operator=(Duplicator&&) = delete;

  bool UpdateFrame();

  void ReleaseFrame();

  [[nodiscard]] ID3D11Texture2D* D3D11Texture2D() const { return d3d11_texture2d_; }

 private:
  CComPtr<IDXGIOutputDuplication> dxgi_output_duplication_{nullptr};
  CComQIPtr<ID3D11Texture2D> d3d11_texture2d_{nullptr};
};
}  // namespace capture

capture::Duplicator::Duplicator(const Device& device, const Output& output) {
  const HRESULT hr = output.DXGIOutput6()->DuplicateOutput(device.D3D11Device(), &dxgi_output_duplication_);
  if (hr == E_INVALIDARG)
    throw std::logic_error{"DuplicateOutput1: Invalid argument"};
  if (hr == E_ACCESSDENIED)
    throw std::runtime_error{"DuplicateOutput1: Access denied"};
  if (hr == DXGI_ERROR_UNSUPPORTED)
    throw std::runtime_error{"DuplicateOutput1: Unsupported"};
  if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
    throw std::runtime_error{"DuplicateOutput1: Not currently available"};
  if (hr == DXGI_ERROR_SESSION_DISCONNECTED)
    throw std::runtime_error{"DuplicateOutput1: Session disconnected"};
  if (FAILED(hr))
    throw std::runtime_error{"DuplicateOutput1: Unknown failure"};
}

bool capture::Duplicator::UpdateFrame() {
  DXGI_OUTDUPL_FRAME_INFO frame_info;
  CComPtr<IDXGIResource> dxgi_resource{nullptr};
  const HRESULT hr = dxgi_output_duplication_->AcquireNextFrame(0, &frame_info, &dxgi_resource);
  if (hr == DXGI_ERROR_WAIT_TIMEOUT || hr == DXGI_ERROR_ACCESS_LOST)
    return false;
  if (FAILED(hr))
    throw std::runtime_error{"Failed to acquire next frame"};
  const CComQIPtr<ID3D11Texture2D> d3d11_texture{dxgi_resource};
  if (!d3d11_texture)
    throw std::runtime_error{"Failed to get texture"};
  d3d11_texture2d_ = d3d11_texture;
  return true;
}

void capture::Duplicator::ReleaseFrame() {
  const HRESULT hr = dxgi_output_duplication_->ReleaseFrame();
  if (hr == DXGI_ERROR_INVALID_CALL)
    throw std::logic_error{"ReleaseFrame: Invalid call"};
  if (hr == DXGI_ERROR_ACCESS_LOST)
    throw std::runtime_error{"ReleaseFrame: Access lost"};
  if (FAILED(hr))
    throw std::runtime_error{"ReleaseFrame: Unknown failure"};
}
