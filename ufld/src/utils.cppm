module;

#include <array>
#include <cassert>
#include <vector>

#include <opencv2/opencv.hpp>

module ufld:utils;

namespace ufld::utils {
/**
 * Draw the input lanes on the image.
 * @param lanes input lanes
 * @param image image to draw on
 */
void DrawLanes(const std::vector<std::vector<cv::Point2f>>& lanes,
               cv::Mat& image);
/**
 * Draw the input area on the image.
 * @param inputArea input area
 * @param image     image to draw on
 */
void DrawInputArea(cv::Rect inputArea, cv::Mat& image);
}  // namespace ufld::utils

// -------- Implementation --------

void DrawLanePoints(const std::vector<std::vector<cv::Point2f>>& lanes,
                    cv::Mat& image,
                    std::array<bool, 4> lanes_to_draw = {true, true, true,
                                                         true}) {
  assert(lanes.size() <= 4);
  const std::array<cv::Scalar, 4> kLaneColors{{{255, 0, 0, 255},
                                               {0, 255, 0, 255},
                                               {0, 0, 255, 255},
                                               {255, 255, 0, 255}}};
  for (uint32_t lane_index = 0; lane_index < lanes.size(); ++lane_index) {
    if (!lanes_to_draw[lane_index])
      continue;
    const auto& lane = lanes[lane_index];
    constexpr auto kRadius = 5;
    constexpr auto kThickness = -1;  // Fill
    for (const auto j : lane) {
      cv::circle(image, j, kRadius, kLaneColors[lane_index], kThickness);
    }
  }
}

void DrawCenterLaneMask(const std::vector<std::vector<cv::Point2f>>& lanes,
                        cv::Mat& image) {
  assert(lanes.size() <= 4);
  const auto& left_divider = lanes[1];
  const auto& right_divider = lanes[2];
  if (left_divider.empty() || right_divider.empty())
    return;
  cv::Mat mask = image.clone();
  std::vector<cv::Point> lane_polygon{};
  lane_polygon.reserve(left_divider.size() + right_divider.size());
  lane_polygon.insert(lane_polygon.end(), left_divider.begin(),
                      left_divider.end());
  lane_polygon.insert(lane_polygon.end(), right_divider.rbegin(),
                      right_divider.rend());
  cv::fillPoly(mask, {lane_polygon}, cv::Scalar{255, 191, 0, 255});
  cv::addWeighted(mask, 0.3, image, 0.7, 0, image);
}

void ufld::utils::DrawLanes(const std::vector<std::vector<cv::Point2f>>& lanes,
                            cv::Mat& image) {
  assert(lanes.size() <= 4);
  DrawLanePoints(lanes, image, {false, true, true, false});
  DrawCenterLaneMask(lanes, image);
}

void ufld::utils::DrawInputArea(cv::Rect inputArea, cv::Mat& image) {
  cv::rectangle(image, inputArea, cv::Scalar{255, 0, 0, 255}, /* thickness=*/5);
}
