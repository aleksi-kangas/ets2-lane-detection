module;

#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <dxgi.h>

module capture.utils;

std::vector<CComPtr<IDXGIAdapter1>> capture::EnumerateDXGIAdapters() {

  CComPtr<IDXGIFactory1> dxgi_factory1{nullptr};
  HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
                                  reinterpret_cast<void**>(&dxgi_factory1));
  if (FAILED(hr)) {
    throw std::runtime_error{"Failed to create DXGI factory"};
  }
  std::vector<CComPtr<IDXGIAdapter1>> dxgi_adapters{};
  CComPtr<IDXGIAdapter1> dxgi_adapter1{nullptr};
  for (UINT i = 0;
       dxgi_factory1->EnumAdapters1(i, &dxgi_adapter1) != DXGI_ERROR_NOT_FOUND;
       ++i) {
    dxgi_adapters.push_back(dxgi_adapter1);
    dxgi_adapter1.Release();
  }
  return dxgi_adapters;
}
