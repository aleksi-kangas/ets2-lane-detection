module;

#include <cstdlib>
#include <type_traits>

#include "xtensor/xmanipulation.hpp"
#include "xtensor/xmath.hpp"
#include "xtensor/xtensor.hpp"

export module ufld.math;

export namespace ufld::math {
/**
 * Compute softmax of the given tensor along the given axis.
 * @tparam A axis
 * @tparam T data type
 * @tparam D tensor dimension
 * @param tensor input tensor
 * @return softmax of the given tensor along the given axis
 */
// clang-format off
template <std::size_t A, typename T, std::size_t D>
requires((std::is_same_v<T, float> || std::is_same_v<T, double>) && D > 1 && A < D)
auto SoftMax(const xt::xtensor<T, D>& tensor) {
  return xt::exp(tensor) / xt::expand_dims(xt::sum(xt::exp(tensor), A), A);
}
// clang-format on
}  // namespace ufld::math
