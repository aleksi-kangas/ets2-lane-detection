#pragma once

#include <vector>

#include <atlbase.h>
#include <dxgi1_2.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace d3d11_capture {

std::vector<CComPtr<IDXGIAdapter1>> EnumerateDXGIAdapters();

}  // namespace d3d11_capture
