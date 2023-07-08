module;

#include <cstdint>
#include <string>
#include <utility>

#include <atlbase.h>
#include <dxgi1_2.h>

export module capture.output;

export namespace capture {
class Output {
 public:
  explicit Output(CComPtr<IDXGIOutput1> dxgi_output);

  [[nodiscard]] IDXGIOutput1* DXGIOutput1() const { return dxgi_output_; }

  [[nodiscard]] std::wstring DeviceName() const;
  [[nodiscard]] std::pair<std::int32_t, std::int32_t> Resolution() const;
  [[nodiscard]] std::int32_t Height() const;
  [[nodiscard]] std::int32_t Width() const;

 private:
  CComPtr<IDXGIOutput1> dxgi_output_{nullptr};
  DXGI_OUTPUT_DESC dxgi_output_desc_{};
};
}  // namespace capture
