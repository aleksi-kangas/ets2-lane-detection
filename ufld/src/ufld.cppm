module;

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <variant>

module ufld:impl;

import ufld;
import :v1;

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
