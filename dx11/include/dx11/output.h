#pragma once

#include <string>
#include <utility>

#include "dx11/common.h"

namespace dx11 {

class Output {
 public:
  explicit Output(CComPtr<IDXGIOutput1> dxgi_output);

  [[nodiscard]] IDXGIOutput1* DXGIOutput1() const { return dxgi_output_; }

  [[nodiscard]] std::wstring DeviceName() const;
  [[nodiscard]] std::pair<int32_t, int32_t> Resolution() const;
  [[nodiscard]] int32_t Height() const;
  [[nodiscard]] int32_t Width() const;

 private:
  CComPtr<IDXGIOutput1> dxgi_output_{nullptr};
  DXGI_OUTPUT_DESC dxgi_output_desc_{};
};

}  // namespace dx11
