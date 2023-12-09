module;

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include <atlbase.h>
#include <dxgi1_6.h>

export module capture:output;

namespace capture {
class Output {
 public:
  explicit Output(CComPtr<IDXGIOutput6> dxgi_output);

  Output(const Output&) = default;
  Output& operator=(const Output&) = default;

  Output(Output&&) = default;
  Output& operator=(Output&&) = default;

  [[nodiscard]] IDXGIOutput6* DXGIOutput6() const { return dxgi_output_; }

  [[nodiscard]] std::wstring DeviceName() const;
  [[nodiscard]] std::pair<std::int32_t, std::int32_t> Resolution() const;
  [[nodiscard]] std::int32_t Height() const;
  [[nodiscard]] std::int32_t Width() const;

 private:
  CComPtr<IDXGIOutput6> dxgi_output_{nullptr};
  DXGI_OUTPUT_DESC dxgi_output_desc_{};
};
}  // namespace capture

capture::Output::Output(CComPtr<IDXGIOutput6> dxgi_output) : dxgi_output_{std::move(dxgi_output)} {
  if (FAILED(dxgi_output_->GetDesc(&dxgi_output_desc_)))
    throw std::runtime_error{"GetDesc: Failure"};
}

std::wstring capture::Output::DeviceName() const {
  return {dxgi_output_desc_.DeviceName, dxgi_output_desc_.DeviceName + wcslen(dxgi_output_desc_.DeviceName)};
}

std::pair<std::int32_t, std::int32_t> capture::Output::Resolution() const {
  return {dxgi_output_desc_.DesktopCoordinates.right - dxgi_output_desc_.DesktopCoordinates.left,
          dxgi_output_desc_.DesktopCoordinates.bottom - dxgi_output_desc_.DesktopCoordinates.top};
}

std::int32_t capture::Output::Height() const {
  return dxgi_output_desc_.DesktopCoordinates.bottom - dxgi_output_desc_.DesktopCoordinates.top;
}

std::int32_t capture::Output::Width() const {
  return dxgi_output_desc_.DesktopCoordinates.right - dxgi_output_desc_.DesktopCoordinates.left;
}
