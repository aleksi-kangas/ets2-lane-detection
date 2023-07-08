module;

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <variant>

module ufld;

std::unique_ptr<ufld::ILaneDetector> ufld::MakeLaneDetector(
    const std::filesystem::path& directory, ufld::Version version,
    std::variant<ufld::v1::Variant> variant) {
  switch (version) {
    case ufld::Version::kV1:
      return std::make_unique<ufld::v1::LaneDetector>(
          directory, std::get<ufld::v1::Variant>(variant));
    default:
      throw std::runtime_error{"Unsupported version"};
  }
}
