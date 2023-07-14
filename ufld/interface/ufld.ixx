module;

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

#include <opencv2/opencv.hpp>

export module ufld;

export import ufld.ld;
export import ufld.math;

import :v1;

export namespace ufld {
/**
 *
 */
struct Settings {
  std::filesystem::path model_directory{};
  Version version{Version::kV1};
  std::variant<v1::Variant> variant{v1::Variant::kCULane};
};

/**
 *
 * @param settings
 * @return
 */
[[nodiscard]] std::unique_ptr<ufld::ILaneDetector> MakeLaneDetector(
    const Settings& settings);
}  // namespace ufld

module :private;

std::unique_ptr<ufld::ILaneDetector> ufld::MakeLaneDetector(
    const Settings& settings) {
  switch (settings.version) {
    case ufld::Version::kV1:
      if (!std::holds_alternative<ufld::v1::Variant>(settings.variant))
        throw std::invalid_argument{"Invalid variant"};
      return std::make_unique<ufld::v1::LaneDetector>(
          settings.model_directory,
          std::get<ufld::v1::Variant>(settings.variant));
    default:
      throw std::invalid_argument{"Unsupported version"};
  }
}
