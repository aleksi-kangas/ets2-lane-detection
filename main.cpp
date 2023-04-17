#include <memory>
#include <stdexcept>
#include <variant>

#include <opencv2/opencv.hpp>

#include "arguments.h"
#include "dx11/capture.h"
#include "ufld/v1.h"

std::unique_ptr<ufld::ILaneDetector> CreateLaneDetector(
    const std::filesystem::path& model_directory, ufld::Version version,
    std::variant<ufld::v1::ModelType> type) {
  switch (version) {
    case ufld::Version::kV1:
      return std::make_unique<ufld::v1::LaneDetector>(
          model_directory, std::get<ufld::v1::ModelType>(type));
    default:
      throw std::invalid_argument{"Invalid model version."};
  }
}

int main(int argc, char** argv) {
  ets2ld::Arguments arguments = ets2ld::Arguments::Parse(argc, argv);
  dx11::Capture capture{};

  const auto lane_detector = CreateLaneDetector(
      arguments.model_directory, arguments.version, arguments.model_type);

  // TODO: Make these configurable.
  constexpr int32_t kDeviceIndex = 0;
  constexpr int32_t kOutputIndex = 0;
  const auto camera = capture.Start(kDeviceIndex, kOutputIndex);

  while (true) {
    std::optional<cv::Mat> frame = camera->GetLatestFrame();
    if (frame) {
      const auto lanes = lane_detector->Detect(frame.value());
      ufld::VisualizeLanes(lanes, frame.value());

      cv::Mat preview{};
      cv::resize(frame.value(), preview, {1280, 720});
      cv::imshow("Detected Lanes", preview);
      if (cv::waitKey(1) == 27) {  // ESC
        break;
      }
    }
  }
  capture.Stop();
}
