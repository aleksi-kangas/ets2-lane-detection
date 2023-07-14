#include <gtest/gtest.h>
#include <xtensor/xadapt.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xtensor.hpp>

import ufld.math;

TEST(UfldMath, SoftMax) {
  {  // 2D (1D)
    const xt::xarray<float> a{{1.0f, 2.0f, 3.0f, 6.0f}};
    const xt::xtensor<float, 2> tensor = xt::adapt(a, {1, 4});
    const xt::xtensor<float, 2> result = ufld::math::SoftMax<1>(tensor);
    const xt::xarray<float> expected{
        {0.00626879, 0.01704033, 0.04632042, 0.93037047}};
    for (std::size_t i = 0; i < expected.shape()[0]; ++i) {
      for (std::size_t j = 0; j < expected.shape()[1]; ++j) {
        EXPECT_NEAR(result(i, j), expected(i, j), 1e-6f);
      }
    }
  }
  {  // 2D
    const xt::xarray<float> a{{1.0f, 2.0f, 3.0f, 6.0f},
                              {2.0f, 4.0f, 5.0f, 6.0f},
                              {1.0f, 2.0f, 3.0f, 6.0f}};
    const xt::xtensor<float, 2> tensor = xt::adapt(a, {3, 4});
    const xt::xtensor<float, 2> result = ufld::math::SoftMax<1>(tensor);
    const xt::xarray<float> expected{
        {0.00626879f, 0.01704033f, 0.04632042f, 0.93037047f},
        {0.01203764f, 0.08894682f, 0.24178252f, 0.65723302f},
        {0.00626879f, 0.01704033f, 0.04632042f, 0.93037047f}};
    for (std::size_t i = 0; i < expected.shape()[0]; ++i) {
      for (std::size_t j = 0; j < expected.shape()[1]; ++j) {
        EXPECT_NEAR(result(i, j), expected(i, j), 1e-6f);
      }
    }
  }
}
