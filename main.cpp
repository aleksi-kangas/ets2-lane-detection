#include <opencv2/opencv.hpp>

#include "d3d11_capture/capture.h"
#include "ufld/v1.h"

int main() {
  d3d11_capture::Capture capture{};
  constexpr int32_t kDeviceIndex = 0;
  constexpr int32_t kOutputIndex = 0;
  const auto camera = capture.Start(kDeviceIndex, kOutputIndex);
  ufld::v1::LaneDetector lane_detector{ufld::v1::ModelType::kCULane};
  while (true) {
    std::optional<cv::Mat> frame = camera->GetLatestFrame();
    if (frame) {
      const auto lanes = lane_detector.Detect(frame.value());
      ufld::VisualizeLanes(lanes, frame.value());
      cv::imshow("Detected Lanes", frame.value());
      if (cv::waitKey(1) == 27) {
        break;
      }
    }
  }
  capture.Stop();
}
