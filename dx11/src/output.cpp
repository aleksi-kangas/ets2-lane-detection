#include "dx11/output.h"

#include <utility>

namespace dx11 {

Output::Output(CComPtr<IDXGIOutput1> dxgi_output)
    : dxgi_output_{std::move(dxgi_output)} {
  dxgi_output_->GetDesc(&dxgi_output_desc_);
}

std::wstring Output::DeviceName() const {
  return {dxgi_output_desc_.DeviceName,
          dxgi_output_desc_.DeviceName + wcslen(dxgi_output_desc_.DeviceName)};
}

std::pair<int32_t, int32_t> Output::Resolution() const {
  return {dxgi_output_desc_.DesktopCoordinates.right -
              dxgi_output_desc_.DesktopCoordinates.left,
          dxgi_output_desc_.DesktopCoordinates.bottom -
              dxgi_output_desc_.DesktopCoordinates.top};
}

int32_t Output::Height() const {
  return dxgi_output_desc_.DesktopCoordinates.bottom -
         dxgi_output_desc_.DesktopCoordinates.top;
}

int32_t Output::Width() const {
  return dxgi_output_desc_.DesktopCoordinates.right -
         dxgi_output_desc_.DesktopCoordinates.left;
}

}  // namespace dx11
