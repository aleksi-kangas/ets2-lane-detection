#include "ets2ld/application.h"

#include <utility>

#include "ets2ld/utils.h"

namespace ets2ld {

Application::Application(Settings settings) : settings_{std::move(settings)} {}

Application::~Application() {
  stop_lane_detection_ = true;
  if (lane_detection_thread_.joinable()) {
    lane_detection_thread_.join();
  }
  capture_.Stop();
}

void Application::Run() {
  camera_ = capture_.Start(0, 0, 1);

  while (true) {
    HandleChangeInLaneDetectionEnabled();

    if (!ui_.BeginFrame())
      break;

    ui_.RenderSettings(lane_detection_enabled_, lane_detection_initializing_);

    if (lane_detection_enabled_) {
      // TODO Update preview image by using LaneDetectionResult

      ui_.RenderPreview(lane_detection_initializing_);
    }

    ui_.EndFrame();
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

void Application::HandleChangeInLaneDetectionEnabled() {
  if (settings_.enable_lane_detection != lane_detection_enabled_) {
    lane_detection_enabled_ = settings_.enable_lane_detection;
    if (lane_detection_enabled_) {
      lane_detection_initializing_ = true;
      std::thread{[&] {
        lane_detector_ = utils::CreateLaneDetector(settings_.model.directory,
                                                   settings_.model.variant,
                                                   settings_.model.version);
        lane_detection_thread_ =
            std::thread{&Application::LaneDetectionThread, this};
        lane_detection_initializing_ = false;
      }}.detach();
    } else {
      stop_lane_detection_ = true;
      lane_detection_thread_.join();
      lane_detector_ = nullptr;
    }
  }
}

}  // namespace ets2ld
