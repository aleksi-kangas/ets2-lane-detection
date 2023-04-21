#include "ets2ld/utils.h"

#include <array>
#include <stdexcept>

namespace ets2ld::utils {
std::tuple<CComPtr<ID3D11Device>, CComPtr<IDXGISwapChain>,
           CComPtr<ID3D11DeviceContext>>
CreateDeviceAndSwapChain(HWND hwnd) {
  CComPtr<ID3D11Device> device;
  CComPtr<IDXGISwapChain> swap_chain;
  CComPtr<ID3D11DeviceContext> device_context;

  DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
  swap_chain_desc.BufferCount = 2;
  swap_chain_desc.BufferDesc.Width = 0;
  swap_chain_desc.BufferDesc.Height = 0;
  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
  swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  swap_chain_desc.OutputWindow = hwnd;
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.SampleDesc.Quality = 0;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  swap_chain_desc.Windowed = TRUE;

  constexpr std::array<D3D_FEATURE_LEVEL, 2> kFeatureLevels = {
      D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
  HRESULT result = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, kFeatureLevels.data(),
      static_cast<UINT>(kFeatureLevels.size()), D3D11_SDK_VERSION,
      &swap_chain_desc, &swap_chain, &device, nullptr, &device_context);
  if (result == DXGI_ERROR_UNSUPPORTED) {
    result = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, kFeatureLevels.data(),
        static_cast<UINT>(kFeatureLevels.size()), D3D11_SDK_VERSION,
        &swap_chain_desc, &swap_chain, &device, nullptr, &device_context);
  }
  if (FAILED(result)) {
    throw std::runtime_error{"Failed to create device and swap chain"};
  }
  return {device, swap_chain, device_context};
}

CComPtr<ID3D11RenderTargetView> CreateRenderTargetView(
    ID3D11Device* device, IDXGISwapChain* swap_chain) {
  CComPtr<ID3D11Texture2D> back_buffer;
  HRESULT result = swap_chain->GetBuffer(
      0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
  if (FAILED(result)) {
    throw std::runtime_error{"Failed to get back buffer"};
  }
  CComPtr<ID3D11RenderTargetView> render_target_view;
  device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
  if (FAILED(result)) {
    throw std::runtime_error{"Failed to create render target view"};
  }
  return render_target_view;
}

}  // namespace ets2ld::utils
