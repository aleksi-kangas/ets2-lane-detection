#include "ets2ld/lane_detection_result_buffer.h"

namespace ets2ld {

LaneDetectionResultBuffer::LaneDetectionResultBuffer(uint32_t capacity)
    : capacity_{capacity} {}

void LaneDetectionResultBuffer::AddResult(ets2ld::LaneDetectionResult result) {
  if (results_.size() >= capacity_) {
    results_.pop_back();
  }
  results_.push_front(std::move(result));
}

[[nodiscard]] std::optional<LaneDetectionResult>
LaneDetectionResultBuffer::GetNewestResult() {
  if (results_.empty()) {
    return std::nullopt;
  }
  return results_.front();
}

}  // namespace ets2ld
