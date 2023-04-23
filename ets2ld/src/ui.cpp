#include "ets2ld/ui.h"

#include <array>
#include <cassert>
#include <stdexcept>
#include <utility>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "ets2ld/utils.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace ets2ld {

UI::UI(Settings& settings) : settings_{settings} {
  CreateUIWindow();

  auto [device, swap_chain, device_context] =
      utils::CreateDeviceAndSwapChain(hwnd_);
  device_ = device;
  swap_chain_ = swap_chain;
  device_context_ = device_context;
  render_target_view_ = utils::CreateRenderTargetView(device_, swap_chain_);

  ::ShowWindow(hwnd_, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd_);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_Init(hwnd_);
  ImGui_ImplDX11_Init(device_, device_context_);
}

UI::~UI() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  ::DestroyWindow(hwnd_);
  ::UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
}

bool UI::BeginFrame() {
  if (!PollEvents())
    return false;

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  return true;
}

void UI::RenderSettings(bool lane_detection_active,
                        bool lane_detection_initializing) {
  ImGui::Begin("Settings");
  {
    ImGui::SeparatorText("General");
    ImGui::BeginDisabled(lane_detection_initializing);
    {
      if (ImGui::Checkbox("Enable", &settings_.enable_lane_detection)) {
        if (on_lane_detection_enable_changed_)
          on_lane_detection_enable_changed_();
      }
    }
    ImGui::EndDisabled();

    ImGui::SeparatorText("Model");
    ImGui::BeginDisabled(lane_detection_active || lane_detection_initializing);
    {
      ImGui::Text("Ultra-Fast-Lane-Detection");
      const char* kModelVariantComboItems[] = {"CULane", "TuSimple"};
      static int chosen_model_variant = 0;
      ImGui::Combo("Variant", &chosen_model_variant, kModelVariantComboItems,
                   IM_ARRAYSIZE(kModelVariantComboItems));
      if (ImGui::Button("Apply")) {
        // TODO Abstract this?
        switch (chosen_model_variant) {
          case 0:
            assert(0 == static_cast<int32_t>(ufld::v1::ModelType::kCULane));
            settings_.model.variant = ufld::v1::ModelType::kCULane;
            break;
          case 1:
            assert(1 == static_cast<int32_t>(ufld::v1::ModelType::kTuSimple));
            settings_.model.variant = ufld::v1::ModelType::kTuSimple;
            break;
          default:
            assert(false);
        }
        if (on_model_settings_changed_)
          on_model_settings_changed_();
      }
      if (lane_detection_initializing) {
        ImGui::SameLine();
        ImGui::Spinner("InitializingSpinner", 10, 2,
                       ImGui::GetColorU32(ImGuiCol_Text));
      }
    }
    ImGui::EndDisabled();

    ImGui::SeparatorText("Capture");
    ImGui::Text("TODO: Capture Settings");
  }
  ImGui::End();
}

void UI::RenderPreview(bool lane_detection_initializing) {
  ImGui::Begin("Preview");
  {
    if (lane_detection_initializing) {
      ImGui::Spinner("InitializingSpinner", 10, 2,
                     ImGui::GetColorU32(ImGuiCol_Text));
      ImGui::SameLine();
      ImGui::Text("Initializing OnnxRuntime and loading model...");
    } else {
      ImGui::Text("TODO PREVIEW");
    }
  }
  ImGui::End();
}

void UI::EndFrame() {
  ImGui::Render();
  device_context_->OMSetRenderTargets(1, &render_target_view_.p, nullptr);
  constexpr auto kClearColor = std::array<float, 4>{0.45f, 0.55f, 0.60f, 1.00f};
  device_context_->ClearRenderTargetView(render_target_view_,
                                         kClearColor.data());
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  swap_chain_->Present(1, 0);  // VSYNC
}

void UI::SetOnLaneDetectionEnableChanged(std::function<void()> callback) {
  on_lane_detection_enable_changed_ = std::move(callback);
}

void UI::SetOnModelSettingsChanged(std::function<void()> callback) {
  on_model_settings_changed_ = std::move(callback);
}

void UI::CreateUIWindow() {
  wc_.cbSize = sizeof(wc_);
  wc_.style = CS_CLASSDC;
  wc_.lpfnWndProc = WndProcWrapper;
  wc_.hInstance = ::GetModuleHandle(nullptr);
  wc_.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
  wc_.lpszClassName = L"ETS2 Lane Detection";
  RegisterClassExW(&wc_);

  hwnd_ = CreateWindowExW(0, wc_.lpszClassName, wc_.lpszClassName,
                          WS_OVERLAPPEDWINDOW, 300, 300, 1280, 720, nullptr,
                          nullptr, wc_.hInstance, this);
  if (!hwnd_) {
    throw std::runtime_error{"Failed to create window"};
  }
}

LRESULT UI::WndProcWrapper(HWND hwnd, UINT message, WPARAM wparam,
                           LPARAM lparam) {
  if (message == WM_NCCREATE) {
    auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA,
                       reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
  }
  auto ui = reinterpret_cast<UI*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
  if (ui) {
    return ui->WndProc(hwnd, message, wparam, lparam);
  }
  return ::DefWindowProc(hwnd, message, wparam, lparam);
}

LRESULT UI::WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
    return true;

  switch (message) {
    case WM_SIZE: {
      if (device_ && wparam != SIZE_MINIMIZED) {
        render_target_view_.Release();
        swap_chain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
        render_target_view_ =
            utils::CreateRenderTargetView(device_, swap_chain_);
      }
      return 0;
    }
    case WM_DESTROY: {
      ::PostQuitMessage(0);
      return 0;
    }
    case WM_SYSCOMMAND: {
      if ((wparam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
        return 0;
    } break;
    default:
      return ::DefWindowProc(hwnd, message, wparam, lparam);
  }
  return ::DefWindowProc(hwnd, message, wparam, lparam);
}

bool UI::PollEvents() {
  MSG msg;
  while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
    if (msg.message == WM_QUIT) {
      return false;
    }
  }
  return true;
}

}  // namespace ets2ld
