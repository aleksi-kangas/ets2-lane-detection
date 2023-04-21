#pragma once

#include <tuple>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>

namespace ets2ld::utils {
std::tuple<CComPtr<ID3D11Device>, CComPtr<IDXGISwapChain>,
           CComPtr<ID3D11DeviceContext>>
CreateDeviceAndSwapChain(HWND hwnd);

CComPtr<ID3D11RenderTargetView> CreateRenderTargetView(
    ID3D11Device* device, IDXGISwapChain* swap_chain);
}  // namespace ets2ld::utils
