#include "ets2ld/ui.h"

#include <array>
#include <stdexcept>
#include <tuple>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

namespace {

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

}  // namespace

namespace ets2ld {

UI::UI() {
  CreateUIWindow();

  auto [device, swap_chain, device_context] = CreateDeviceAndSwapChain(hwnd_);
  device_ = device;
  swap_chain_ = swap_chain;
  device_context_ = device_context;
  render_target_view_ = CreateRenderTargetView(device_, swap_chain_);

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

void UI::Render() {
  BeginFrame();

  ImGui::Begin("ETS2 Lane Detection");
  ImGui::Text("Hello, world!");
  ImGui::End();

  EndFrame();
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
        render_target_view_ = CreateRenderTargetView(device_, swap_chain_);
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

void UI::BeginFrame() {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
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

}  // namespace ets2ld
