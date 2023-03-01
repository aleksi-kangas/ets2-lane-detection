#include "d3d11_capture/output.h"

#include <utility>

namespace d3d11_capture {

Output::Output(CComPtr<IDXGIOutput1> dxgi_output) : dxgi_output_{std::move(dxgi_output)} {
  dxgi_output_->GetDesc(&dxgi_output_desc_);
}

std::string Output::DeviceName() const {
  return {dxgi_output_desc_.DeviceName, dxgi_output_desc_.DeviceName + wcslen(dxgi_output_desc_.DeviceName)};
}

std::pair<int32_t, int32_t> Output::Resolution() const {
  return {dxgi_output_desc_.DesktopCoordinates.right - dxgi_output_desc_.DesktopCoordinates.left,
          dxgi_output_desc_.DesktopCoordinates.bottom - dxgi_output_desc_.DesktopCoordinates.top};
}

}  // namespace d3d11_capture
