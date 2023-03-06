#include <opencv2/opencv.hpp>

#include "d3d11_capture/factory.h"
#include "ufld/v1.h"

int main() {
  d3d11_capture::Factory d3d11_camera_factory{};
  auto& camera = d3d11_camera_factory.Create();

  ufld::v1::LaneDetector lane_detector{ufld::v1::ModelType::kCULane};

  camera.StartCapture();
  while (true) {
    std::optional<cv::Mat> frame = camera.GetLatestFrame();
    if (frame) {
      const auto lanes = lane_detector.Detect(frame.value());
      ufld::VisualizeLanes(lanes, frame.value());
      cv::imshow("Detected Lanes", frame.value());
      if (cv::waitKey(1) == 27) {
        break;
      }
    }
  }
  camera.StopCapture();
}
