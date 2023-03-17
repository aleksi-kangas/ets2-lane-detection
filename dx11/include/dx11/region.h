#pragma once

#include <cstdint>

namespace dx11 {

struct Region {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  Region(int32_t left, int32_t top, int32_t right, int32_t bottom);
};

}  // namespace dx11
