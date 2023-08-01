module;

#include <mutex>

export module ets2ld;

import :application;

export namespace ets2ld {
/**
 * Run the ETS2 Lane Detection -application.
 */
void RunApplication() {
  static std::mutex run_mutex{};
  std::lock_guard<std::mutex> lock{run_mutex};
  ets2ld::Application ets2_lane_detection{};
  ets2_lane_detection.Run();
}
}  // namespace ets2ld
