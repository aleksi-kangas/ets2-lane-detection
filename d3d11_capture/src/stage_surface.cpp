#include "d3d11_capture/stage_surface.h"

namespace d3d11_capture {

StageSurface::StageSurface(Device& device, Output& output) : output_{output}, device_{device} {
  Rebuild();
}

DXGI_MAPPED_RECT StageSurface::Map() {
  DXGI_MAPPED_RECT dxgi_mapped_rect{};
  CComQIPtr<IDXGISurface> dxgi_surface{d3d11_texture2d_};
  dxgi_surface->Map(&dxgi_mapped_rect, DXGI_MAP_READ);
  return dxgi_mapped_rect;
}

void StageSurface::UnMap() {
  CComQIPtr<IDXGISurface> dxgi_surface{d3d11_texture2d_};
  dxgi_surface->Unmap();
}

void StageSurface::Rebuild() {
  const auto [width, height] = output_.Resolution();
  width_ = width;
  height_ = height;
  if (d3d11_texture2d_ == nullptr) {
    d3d11_texture2d_desc_.Width = width_;
    d3d11_texture2d_desc_.Height = height_;
    d3d11_texture2d_desc_.MipLevels = 1;
    d3d11_texture2d_desc_.ArraySize = 1;
    d3d11_texture2d_desc_.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    d3d11_texture2d_desc_.SampleDesc.Count = 1;
    d3d11_texture2d_desc_.SampleDesc.Quality = 0;
    d3d11_texture2d_desc_.Usage = D3D11_USAGE_STAGING;
    d3d11_texture2d_desc_.BindFlags = 0;
    d3d11_texture2d_desc_.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    d3d11_texture2d_desc_.MiscFlags = 0;
    device_.D3D11Device()->CreateTexture2D(&d3d11_texture2d_desc_, nullptr, &d3d11_texture2d_);
  }
}

}  // namespace d3d11_capture
