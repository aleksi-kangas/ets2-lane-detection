#pragma once

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>

#include "ets2ld/settings.h"

namespace ets2ld {
class UI {
 public:
  explicit UI(Settings& settings);
  ~UI();

  UI(const UI&) = delete;
  UI& operator=(const UI&) = delete;
  UI(UI&&) = delete;
  UI& operator=(UI&&) = delete;

  [[nodiscard]] bool BeginFrame();

  void RenderSettings(bool lane_detection_enabled, bool is_initializing);
  void RenderPreview(bool is_initializing);

  void EndFrame();

 private:
  Settings& settings_;

  WNDCLASSEXW wc_{};
  HWND hwnd_{nullptr};
  CComPtr<ID3D11Device> device_{nullptr};
  CComPtr<ID3D11DeviceContext> device_context_{nullptr};
  CComPtr<IDXGISwapChain> swap_chain_{nullptr};
  CComPtr<ID3D11RenderTargetView> render_target_view_{nullptr};

  void CreateUIWindow();

  static LRESULT CALLBACK WndProcWrapper(HWND hwnd, UINT message, WPARAM wparam,
                                         LPARAM lparam);
  LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  [[nodiscard]] bool PollEvents();
};
}  // namespace ets2ld
