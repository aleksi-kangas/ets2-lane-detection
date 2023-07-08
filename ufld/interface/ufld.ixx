module;

#include <filesystem>
#include <memory>
#include <variant>

export module ufld;

export import ufld.base;
export import ufld.v1;

export namespace ufld {
/**
 *
 * @param directory
 * @param version
 * @param variant
 * @return
 */
[[nodiscard]] std::unique_ptr<ufld::ILaneDetector> MakeLaneDetector(
    const std::filesystem::path& directory, ufld::Version version,
    std::variant<ufld::v1::Variant> variant);
}  // namespace ufld
