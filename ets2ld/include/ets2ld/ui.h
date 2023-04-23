#pragma once

#include <functional>

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

  void RenderSettings(bool lane_detection_active, bool lane_detection_initializing);
  void RenderPreview(bool lane_detection_initializing);

  void EndFrame();

  void SetOnLaneDetectionEnableChanged(std::function<void()> callback);
  void SetOnModelSettingsChanged(std::function<void()> callback);

 private:
  Settings& settings_;

  WNDCLASSEXW wc_{};
  HWND hwnd_{nullptr};
  CComPtr<ID3D11Device> device_{nullptr};
  CComPtr<ID3D11DeviceContext> device_context_{nullptr};
  CComPtr<IDXGISwapChain> swap_chain_{nullptr};
  CComPtr<ID3D11RenderTargetView> render_target_view_{nullptr};

  // Callbacks
  std::function<void()> on_lane_detection_enable_changed_{};
  std::function<void()> on_model_settings_changed_{};

  void CreateUIWindow();

  static LRESULT CALLBACK WndProcWrapper(HWND hwnd, UINT message, WPARAM wparam,
                                         LPARAM lparam);
  LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  [[nodiscard]] bool PollEvents();
};
}  // namespace ets2ld
