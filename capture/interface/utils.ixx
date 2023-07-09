module;

#include <cstdint>
#include <utility>
#include <vector>

#include <atlbase.h>
#include <dxgi.h>

export module capture.utils;

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
