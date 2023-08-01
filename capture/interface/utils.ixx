module;

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include <atlbase.h>
#include <dxgi.h>

export module capture:utils;

export namespace capture::utils {
/**
 *
 * @return
 */
std::vector<CComPtr<IDXGIAdapter1>> EnumerateDXGIAdapters();

/**
 *
 * @return
 */
std::pair<std::int32_t, std::int32_t> QueryPrimaryMonitorResolution();
}  // namespace capture::utils

// -------- Implementation --------

std::vector<CComPtr<IDXGIAdapter1>> capture::utils::EnumerateDXGIAdapters() {

  CComPtr<IDXGIFactory1> dxgi_factory1{nullptr};
  HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgi_factory1));
  if (FAILED(hr)) {
    throw std::runtime_error{"Failed to create DXGI factory"};
  }
  std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters{};
  CComPtr<IDXGIAdapter1> dxgi_adapter1{nullptr};
  for (UINT i = 0; dxgi_factory1->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
    dxgi_adapters.push_back(dxgi_adapter1);
    dxgi_adapter1.Release();
  }
  return dxgi_adapters;
}

std::pair<std::int32_t, std::int32_t> capture::utils::QueryPrimaryMonitorResolution() {
  return {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
}
