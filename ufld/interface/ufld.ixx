module;

#include <filesystem>
#include <memory>
#include <variant>

export module ufld;

export import ufld.base;
export import ufld.v1;

export namespace ufld {
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
