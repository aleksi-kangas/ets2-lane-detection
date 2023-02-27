#include "ufld/ufld.h"

#include <array>
#include <cassert>

namespace ufld {

void VisualizeLanes(const std::vector<Lane>& lanes, cv::Mat& image) {
  assert(lanes.size() <= 4);
  const std::array<cv::Scalar, 4> kLaneColors{{{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}}};
  for (auto i = 0; i < lanes.size(); ++i) {
    const auto& lane = lanes[i];
    const auto& color = kLaneColors[i];
    constexpr auto kRadius = 10;
    constexpr auto kThickness = 2;
    for (auto j = 0; j < lane.size() - 1; ++j) {
      cv::circle(image, lane[j], kRadius, color, kThickness);
    }
  }
}

}  // namespace ufld
