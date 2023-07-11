module;

#include <array>
#include <cassert>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <utility>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

module ets2ld.ui;

import capture;
import ets2ld.utils;
import ufld;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

ets2ld::UI::UI() {
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

ets2ld::UI::~UI() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  ::DestroyWindow(hwnd_);
  ::UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
}

bool ets2ld::UI::BeginFrame() {
  if (!PollEvents())
    return false;

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  return true;
}

void ets2ld::UI::RenderSettings(bool lane_detection_initializing,
                                bool lane_detection_active) {
  ImGui::Begin("Settings");
  {
    RenderSettingsGeneral(lane_detection_initializing);
    RenderSettingsModel(lane_detection_initializing, lane_detection_active);
    RenderSettingsCapture(lane_detection_initializing, lane_detection_active);
  }
  ImGui::End();
}

void ets2ld::UI::UpdatePreview(const cv::Mat& preview) {
  D3D11_TEXTURE2D_DESC texture_desc{};
  texture_desc.Width = preview.cols;
  texture_desc.Height = preview.rows;
  texture_desc.MipLevels = 1;
  texture_desc.ArraySize = 1;
  texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texture_desc.CPUAccessFlags = 0;
  texture_desc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA subresource_data{};
  subresource_data.pSysMem = preview.data;
  subresource_data.SysMemPitch = preview.cols * 4;
  subresource_data.SysMemSlicePitch = 0;

  CComPtr<ID3D11Texture2D> texture;
  device_->CreateTexture2D(&texture_desc, &subresource_data, &texture);

  D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
  shader_resource_view_desc.Format = texture_desc.Format;
  shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shader_resource_view_desc.Texture2D.MipLevels = 1;
  shader_resource_view_desc.Texture2D.MostDetailedMip = 0;

  CComPtr<ID3D11ShaderResourceView> shader_resource_view;
  device_->CreateShaderResourceView(texture, &shader_resource_view_desc,
                                    &shader_resource_view);

  preview_texture_ = texture;
  preview_srv_ = shader_resource_view;
}

void ets2ld::UI::RenderPreview(bool lane_detection_initializing) {
  ImGui::Begin("Preview");
  {
    if (lane_detection_initializing) {
      ImGui::Spinner("InitializingSpinner", 10, 2,
                     ImGui::GetColorU32(ImGuiCol_Text));
      ImGui::SameLine();
      ImGui::Text("Initializing OnnxRuntime and loading model...");
    } else {
      ImGui::Image(reinterpret_cast<void*>(preview_srv_.p),
                   ImGui::GetContentRegionAvail(), ImVec2{0, 0}, ImVec2{1, 1});
    }
  }
  ImGui::End();
}

void ets2ld::UI::UpdateStatistics(
    const ufld::LaneDetectionStatistics& statistics) {
  statistics_ = statistics;
}

void ets2ld::UI::RenderStatistics() {
  ImGui::Begin("Statistics");
  ImGui::Text(
      std::format("Pre-process duration: {:.1f} ms",
                  static_cast<float>(statistics_.pre_process_duration.count()))
          .c_str());
  ImGui::Text(
      std::format("Inference duration: {:.1f} ms",
                  static_cast<float>(statistics_.inference_duration.count()))
          .c_str());
  ImGui::Text(
      std::format("Post-process duration: {:.1f} ms",
                  static_cast<float>(statistics_.post_process_duration.count()))
          .c_str());
  ImGui::End();
}

void ets2ld::UI::EndFrame() {
  ImGui::Render();
  device_context_->OMSetRenderTargets(1, &render_target_view_.p, nullptr);
  constexpr auto kClearColor = std::array<float, 4>{0.45f, 0.55f, 0.60f, 1.00f};
  device_context_->ClearRenderTargetView(render_target_view_,
                                         kClearColor.data());
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  swap_chain_->Present(1, 0);  // VSYNC
}

void ets2ld::UI::SetOnLaneDetectionEnableChanged(
    std::function<void(bool, capture::Settings, ufld::Settings)> callback) {
  on_lane_detection_enable_changed_ = std::move(callback);
}

void ets2ld::UI::ShowErrorMessage(const std::string& message) {
  ::MessageBox(hwnd_, message.c_str(), "Error", MB_ICONERROR);
}

void ets2ld::UI::CreateUIWindow() {
  wc_.cbSize = sizeof(wc_);
  wc_.style = CS_CLASSDC;
  wc_.lpfnWndProc = WndProcWrapper;
  wc_.hInstance = ::GetModuleHandleW(nullptr);
  wc_.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
  wc_.lpszClassName = L"ETS2 Lane Detection";
  ::RegisterClassExW(&wc_);

  hwnd_ = CreateWindowExW(0, wc_.lpszClassName, wc_.lpszClassName,
                          WS_OVERLAPPEDWINDOW, 300, 300, 1280, 720, nullptr,
                          nullptr, wc_.hInstance, this);
  if (!hwnd_) {
    throw std::runtime_error{"Failed to create window"};
  }
}

