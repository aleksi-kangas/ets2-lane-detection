#include "ufld/utils.h"

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
const std::array<uint32_t, 3> kDimensions = {2, 2, 3};

TEST(UfldUtils, ArgMax_0) {
  auto result = ufld::utils::ArgMax_0(kTensor, kDimensions);
  std::vector<uint32_t> expected = {1, 1, 1, 1, 1, 1};
  EXPECT_EQ(result, expected);
}

TEST(UfldUtils, Linspace) {
  auto result = ufld::utils::Linspace(0, 1, 5);
  std::vector<double> expected = {0, 0.25, 0.5, 0.75, 1};
  EXPECT_EQ(result, expected);
}

TEST(UfldUtils, SoftMax_0) {
  const auto result = ufld::utils::Softmax_0(kTensor, kDimensions);
  const std::vector<float> expected = {
      0.00247262302, 0.00247262302, 0.00247262325, 0.00247262325,
      0.00247262325, 0.00247262325, 0.997527421,   0.997527361,
      0.997527361,   0.997527421,   0.997527361,   0.997527421};
  EXPECT_EQ(result, expected);
}
