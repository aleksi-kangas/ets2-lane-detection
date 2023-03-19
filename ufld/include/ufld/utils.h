#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace ufld::utils {

/**
 * Compute the argmax of the first dimension of a 3D tensor.
 * @param input 3D tensor as contiguous 1D span
 * @param dimensions dimensions of the 3D tensor
 * @return argmax of the first dimension of the 3D tensor
 */
std::vector<int32_t> ArgMax_0(std::span<const float> input,
                              const std::array<int32_t, 3>& dimensions);

/**
 * Generate a vector of evenly spaced numbers over a specified interval [start, end].
 * @param begin start of the interval
 * @param end end of the interval
 * @param count how many numbers to generate
 * @return a vector of evenly spaced numbers over a specified interval [start, end]
 */
std::vector<double> Linspace(double begin, double end, int32_t count);

/**
 * Compute the softmax of the first dimension of a 3D tensor.
 * @param input 3D tensor as contiguous 1D span
 * @param dimensions dimensions of the 3D tensor
 * @return argmax of the first dimension of the 3D tensor
 */
std::vector<float> Softmax_0(std::span<const float> input,
                             const std::array<int32_t, 3>& dimensions);

}  // namespace ufld::utils
