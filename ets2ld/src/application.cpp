#include "ets2ld/application.h"

#include <utility>

#include "dx11/capture.h"
#include "ets2ld/utils.h"

namespace ets2ld {
Application::~Application() {
  stop_lane_detection_signal_ = true;
  if (lane_detection_thread_.joinable()) {
    lane_detection_thread_.join();
  }
}

void Application::Run() {
  ui_.SetOnLaneDetectionEnableChanged(
      [this]() { HandleLaneDetectionEnableChanged(); });

  while (true) {
    if (!ui_.BeginFrame())
      break;

    ui_.RenderSettings(lane_detection_initializing_, lane_detection_active_);

    if (lane_detection_active_ && lane_detection_result_available_) {
      LaneDetectionResult result{};
      {
        std::lock_guard<std::mutex> lock{lane_detection_mutex_};
        result = std::move(lane_detection_result_);
        lane_detection_result_available_ = false;
      }
      ui_.UpdatePreview(result.preview);
    }

    if (lane_detection_initializing_ || lane_detection_active_) {
      ui_.RenderPreview(lane_detection_initializing_);
    }

    ui_.EndFrame();
  }
}

void Application::InitializeAndStartLaneDetector(const Settings& settings) {
  lane_detection_initializing_ = true;
  try {
    lane_detector_ = utils::CreateLaneDetector(settings.model.directory,
                                               settings.model.version,
                                               settings.model.variant);
  } catch (const std::exception& e) {
    ui_.ShowErrorMessage(e.what());
    lane_detection_initializing_ = false;
    lane_detection_active_ = false;
    settings_.enable_lane_detection = false;
    return;
  }
  lane_detection_initializing_ = false;
  lane_detection_active_ = true;
  lane_detection_thread_ =
      std::thread{&Application::LaneDetectionThread, this, settings.capture};
}

void Application::LaneDetectionThread(CaptureSettings capture_settings) {
  dx11::Capture capture{};
  auto camera =
      capture.Start(0, 0, 1,
                    cv::Rect{capture_settings.x, capture_settings.y,
                             capture_settings.width, capture_settings.height});

  while (!stop_lane_detection_signal_) {
    const cv::Mat frame = camera->GetNewestFrame();

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

void Application::HandleLaneDetectionEnableChanged() {
  if (lane_detection_initializing_)
    return;
  if (settings_.enable_lane_detection == lane_detection_active_)
    return;

  if (lane_detection_active_) {
    stop_lane_detection_signal_ = true;
    if (lane_detection_thread_.joinable())
      lane_detection_thread_.join();
    lane_detection_active_ = false;
  }

  if (lane_detector_ == nullptr) {
    std::thread{&Application::InitializeAndStartLaneDetector, this, settings_}
        .detach();
  } else {
    if (AreModelSettingsDifferent(settings_.model, lane_detector_)) {
      std::thread{&Application::InitializeAndStartLaneDetector, this, settings_}
          .detach();
    } else {
      lane_detection_active_ = true;
      lane_detection_thread_ = std::thread{&Application::LaneDetectionThread,
                                           this, settings_.capture};
    }
  }
}

bool Application::AreModelSettingsDifferent(
    const ets2ld::ModelSettings& model_settings,
    const std::unique_ptr<ufld::ILaneDetector>& lane_detector) {
  if (model_settings.directory != lane_detector->ModelDirectory())
    return true;
  if (model_settings.version != lane_detector->ModelVersion())
    return true;
  if (model_settings.variant != lane_detector->ModelVariant())
    return true;
  return false;
}

}  // namespace ets2ld
