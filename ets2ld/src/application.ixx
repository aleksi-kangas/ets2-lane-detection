module;

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <variant>

export module ets2ld:application;

import :ui;
import :utils;

import capture;
import ufld;

namespace ets2ld {
export class Application {
 public:
  enum class State { kIdle, kInitializing, kActive };

  Application() = default;

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  Application(Application&&) = delete;
  Application& operator=(Application&&) = delete;

  void Run();

 private:
  std::atomic<State> state_{State::kIdle};
  UI ui_{};

  std::optional<std::future<ufld::lane_detector_t>> detector_initialization_future_{std::nullopt};
  ufld::lane_detector_t lane_detector_{};
  std::jthread detection_thread_{};
  std::atomic_flag detection_result_available_{};
  std::mutex detection_mutex_{};
  ufld::LaneDetectionResult detection_result_{};

  void DetectorInitialization();

  void ObtainAndUpdateResult();

  void StartLaneDetection();

  void DetectionLoop(std::stop_token st, capture::Settings&& capture_settings);

  void HandleLaneDetectionEnableChanged(bool enable);

  [[nodiscard]] static constexpr UI::State MapState(State state);
};

}  // namespace ets2ld

void ets2ld::Application::Run() {
  ui_.RegisterOnLaneDetectionEnableChange(std::bind_front(&Application::HandleLaneDetectionEnableChanged, this));
  while (true) {
    DetectorInitialization();
    ObtainAndUpdateResult();

    if (!ui_.BeginFrame())
      break;
    {
      ui_.RenderSettings(MapState(state_));
      ui_.RenderStatistics();
      ui_.RenderPreview(MapState(state_));
    }
    ui_.EndFrame();
  }
}

void ets2ld::Application::DetectorInitialization() {
  if (!detector_initialization_future_.has_value())
    return;
  state_ = State::kInitializing;
  switch (const auto status = detector_initialization_future_->wait_for(std::chrono::milliseconds{0}); status) {
    case std::future_status::ready: {
      try {
        lane_detector_ = detector_initialization_future_->get();
        StartLaneDetection();
      } catch (const std::exception& e) {
        ui_.ShowErrorMessage(e.what());
        state_ = State::kIdle;
      }
      detector_initialization_future_.reset();
    } break;
    default:
      break;  // Do nothing
  }
}

void ets2ld::Application::ObtainAndUpdateResult() {
  if (!detection_result_available_.test())
    return;
  ufld::LaneDetectionResult result{};
  {
    std::scoped_lock lock{detection_mutex_};
    result = std::move(detection_result_);
  }
  detection_result_available_.clear();
  ui_.UpdatePreview(result.preview);
  ui_.UpdateStatistics(result.statistics);
}

void ets2ld::Application::StartLaneDetection() {
  if (std::holds_alternative<std::monostate>(lane_detector_))
    throw std::logic_error{"Lane detector has not been initialized"};
  detection_thread_ = std::jthread{std::bind_front(&Application::DetectionLoop, this), ui_.GetCaptureSettings()};
}

void ets2ld::Application::DetectionLoop(std::stop_token st, capture::Settings&& capture_settings) {
  state_ = State::kActive;
  capture::Timer timer{capture_settings.target_fps};
  auto camera = capture::Camera{std::move(capture_settings)};
  while (!st.stop_requested()) {
    timer.Wait();
    auto frame = camera.CaptureFrame(capture_settings.region);
    if (!frame.has_value())
      continue;
    // clang-format off
    std::visit(ufld::Overloaded{
      [](std::monostate&) {},
      [&](auto& lane_detector){
        auto result = lane_detector.Detect(std::move(frame.value()), true);
        {
          std::scoped_lock lock{detection_mutex_};
          detection_result_ = std::move(result);
        }
        detection_result_available_.test_and_set();
      }},lane_detector_);
    // clang-format on
  }
}

void ets2ld::Application::HandleLaneDetectionEnableChanged(bool enable) {
  if (state_ == State::kInitializing)
    throw std::logic_error{"Lane detection enable changed while lane detection is initializing."};
  if (enable) {
    ufld::Settings ufld_settings = ui_.GetUFLDSettings();
    if (ufld_settings == lane_detector_) {
      StartLaneDetection();
    } else {
      detector_initialization_future_ = std::async(
          std::launch::async, [](ufld::Settings&& settings) { return ufld::MakeLaneDetector(std::move(settings)); },
          std::move(ufld_settings));
    }
  } else {
    detection_thread_.get_stop_source().request_stop();
    if (detection_thread_.joinable()) {
      detection_thread_.join();
    }
    state_ = State::kIdle;
  }
}

constexpr ets2ld::UI::State ets2ld::Application::MapState(State state) {
  switch (state) {
    case State::kIdle:
      return UI::State::kIdle;
    case State::kInitializing:
      return UI::State::kInitializing;
    case State::kActive:
      return UI::State::kActive;
    default:
      throw std::logic_error{"Unknown state"};
  }
}
