module;

#include <vector>

#include <atlbase.h>
#include <dxgi.h>

export module capture.utils;

export namespace capture {
/**
 *
 * @return
 */
std::vector<CComPtr<IDXGIAdapter1>> EnumerateDXGIAdapters();
}  // namespace capture
