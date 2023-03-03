#pragma once

#include <cstdint>
#include <stdexcept>

namespace d3d11_capture {

struct Region {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  Region(int32_t left, int32_t top, int32_t right, int32_t bottom);
};

}  // namespace d3d11_capture
