#pragma once

#include <string>
#include <utility>

#include <atlbase.h>
#include <dxgi1_2.h>

namespace d3d11_capture {

class Output {
 public:
  explicit Output(CComPtr<IDXGIOutput1> dxgi_output);

  [[nodiscard]] IDXGIOutput1* DXGIOutput1() const { return dxgi_output_; }

  [[nodiscard]] std::string DeviceName() const;
  [[nodiscard]] std::pair<int32_t, int32_t> Resolution() const;

 private:
  CComPtr<IDXGIOutput1> dxgi_output_{nullptr};
  DXGI_OUTPUT_DESC dxgi_output_desc_{};
};

}  // namespace d3d11_capture
