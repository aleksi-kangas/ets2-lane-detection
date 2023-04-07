#include "ufld/utils.h"

#include <array>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

// 3D tensor with dimensions 2x2x3
// ---                 --- |
// |   | ---      --- |    |
// |   | [  1  2  3 ] |    |
// |   | [  4  5  6 ] |    |
// |   | ---      --- |    |
// |   | ---      --- |    |
// |   | [  7  8  9 ] |    |
// |   | [ 10 11 12 ] |    |
// |   | ---      --- |    |
// ---                 --- |
const std::vector<float> kTensor = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
const std::array<uint32_t, 4> kDimensions = {1, 2, 2, 3};

TEST(UfldUtils, ArgMax_1) {
  auto result = ufld::utils::ArgMax_1(kTensor, kDimensions);
  std::vector<uint32_t> expected = {1, 1, 1, 1, 1, 1};
  EXPECT_EQ(result, expected);
}

TEST(UfldUtils, Linspace) {
  auto result = ufld::utils::Linspace(0, 1, 5);
  std::vector<double> expected = {0, 0.25, 0.5, 0.75, 1};
  EXPECT_EQ(result, expected);
}

TEST(UfldUtils, SoftMax_1) {
  const auto result = ufld::utils::Softmax_1(kTensor, kDimensions);
  const std::vector<float> expected = {
      0.00247262302, 0.00247262302, 0.00247262325, 0.00247262325,
      0.00247262325, 0.00247262325, 0.997527421,   0.997527361,
      0.997527361,   0.997527421,   0.997527361,   0.997527421};
  EXPECT_EQ(result, expected);
}
