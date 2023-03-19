#include "ufld/utils.h"

#include <cassert>
#include <cmath>

namespace ufld::utils {

std::vector<uint32_t> ArgMax_0(std::span<const float> input,
                               const std::array<uint32_t, 3>& dimensions) {
  assert(input.size() == dimensions[0] * dimensions[1] * dimensions[2]);
  const auto dim1 = dimensions[0];
  const auto dim2 = dimensions[1];
  const auto dim3 = dimensions[2];
  auto Index = [=](auto d1, auto d2, auto d3) -> uint64_t {
    return d1 * dim2 * dim3 + d2 * dim3 + d3;
  };
  std::vector<uint32_t> result(dim2 * dim3);
  for (uint32_t d2 = 0; d2 < dim2; ++d2) {
    for (uint32_t d3 = 0; d3 < dim3; ++d3) {
      float max = input[Index(0, d2, d3)];
      uint32_t argmax = 0;
      for (uint32_t d1 = 1; d1 < dim1; ++d1) {
        if (input[Index(d1, d2, d3)] > max) {
          max = input[Index(d1, d2, d3)];
          argmax = d1;
        }
      }
      result[Index(0, d2, d3)] = argmax;
    }
  }
  return result;
}

std::vector<double> Linspace(double begin, double end, uint32_t count) {
  assert(count > 0);
  std::vector<double> result(count);
  const double step = end - begin / (count - 1);
  for (uint32_t i = 0; i < count - 1; ++i) {
    result[i] = begin + i * step;
  }
  result[count - 1] = end;
  return result;
}

std::vector<float> Softmax_0(std::span<const float> input,
                             const std::array<uint32_t, 3>& dimensions) {
  assert(input.size() == dimensions[0] * dimensions[1] * dimensions[2]);
  const auto dim1 = dimensions[0];
  const auto dim2 = dimensions[1];
  const auto dim3 = dimensions[2];
  auto Index = [=](auto d1, auto d2, auto d3) -> uint64_t {
    return d1 * dim2 * dim3 + d2 * dim3 + d3;
  };
  std::vector<float> result(input.size());
  for (uint32_t d2 = 0; d2 < dim2; ++d2) {
    for (uint32_t d3 = 0; d3 < dim3; ++d3) {
      float sum = 0;
      for (uint32_t d1 = 0; d1 < dim1; ++d1) {
        sum += std::exp(input[Index(d1, d2, d3)]);
      }
      for (uint32_t d1 = 0; d1 < dim1; ++d1) {
        result[Index(d1, d2, d3)] = std::exp(input[Index(d1, d2, d3)]) / sum;
      }
    }
  }
  return result;
}

}  // namespace ufld::utils
