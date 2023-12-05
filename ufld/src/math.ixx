module;

#include <xtensor/xexpression.hpp>
#include <xtensor/xmanipulation.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xoperation.hpp>

export module ufld:math;

namespace ufld::math {
/**
 * Compute softmax expression of the given expression along the given axis.
 * @tparam A axis
 * @tparam E expression type
 * @param x expression
 * @return softmax expression of the given expression along the given axis
 */
export template <std::size_t A, class E>
auto SoftMax(const xt::xexpression<E>& x) {
  return xt::exp(x.derived_cast()) / xt::expand_dims(xt::sum(xt::exp(x.derived_cast()), A), A);
}
}  // namespace ufld::math
