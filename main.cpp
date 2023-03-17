#include <cstdint>
#include <iostream>
#include <variant>

#include <opencv2/opencv.hpp>

#include "dx11/capture.h"
#include "ufld/v1.h"

/**
 * The following command line arguments are supported:
 * -mv: The major version of the model to use.
 *      Valid values are: 1
 *      Default is: 1
 * -mt: The model type to use, depends on -mv.
 *      Valid values are: V1 -> [CULane, TuSimple]
 *      Default is: V1 -> CULane
 */
struct Arguments {
  ufld::Version version{ufld::Version::kV1};
  std::variant<ufld::v1::ModelType> model_type{ufld::v1::ModelType::kCULane};

  static Arguments Parse(int argc, char** argv);

 private:
  static ufld::Version ParseVersion(int32_t argc, char** argv, int32_t index);
  static std::variant<ufld::v1::ModelType> ParseModelType(
      int32_t argc, char** argv, int32_t index, ufld::Version version);
};

int main(int argc, char** argv) {
  Arguments options = Arguments::Parse(argc, argv);
  dx11::Capture capture{};
  constexpr int32_t kDeviceIndex = 0;
  constexpr int32_t kOutputIndex = 0;
  const auto camera = capture.Start(kDeviceIndex, kOutputIndex);
  auto lane_detector = [=]() {
    switch (options.version) {
      case ufld::Version::kV1:
        return ufld::v1::LaneDetector{
            std::get<ufld::v1::ModelType>(options.model_type)};
      default:
        throw std::invalid_argument("Invalid major version.");
    }
  }();
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

Arguments Arguments::Parse(int argc, char** argv) {
  Arguments arguments{};
  for (int32_t i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-mv") {  // Major version
      arguments.version = ParseVersion(argc, argv, i);
    }
    if (std::string(argv[i]) == "-mt") {  // Model type
      arguments.model_type = ParseModelType(argc, argv, i, arguments.version);
    }
  }
  return arguments;
}

ufld::Version Arguments::ParseVersion(int32_t argc, char** argv,
                                      int32_t index) {
  if (index + 1 < argc) {
    const auto value = std::stoi(argv[index + 1]);
    if (value != 1) {
      std::cerr << "Invalid major version: " << value << std::endl;
      std::exit(1);
    }
    return ufld::Version::kV1;
  } else {
    std::cerr << "-mv requires one argument." << std::endl;
    std::exit(1);
  }
}

std::variant<ufld::v1::ModelType> Arguments::ParseModelType(
    int32_t argc, char** argv, int32_t index, ufld::Version version) {
  if (index + 1 < argc) {
    switch (version) {
      case ufld::Version::kV1:
        try {
          return ufld::v1::ModelTypeFromString(argv[index + 1]);
        } catch (const std::invalid_argument& e) {
          std::cerr << "Invalid model type: " << argv[index + 1] << std::endl;
          std::exit(1);
        }
      default:
        std::cerr << "Invalid major version: " << static_cast<int32_t>(version)
                  << std::endl;
        std::exit(1);
    }
  } else {
    std::cerr << "-mt requires one argument." << std::endl;
    std::exit(1);
  }
}
