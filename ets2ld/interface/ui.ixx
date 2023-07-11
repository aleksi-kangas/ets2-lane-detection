module;

#include <functional>
#include <string>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <opencv2/opencv.hpp>

export module ets2ld.ui;

import capture;
import ufld;

export namespace ets2ld {
class UI {
 public:
  explicit UI();
  ~UI();

  UI(const UI&) = delete;
  UI& operator=(const UI&) = delete;
  UI(UI&&) = delete;
  UI& operator=(UI&&) = delete;

  [[nodiscard]] bool BeginFrame();

  void RenderSettings(bool lane_detection_initializing,
                      bool lane_detection_active);

  void UpdatePreview(const cv::Mat& preview);
  void RenderPreview(bool lane_detection_initializing);

  void UpdateStatistics(const ufld::LaneDetectionStatistics& statistics);
  void RenderStatistics();

  void EndFrame();

  void SetOnLaneDetectionEnableChanged(
      std::function<void(bool, capture::Settings, ufld::Settings)> callback);

  void ShowErrorMessage(const std::string& message);

 private:
  capture::Settings capture_settings_{};
  ufld::Settings ufld_settings_{};
  ufld::LaneDetectionStatistics statistics_{};

  WNDCLASSEXW wc_{};
  HWND hwnd_{nullptr};
  CComPtr<ID3D11Device> device_{nullptr};
  CComPtr<ID3D11DeviceContext> device_context_{nullptr};
  CComPtr<IDXGISwapChain> swap_chain_{nullptr};
  CComPtr<ID3D11RenderTargetView> render_target_view_{nullptr};

  CComPtr<ID3D11Texture2D> preview_texture_{nullptr};
  CComPtr<ID3D11ShaderResourceView> preview_srv_{nullptr};

  // Callbacks
  std::function<void(bool, capture::Settings, ufld::Settings)>
      on_lane_detection_enable_changed_{};

  void CreateUIWindow();

  static LRESULT CALLBACK WndProcWrapper(HWND hwnd, UINT message, WPARAM wparam,
                                         LPARAM lparam);
  LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  [[nodiscard]] bool PollEvents();

  void RenderSettingsGeneral(bool lane_detection_initializing);
  void RenderSettingsModel(bool lane_detection_initializing,
                           bool lane_detection_active);
  void RenderSettingsCapture(bool lane_detection_initializing,
                             bool lane_detection_active);
};
}  // namespace ets2ld
