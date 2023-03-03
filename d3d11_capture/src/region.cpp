#include "d3d11_capture/region.h"

#include <stdexcept>

namespace d3d11_capture {

Region::Region(int32_t left, int32_t top, int32_t right, int32_t bottom)
    : left{left}, top{top}, right{right}, bottom{bottom} {
  if (left >= right || top >= bottom) {
    throw std::invalid_argument{"Invalid region"};
  }
}

}  // namespace d3d11_capture
