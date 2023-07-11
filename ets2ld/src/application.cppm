module;

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <variant>

#include <opencv2/opencv.hpp>

module ets2ld.application;

import capture;
import ets2ld.utils;
import ufld;

ets2ld::Application::~Application() {
  stop_lane_detection_signal_ = true;
  if (lane_detection_thread_.joinable()) {
    lane_detection_thread_.join();
  }
}

void ets2ld::Application::Run() {
  ui_.SetOnLaneDetectionEnableChanged(std::bind(
      &Application::HandleLaneDetectionEnableChanged, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  while (true) {
    if (lane_detection_result_available_) {
      ufld::LaneDetectionResult result{};
      {
        std::lock_guard<std::mutex> lock{lane_detection_mutex_};
        result = std::move(lane_detection_result_);
        lane_detection_result_available_ = false;
      }
      ui_.UpdateStatistics(result.statistics);
      if (result.preview.has_value()) {
        ui_.UpdatePreview(result.preview.value());
      }
    }

    if (!ui_.BeginFrame())
      break;

    ui_.RenderSettings(lane_detection_initializing_, lane_detection_active_);
    ui_.RenderStatistics();
    if (lane_detection_initializing_ || lane_detection_active_) {
      ui_.RenderPreview(lane_detection_initializing_);
    }

    ui_.EndFrame();
  }
}

void ets2ld::Application::LaneDetectorInitializerThread(
    capture::Settings capture_settings, ufld::Settings ufld_settings) {
  lane_detection_initializing_ = true;
  try {
    lane_detector_ = ufld::MakeLaneDetector(ufld_settings);
    lane_detection_initializing_ = false;
    lane_detection_thread_ =
        std::thread{&Application::LaneDetectorThread, this, capture_settings};
  } catch (const std::exception& e) {
    ui_.ShowErrorMessage(e.what());
    lane_detection_initializing_ = false;
  }
}

void ets2ld::Application::LaneDetectorThread(
    capture::Settings capture_settings) {
  lane_detection_active_ = true;

  capture::CaptureManager capture_manager{};
  auto camera = capture_manager.Start(capture_settings);

  while (!stop_lane_detection_signal_) {
    const cv::Mat frame = camera->GetNewestFrame();
    ufld::LaneDetectionResult result =
        lane_detector_->Detect(frame, frame.clone());
    {
      std::lock_guard<std::mutex> lock{lane_detection_mutex_};
      lane_detection_result_ = std::move(result);
      lane_detection_result_available_ = true;
    }
  }
  lane_detection_active_ = false;
}

[[nodiscard]] bool AreModelSettingsEqual(
    const ufld::Settings& ufld_settings,
    const ufld::ILaneDetector* lane_detector) {
  if (ufld_settings.model_directory != lane_detector->ModelDirectory())
    return false;
  if (ufld_settings.version != lane_detector->ModelVersion())
    return false;
  if (ufld_settings.variant != lane_detector->ModelVariant())
    return false;
  return true;
}

void ets2ld::Application::HandleLaneDetectionEnableChanged(
    bool enable, capture::Settings capture_settings,
    ufld::Settings ufld_settings) {
  if (lane_detection_initializing_)
    throw std::logic_error{
        "Lane detection enable changed while lane detection is "
        "initializing."};
  capture_settings_ = capture_settings;
  ufld_settings_ = ufld_settings;
  if (enable) {
    if (lane_detector_ != nullptr &&
        AreModelSettingsEqual(ufld_settings_, lane_detector_.get())) {
      lane_detection_thread_ = std::thread{&Application::LaneDetectorThread,
                                           this, capture_settings_};
    } else {
      std::thread{&Application::LaneDetectorInitializerThread, this,
                  capture_settings_, ufld_settings_}
          .detach();
    }
  } else {
    stop_lane_detection_signal_ = true;
    if (lane_detection_thread_.joinable())
      lane_detection_thread_.join();
    stop_lane_detection_signal_ = false;
  }
}