LRESULT ets2ld::UI::WndProcWrapper(HWND hwnd, UINT message, WPARAM wparam,
                                   LPARAM lparam) {
  if (message == WM_NCCREATE) {
    auto cs = reinterpret_cast<CREATESTRUCT*>(lparam);
    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
  }
  auto ui = reinterpret_cast<UI*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  if (ui) {
    return ui->WndProc(hwnd, message, wparam, lparam);
  }
  return ::DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT ets2ld::UI::WndProc(HWND hwnd, UINT message, WPARAM wparam,
                            LPARAM lparam) {
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
      return ::DefWindowProcW(hwnd, message, wparam, lparam);
  }
  return ::DefWindowProcW(hwnd, message, wparam, lparam);
}

bool ets2ld::UI::PollEvents() {
  MSG msg;
  while (::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    ::TranslateMessage(&msg);
    ::DispatchMessageW(&msg);
    if (msg.message == WM_QUIT) {
      return false;
    }
  }
  return true;
}

void ets2ld::UI::RenderSettingsGeneral(bool lane_detection_initializing) {
  ImGui::SeparatorText("General");
  ImGui::BeginDisabled(lane_detection_initializing);
  {
    static bool lane_detection_enabled{false};
    if (ImGui::Checkbox("Enable", &lane_detection_enabled)) {
      if (on_lane_detection_enable_changed_)
        on_lane_detection_enable_changed_(lane_detection_enabled,
                                          capture_settings_, ufld_settings_);
    }
  }
  ImGui::EndDisabled();
}

void ets2ld::UI::RenderSettingsModel(bool lane_detection_initializing,
                                     bool lane_detection_active) {
  ImGui::SeparatorText("Model");
  ImGui::BeginDisabled(lane_detection_initializing || lane_detection_active);
  {
    // Models directory
    static std::filesystem::path chosen_model_directory =
        std::filesystem::current_path() / "models";
    ImGui::Text("Models directory: %s",
                chosen_model_directory.string().c_str());
    if (ImGui::Button("Browse...")) {
      auto path = utils::BrowseFolderDialog();
      if (path && !path->empty()) {
        chosen_model_directory = std::move(path.value());
      }
    }

    // Model selection
    ImGui::Text("Ultra-Fast-Lane-Detection");
    static constexpr std::array<const char*, 1> kModelVersionComboItems = {
        "V1"};
    static int chosen_model_version = 0;
    ImGui::Combo("Version", &chosen_model_version,
                 kModelVersionComboItems.data(),
                 static_cast<int32_t>(kModelVersionComboItems.size()));

    static constexpr std::array<const char*, 2> kModelVariantComboItemsV1 = {
        "CULane", "TuSimple"};
    static int chosen_model_variant_v1 = 0;

    switch (chosen_model_version) {
      case 0:
        static_assert(0 == static_cast<int32_t>(ufld::Version::kV1));
        ImGui::Combo("Variant", &chosen_model_variant_v1,
                     kModelVariantComboItemsV1.data(),
                     static_cast<int32_t>(kModelVariantComboItemsV1.size()));
        break;
      default:
        assert(false);
    }

    ufld_settings_.model_directory = chosen_model_directory;
    switch (chosen_model_version) {
      case 0:
        static_assert(0 == static_cast<int32_t>(ufld::Version::kV1));
        ufld_settings_.version = ufld::Version::kV1;
        switch (chosen_model_variant_v1) {
          case 0:
            static_assert(0 ==
                          static_cast<int32_t>(ufld::v1::Variant::kCULane));
            ufld_settings_.variant = ufld::v1::Variant::kCULane;
            break;
          case 1:
            static_assert(1 ==
                          static_cast<int32_t>(ufld::v1::Variant::kTuSimple));
            ufld_settings_.variant = ufld::v1::Variant::kTuSimple;
            break;
          default:
            assert(false);
        }
        break;
      default:
        assert(false);
    }
  }
  ImGui::EndDisabled();
}

void ets2ld::UI::RenderSettingsCapture(bool lane_detection_initializing,
                                       bool lane_detection_active) {
  static const auto kPrimaryMonitorResolution =
      capture::utils::QueryPrimaryMonitorResolution();

  ImGui::SeparatorText("Capture");
  ImGui::BeginDisabled(lane_detection_initializing || lane_detection_active);
  {
    ImGui::SliderInt(
        "X", &capture_settings_.region.x, 0,
        kPrimaryMonitorResolution.first - capture_settings_.region.width);
    ImGui::SliderInt(
        "Y", &capture_settings_.region.y, 0,
        kPrimaryMonitorResolution.second - capture_settings_.region.height);
    ImGui::SliderInt(
        "Width", &capture_settings_.region.width, 1,
        kPrimaryMonitorResolution.first - capture_settings_.region.x);
    ImGui::SliderInt(
        "Height", &capture_settings_.region.height, 1,
        kPrimaryMonitorResolution.second - capture_settings_.region.y);
  }
  ImGui::EndDisabled();
}
