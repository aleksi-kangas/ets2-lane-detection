module;

#include <array>
#include <vector>

#include <opencv2/opencv.hpp>

export module ufld:draw;

namespace ufld::draw {
void DrawLanes(const std::array<std::vector<cv::Point2f>, 4>& lanes, cv::Mat& image,
               const std::array<bool, 4>& lanes_to_draw = {{true, true, true, true}});
void DrawInputArea(cv::Rect input_area, cv::Mat& image);
}  // namespace ufld::draw

void DrawLanePoints(const std::array<std::vector<cv::Point2f>, 4>& lanes, cv::Mat& image,
                    const std::array<bool, 4>& lanes_to_draw = {true, true, true, true}) {
  const std::array<cv::Scalar, 4> kLaneColors{
      {{255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255}, {255, 255, 0, 255}}};
  for (uint32_t lane_index = 0; lane_index < lanes.size(); ++lane_index) {
    if (!lanes_to_draw[lane_index])
      continue;
    for (const auto j : lanes[lane_index]) {
      cv::circle(image, j, /*radius=*/5, kLaneColors[lane_index], /*thickness=*/-1);
    }
  }
}

void DrawCenterLaneMask(const std::array<std::vector<cv::Point2f>, 4>& lanes, cv::Mat& image) {
  const auto& left_divider = lanes[1];
  const auto& right_divider = lanes[2];
  if (left_divider.empty() || right_divider.empty())
    return;
  cv::Mat mask = image.clone();
  std::vector<cv::Point> lane_polygon{};
  lane_polygon.reserve(left_divider.size() + right_divider.size());
  lane_polygon.insert(lane_polygon.end(), left_divider.begin(), left_divider.end());
  lane_polygon.insert(lane_polygon.end(), right_divider.rbegin(), right_divider.rend());
  cv::fillPoly(mask, {lane_polygon}, cv::Scalar{255, 191, 0, 255});
  cv::addWeighted(mask, 0.3, image, 0.7, 0, image);
}

void ufld::draw::DrawLanes(const std::array<std::vector<cv::Point2f>, 4>& lanes, cv::Mat& image,
                           const std::array<bool, 4>& lanes_to_draw) {
  DrawLanePoints(lanes, image, lanes_to_draw);
  DrawCenterLaneMask(lanes, image);
}

void ufld::draw::DrawInputArea(cv::Rect input_area, cv::Mat& image) {
  cv::rectangle(image, input_area, cv::Scalar{255, 0, 0, 255}, /*thickness=*/5);
}
