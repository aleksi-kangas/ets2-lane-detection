#include "ets2ld/utils.h"

#include <array>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include <ShlObj.h>
#include <imgui_internal.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
struct ComInit {
  ComInit() {
    if (FAILED(CoInitialize(nullptr)))
      throw std::runtime_error{"Failed to initialize COM."};
  }
  ~ComInit() { CoUninitialize(); }
};
}  // namespace

namespace ets2ld::utils {
std::optional<std::filesystem::path> BrowseFolderDialog() {
  ComInit com_init{};  // Initialize COM to be able to use the IFileOpenDialog

  CComPtr<IFileOpenDialog> file_open_dialog;
  if (FAILED(file_open_dialog.CoCreateInstance(CLSID_FileOpenDialog)))
    throw std::runtime_error{"Failed to create IFileOpenDialog instance."};

  FILEOPENDIALOGOPTIONS options{};
  if (FAILED(file_open_dialog->GetOptions(&options)))
    throw std::runtime_error{"Failed to get options for IFileOpenDialog."};
  if (FAILED(file_open_dialog->SetOptions(
          options | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM)))
    throw std::runtime_error{"Failed to set options for IFileOpenDialog."};

  if (FAILED(file_open_dialog->Show(nullptr)))
    return std::nullopt;

  CComPtr<IShellItem> selected_item;
  if (FAILED(file_open_dialog->GetResult(&selected_item)))
    throw std::runtime_error{"Failed to get result from IFileOpenDialog."};

  CComHeapPtr<wchar_t> path;
  if (FAILED(selected_item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
    throw std::runtime_error{"Failed to get path from IShellItem."};

  return path.m_pData;
}

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

std::unique_ptr<ufld::ILaneDetector> CreateLaneDetector(
    const std::filesystem::path& model_directory,
    std::variant<ufld::v1::ModelType> variant, ufld::Version version) {
  switch (version) {
    case ufld::Version::kV1:
      return std::make_unique<ufld::v1::LaneDetector>(
          model_directory, std::get<ufld::v1::ModelType>(variant));
    default:
      throw std::runtime_error{"Unsupported version"};
  }
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

std::pair<int32_t, int32_t> QueryPrimaryMonitorResolution() {
  return {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
}

}  // namespace ets2ld::utils

namespace ImGui {
void Spinner(const char* label, float radius, float thickness,
             const ImU32& color) {
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems)
    return;

  const ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);

  const ImVec2 pos = window->DC.CursorPos;
  const ImVec2 size{radius * 2, (radius + style.FramePadding.y) * 2};

  const ImRect bb{pos, ImVec2(pos.x + size.x, pos.y + size.y)};
  ItemSize(bb, style.FramePadding.y);
  if (!ItemAdd(bb, id))
    return;

  // Render
  window->DrawList->PathClear();

  constexpr int32_t num_segments = 30;
  const auto start = static_cast<int32_t>(
      std::abs(std::sinf(static_cast<float>(g.Time) * 1.8f) *
               static_cast<float>((num_segments - 5))));

  const float a_min = std::numbers::pi_v<float> * 2.0f *
                      static_cast<float>(start) /
                      static_cast<float>(num_segments);
  const float a_max = std::numbers::pi_v<float> * 2.0f *
                      static_cast<float>(num_segments - 3) /
                      static_cast<float>(num_segments);

  const ImVec2 centre{pos.x + radius, pos.y + radius + style.FramePadding.y};

  for (int32_t i = 0; i < num_segments; i++) {
    const float a =
        a_min + (static_cast<float>(i) / static_cast<float>(num_segments)) *
                    (a_max - a_min);
    window->DrawList->PathLineTo(
        {centre.x + std::cosf(a + static_cast<float>(g.Time) * 8.0f) * radius,
         centre.y + std::sinf(a + static_cast<float>(g.Time) * 8.0f) * radius});
  }

  window->DrawList->PathStroke(color, false, thickness);
}

}  // namespace ImGui
