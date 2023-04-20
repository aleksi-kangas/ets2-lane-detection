#include "ets2ld/application.h"

#include <stdexcept>
#include <utility>

namespace ets2ld {

Application::Application(Arguments arguments) {
  switch (arguments.version) {
    case ufld::Version::kV1:
      lane_detector_ = std::make_unique<ufld::v1::LaneDetector>(
          arguments.model_directory,
          std::get<ufld::v1::ModelType>(arguments.model_type));
      break;
    default:
      throw std::invalid_argument{"Invalid model version."};
  }
}

Application::~Application() {
  stop_lane_detection_ = true;
  lane_detection_thread_.join();
  capture_.Stop();
}

void Application::Run() {
  camera_ = capture_.Start(0, 0, 1);
  lane_detection_thread_ = std::thread{&Application::LaneDetectionThread, this};

  // UI loop
  while (true) {
    if (!ui_.PollEvents()) {
      break;
    }

    if (lane_detection_result_available_) {
      LaneDetectionResult result{};
      {
        std::lock_guard<std::mutex> lock{lane_detection_mutex_};
        result = std::move(lane_detection_result_);
        lane_detection_result_available_ = false;
      }

      cv::Mat preview{};
      cv::resize(result.preview, preview, {1280, 720});
      cv::imshow("Preview", preview);
      if (cv::waitKey(1) == 27) {  // ESC
        break;
      }
    }

    ui_.Render();
  }
}

void Application::LaneDetectionThread() {
  while (!stop_lane_detection_) {
    // TODO What if camera is not valid due to region change etc?
    const cv::Mat frame = camera_->GetNewestFrame();

    LaneDetectionResult result{};
    result.frame = frame;
    result.lanes = lane_detector_->Detect(frame);
    result.preview = ufld::VisualizeLanes(result.lanes, result.frame);
    {
      std::lock_guard<std::mutex> lock{lane_detection_mutex_};
      lane_detection_result_ = std::move(result);
      lane_detection_result_available_ = true;
    }
  }
}

}  // namespace ets2ld
