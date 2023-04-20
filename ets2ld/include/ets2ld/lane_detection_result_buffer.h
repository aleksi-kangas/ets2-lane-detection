#pragma once

#include <cstdint>
#include <deque>
#include <optional>

#include <opencv2/opencv.hpp>

#include "ufld/ufld.h"

namespace ets2ld {

struct LaneDetectionResult {
  std::vector<ufld::Lane> lanes;
  cv::Mat frame;
  cv::Mat preview;
};

/**
 * Buffer for storing lane detection results.
 * Note, this class is not thread-safe. Thread-safety is the responsibility of the caller.
 */
class LaneDetectionResultBuffer {
 public:
  explicit LaneDetectionResultBuffer(uint32_t capacity = 16);

  LaneDetectionResultBuffer(const LaneDetectionResultBuffer&) = delete;
  LaneDetectionResultBuffer& operator=(const LaneDetectionResultBuffer&) =
      delete;

  void AddResult(LaneDetectionResult result);

  [[nodiscard]] std::optional<LaneDetectionResult> GetNewestResult();

 private:
  uint32_t capacity_;
  std::deque<LaneDetectionResult> results_{};
};

}  // namespace ets2ld
