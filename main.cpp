#include <opencv2/opencv.hpp>

#include "d3d11_capture/factory.h"
#include "ufld/v1.h"

int main() {
  auto& camera = d3d11_capture::Factory::Instance().Create();
  camera.StartCapture();

  ufld::v1::LaneDetector lane_detector{ufld::v1::ModelType::kCULane};

  while (true) {
    std::optional<cv::Mat> frame = camera.GetLatestFrame();
    if (frame) {
      cv::imshow("Capture", frame.value());
      if (cv::waitKey(1) == 27) {
        break;
      }
    }
  }
  camera.StopCapture();
}
