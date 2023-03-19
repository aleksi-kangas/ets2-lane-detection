#pragma once

#include <vector>

#include <atlbase.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace dx11 {

std::vector<CComPtr<IDXGIAdapter1>> EnumerateDXGIAdapters();

}  // namespace dx11
