#include "ufld/utils.h"

#include <cmath>

namespace ufld::utils {

std::vector<int32_t> ArgMax_0(std::span<const float> input,
                              const std::array<int32_t, 3>& dimensions) {
  const auto dim1 = dimensions[0];
  const auto dim2 = dimensions[1];
  const auto dim3 = dimensions[2];
  auto Index = [=](auto d1, auto d2, auto d3) -> uint64_t {
    return d1 * dim2 * dim3 + d2 * dim3 + d3;
  };
  std::vector<int32_t> result(dim2 * dim3);
  for (int32_t d2 = 0; d2 < dim2; ++d2) {
    for (int32_t d3 = 0; d3 < dim3; ++d3) {
      float max = input[Index(0, d2, d3)];
      int32_t argmax = 0;
      for (int32_t d1 = 1; d1 < dim1; ++d1) {
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

std::vector<double> Linspace(double begin, double end, int32_t count) {
  std::vector<double> result(count);
  const double step = end - begin / (count - 1);
  for (int32_t i = 0; i < count - 1; ++i) {
    result[i] = begin + i * step;
  }
  result[count - 1] = end;
  return result;
}

std::vector<float> Softmax_0(std::span<const float> input,
                             const std::array<int32_t, 3>& dimensions) {
  const auto dim1 = dimensions[0];
  const auto dim2 = dimensions[1];
  const auto dim3 = dimensions[2];
  auto Index = [=](auto d1, auto d2, auto d3) -> uint64_t {
    return d1 * dim2 * dim3 + d2 * dim3 + d3;
  };
  std::vector<float> result(input.size());
  for (int32_t d2 = 0; d2 < dim2; ++d2) {
    for (int32_t d3 = 0; d3 < dim3; ++d3) {
      float sum = 0;
      for (int32_t d1 = 0; d1 < dim1; ++d1) {
        sum += std::exp(input[Index(d1, d2, d3)]);
      }
      for (int32_t d1 = 0; d1 < dim1; ++d1) {
        result[Index(d1, d2, d3)] = std::exp(input[Index(d1, d2, d3)]) / sum;
      }
    }
  }
  return result;
}

}  // namespace ufld::utils
