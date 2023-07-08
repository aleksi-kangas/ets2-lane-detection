module;

#include <vector>

#include <opencv2/opencv.hpp>

export module ufld.utils;

export namespace ufld::utils {
/**
 *
 * @param lanes
 * @param image
 */
void DrawLanes(const std::vector<std::vector<cv::Point2f>>& lanes,
               cv::Mat& image);
/**
 *
 * @param inputArea
 * @param image
 */
void DrawInputArea(cv::Rect inputArea, cv::Mat& image);
}  // namespace ufld::utils
