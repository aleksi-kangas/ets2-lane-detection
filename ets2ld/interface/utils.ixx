module;

#include <filesystem>
#include <optional>
#include <tuple>
#include <utility>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>

export module ets2ld.utils;

import ufld;

export namespace ets2ld::utils {
/**
 *
 * @return
 */
std::optional<std::filesystem::path> BrowseFolderDialog();

/**
 *
 * @param hwnd
 * @return
 */
std::tuple<CComPtr<ID3D11Device>, CComPtr<IDXGISwapChain>,
           CComPtr<ID3D11DeviceContext>>
CreateDeviceAndSwapChain(HWND hwnd);

/**
 *
 * @param device
 * @param swap_chain
 * @return
 */
CComPtr<ID3D11RenderTargetView> CreateRenderTargetView(
    ID3D11Device* device, IDXGISwapChain* swap_chain);

/**
 *
 * @return
 */
std::pair<int32_t, int32_t> QueryPrimaryMonitorResolution();
}  // namespace ets2ld::utils

export namespace ImGui {
/**
 *
 * @param label
 * @param radius
 * @param thickness
 * @param color
 */
void Spinner(const char* label, float radius, float thickness,
             const ImU32& color);
}  // namespace ImGui
