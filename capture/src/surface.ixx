module;

#include <cstdint>
#include <stdexcept>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>

export module capture:surface;

import :device;
import :output;

namespace capture {
class Surface {
 public:
  Surface(Device& device, Output& output);

  Surface(const Surface&) = delete;
  Surface& operator=(const Surface&) = delete;

  Surface(Surface&&) = delete;
  Surface& operator=(Surface&&) = delete;

  [[nodiscard]] DXGI_MAPPED_RECT Map() const;
  void UnMap() const;

  [[nodiscard]] ID3D11Texture2D* D3D11Texture2D() const;

 private:
  Device& device_;
  Output& output_;
  std::int32_t width_{0};
  std::int32_t height_{0};
  DXGI_FORMAT dxgi_format_{DXGI_FORMAT_B8G8R8A8_UNORM};
  D3D11_TEXTURE2D_DESC d3d11_texture2d_desc_{};
  CComPtr<ID3D11Texture2D> d3d11_texture2d_{nullptr};

  void Rebuild();
};
}  // namespace capture

capture::Surface::Surface(Device& device, Output& output) : device_{device}, output_{output} {
  Rebuild();
}

DXGI_MAPPED_RECT capture::Surface::Map() const {
  DXGI_MAPPED_RECT dxgi_mapped_rect{};
  const CComQIPtr<IDXGISurface> dxgi_surface{d3d11_texture2d_};
  if (FAILED(dxgi_surface->Map(&dxgi_mapped_rect, DXGI_MAP_READ)))
    throw std::runtime_error{"Map: Failure"};
  return dxgi_mapped_rect;
}

void capture::Surface::UnMap() const {
  const CComQIPtr<IDXGISurface> dxgi_surface{d3d11_texture2d_};
  if (FAILED(dxgi_surface->Unmap()))
    throw std::runtime_error{"Unmap: Failure"};
}

ID3D11Texture2D* capture::Surface::D3D11Texture2D() const {
  return d3d11_texture2d_;
}

void capture::Surface::Rebuild() {
  const auto [width, height] = output_.Resolution();
  width_ = width;
  height_ = height;
  if (d3d11_texture2d_ == nullptr) {
    d3d11_texture2d_desc_.Width = static_cast<std::uint32_t>(width_);
    d3d11_texture2d_desc_.Height = static_cast<std::uint32_t>(height_);
    d3d11_texture2d_desc_.MipLevels = 1;
    d3d11_texture2d_desc_.ArraySize = 1;
    d3d11_texture2d_desc_.Format = dxgi_format_;
    d3d11_texture2d_desc_.SampleDesc.Count = 1;
    d3d11_texture2d_desc_.SampleDesc.Quality = 0;
    d3d11_texture2d_desc_.Usage = D3D11_USAGE_STAGING;
    d3d11_texture2d_desc_.BindFlags = 0;
    d3d11_texture2d_desc_.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    d3d11_texture2d_desc_.MiscFlags = 0;
    if (FAILED(device_.D3D11Device()->CreateTexture2D(&d3d11_texture2d_desc_, nullptr, &d3d11_texture2d_)))
      throw std::runtime_error{"CreateTexture2D: Failure"};
  }
}
