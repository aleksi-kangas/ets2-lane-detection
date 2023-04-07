#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

namespace ufld::utils {
/**
 * Compute the argmax of the second dimension of a 4D tensor.
 * @param input 4D tensor as contiguous 1D span
 * @param dimensions dimensions of the 4D tensor
 * @return argmax of the second dimension of the 4D tensor
 */
std::vector<uint32_t> ArgMax_1(std::span<const float> input,
                               const std::array<uint32_t, 4>& dimensions);

/**
 * Generate a vector of evenly spaced numbers over a specified interval [start, end].
 * @param begin start of the interval
 * @param end end of the interval
 * @param count how many numbers to generate
 * @return a vector of evenly spaced numbers over a specified interval [start, end]
 */
std::vector<double> Linspace(double begin, double end, uint32_t count);

/**
 * Compute the softmax of the second dimension of a 4D tensor.
 * @param input 4D tensor as contiguous 1D span
 * @param dimensions dimensions of the 4D tensor
 * @return argmax of the second dimension of the 4D tensor
 */
std::vector<float> Softmax_1(std::span<const float> input,
                             const std::array<uint32_t, 4>& dimensions);
}  // namespace ufld::utils
