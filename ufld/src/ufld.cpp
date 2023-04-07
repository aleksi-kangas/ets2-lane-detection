#include "ufld/ufld.h"

#include <array>
#include <cassert>

namespace ufld {

void VisualizeLanes(const std::vector<Lane>& lanes, cv::Mat& image) {
  assert(lanes.size() <= 4);
  const std::array<cv::Scalar, 4> kLaneColors{{{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}}};
  for (uint32_t i = 0; i < lanes.size(); ++i) {
    const auto& lane = lanes[i];
    const auto& color = kLaneColors[i];
    constexpr auto kRadius = 5;
    constexpr auto kThickness = -1;  // Fill
    for (const auto j : lane) {
      cv::circle(image, j, kRadius, color, kThickness);
    }
  }
}

std::vector<Lane> ILaneDetector::Detect(const cv::Mat& image) {
  const auto input = Preprocess(image);
  const auto outputs = Inference(input);
  return PredictionsToLanes(outputs, image.cols, image.rows);
}

}  // namespace ufld
