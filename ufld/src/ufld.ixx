module;

#include <stdexcept>
#include <utility>
#include <variant>

export module ufld;

export import :core;
export import :draw;
export import :math;
export import :v1;
export import :v1.config;
export import :v2;
export import :v2.config;

namespace ufld {
export using lane_detector_t = std::variant<std::monostate, v1::LaneDetector, v2::LaneDetector>;

export lane_detector_t MakeLaneDetector(const Settings& settings) {
  switch (settings.model_version) {
    case Version::kV1: {
      if (!std::holds_alternative<v1::Variant>(settings.model_variant))
        throw std::invalid_argument{"Invalid model variant"};
      return lane_detector_t{std::in_place_type<v1::LaneDetector>, settings.model_directory,
                             std::get<v1::Variant>(settings.model_variant)};
    }
    case Version::kV2: {
      if (!std::holds_alternative<v2::Variant>(settings.model_variant))
        throw std::invalid_argument{"Invalid model variant"};
      return lane_detector_t{std::in_place_type<v2::LaneDetector>, settings.model_directory,
                             std::get<v2::Variant>(settings.model_variant)};
    }
    default:
      throw std::invalid_argument{"Invalid model version"};
  }
}

export bool operator==(const Settings& settings, const lane_detector_t& lane_detector) {
  if (std::holds_alternative<v1::LaneDetector>(lane_detector)) {
    if (settings.model_directory != std::get<v1::LaneDetector>(lane_detector).ModelDirectory())
      return false;
    if (settings.model_version != std::get<v1::LaneDetector>(lane_detector).ModelVersion())
      return false;
    if (settings.model_variant != std::get<v1::LaneDetector>(lane_detector).ModelVariant())
      return false;
    return true;
  }
  if (std::holds_alternative<v2::LaneDetector>(lane_detector)) {
    if (settings.model_directory != std::get<v2::LaneDetector>(lane_detector).ModelDirectory())
      return false;
    if (settings.model_version != std::get<v2::LaneDetector>(lane_detector).ModelVersion())
      return false;
    if (settings.model_variant != std::get<v2::LaneDetector>(lane_detector).ModelVariant())
      return false;
    return true;
  }
  return false;
}
}  // namespace ufld
