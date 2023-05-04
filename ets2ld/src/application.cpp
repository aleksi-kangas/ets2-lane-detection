#include "ets2ld/application.h"

#include <cassert>
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
  ui_.SetOnModelSettingsChanged([this]() { HandleModelSettingsChanged(); });

  while (true) {
    if (!ui_.BeginFrame())
      break;

    ui_.RenderSettings(lane_detection_active_, lane_detection_initializing_);

    if (lane_detection_active_) {
      if (lane_detection_result_available_) {
        LaneDetectionResult result{};
        {
          std::lock_guard<std::mutex> lock{lane_detection_mutex_};
          result = std::move(lane_detection_result_);
          lane_detection_result_available_ = false;
        }
        ui_.UpdatePreview(result.preview);
      }

      ui_.RenderPreview(lane_detection_initializing_);
    }

    ui_.EndFrame();
  }
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
  if (settings_.enable_lane_detection != lane_detection_active_) {
    lane_detection_active_ = settings_.enable_lane_detection;
    if (lane_detection_active_) {
      if (lane_detector_ == nullptr) {
        auto InitializeAndStartLaneDetector = [&](const Settings& settings) {
          lane_detection_initializing_ = true;
          try {
            lane_detector_ = utils::CreateLaneDetector(settings.model.directory,
                                                       settings.model.version,
                                                       settings.model.variant);
          } catch (const std::exception& e) {
            ui_.ShowErrorMessage(e.what());
            lane_detection_initializing_ = false;
            lane_detection_active_ = false;
            return;
          }
          lane_detection_initializing_ = false;
          lane_detection_active_ = true;
          lane_detection_thread_ = std::thread{
              &Application::LaneDetectionThread, this, settings.capture};
        };
        std::thread{InitializeAndStartLaneDetector, settings_}.detach();
      } else {
        lane_detection_thread_ = std::thread{&Application::LaneDetectionThread,
                                             this, settings_.capture};
      }
    } else {
      stop_lane_detection_signal_ = true;
      if (lane_detection_thread_.joinable())
        lane_detection_thread_.join();
    }
  }
}

void Application::HandleModelSettingsChanged() {
  assert(!lane_detection_active_);
  auto InitializeLaneDetector = [&](const ModelSettings& settings) {
    lane_detection_initializing_ = true;
    try {
      lane_detector_ = utils::CreateLaneDetector(
          settings.directory, settings.version, settings.variant);
    } catch (const std::exception& e) {
      ui_.ShowErrorMessage(e.what());
      lane_detection_initializing_ = false;
      return;
    }
    lane_detection_initializing_ = false;
  };
  std::thread{InitializeLaneDetector, settings_.model}.detach();
}

}  // namespace ets2ld
