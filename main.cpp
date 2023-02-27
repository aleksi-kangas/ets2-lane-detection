#include <opencv2/opencv.hpp>

#include "ufld/v1.h"

int main() {
  auto image = cv::imread("image.jpg");
  ufld::v1::LaneDetector lane_detector{ufld::v1::ModelType::kCULane};
  const auto lanes = lane_detector.Detect(image);
  ufld::VisualizeLanes(lanes, image);
  cv::imshow("Image", image);
  cv::waitKey(0);
}
