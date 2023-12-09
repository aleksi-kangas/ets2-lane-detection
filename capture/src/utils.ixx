module;

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include <atlbase.h>
#include <dxgi.h>
#include <dxgi1_6.h>

export module capture:utils;

namespace capture::utils {
std::vector<CComPtr<IDXGIAdapter4>> EnumerateDXGIAdapters();

export std::pair<std::int32_t, std::int32_t> QueryPrimaryMonitorResolution();
}  // namespace capture::utils

std::vector<CComPtr<IDXGIAdapter4>> capture::utils::EnumerateDXGIAdapters() {
  CComPtr<IDXGIFactory6> dxgi_factory6{nullptr};
  if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgi_factory6))))
    throw std::runtime_error{"CreateDXGIFactory1: Failure"};
  std::vector<CComPtr<IDXGIAdapter4>> dxgi_adapters{};
  std::uint32_t i{0};
  while (true) {
    CComPtr<IDXGIAdapter1> dxgi_adapter1{nullptr};
    if (FAILED(dxgi_factory6->EnumAdapters1(i++, &dxgi_adapter1)))
      break;
    CComQIPtr<IDXGIAdapter4> dxgi_adapter4{dxgi_adapter1};
    if (dxgi_adapter4 == nullptr)
      throw std::runtime_error{"DXGIAdapter4 not supported"};
    dxgi_adapters.emplace_back(dxgi_adapter4);
  }
  if (dxgi_adapters.empty())
    throw std::runtime_error{"No DXGIAdapters found"};
  return dxgi_adapters;
}

std::pair<std::int32_t, std::int32_t> capture::utils::QueryPrimaryMonitorResolution() {
  return {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
}
