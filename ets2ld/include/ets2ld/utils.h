#pragma once

#include <tuple>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>

namespace ets2ld::utils {
std::tuple<CComPtr<ID3D11Device>, CComPtr<IDXGISwapChain>,
           CComPtr<ID3D11DeviceContext>>
CreateDeviceAndSwapChain(HWND hwnd);

CComPtr<ID3D11RenderTargetView> CreateRenderTargetView(
    ID3D11Device* device, IDXGISwapChain* swap_chain);
}  // namespace ets2ld::utils

namespace ImGui {
/**
 * Source: https://github.com/ocornut/imgui/issues/1901#issue-335266223
 */
void Spinner(const char* label, float radius, float thickness,
             const ImU32& color);
}  // namespace ImGui
