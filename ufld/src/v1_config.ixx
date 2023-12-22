module;

#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

export module ufld:v1.config;

import :core;

namespace ufld::v1::config {
template <class C>
struct Config {
  std::uint32_t row_anchor_cell_count{};
  std::vector<std::uint32_t> row_anchors{};

  [[nodiscard]] constexpr std::uint32_t RowAnchorCount() const;

 private:
  constexpr Config(std::uint32_t row_anchor_cell_count, std::vector<std::uint32_t> row_anchors)
      : row_anchor_cell_count{row_anchor_cell_count}, row_anchors{std::move(row_anchors)} {}
  friend C;
};

struct CULaneConfig : Config<CULaneConfig> {
  constexpr CULaneConfig()
      : Config{200 + 1, {121, 131, 141, 150, 160, 170, 180, 189, 199, 209, 219, 228, 238, 248, 258, 267, 277, 287}} {}
};

struct TuSimpleConfig : Config<TuSimpleConfig> {
  constexpr TuSimpleConfig()
      : Config{100 + 1, {64,  68,  72,  76,  80,  84,  88,  92,  96,  100, 104, 108, 112, 116, 120, 124, 128, 132, 136,
                         140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 196, 200, 204, 208, 212,
                         216, 220, 224, 228, 232, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272, 276, 280, 284}} {}
};

using config_t = std::variant<std::monostate, CULaneConfig, TuSimpleConfig>;

template <class C>
constexpr std::uint32_t Config<C>::RowAnchorCount() const {
  return static_cast<std::uint32_t>(row_anchors.size());
}

constexpr std::uint32_t RowAnchor(const config_t& config, std::uint32_t i) {
  // clang-format off
  return std::visit(Overloaded{
    [](const std::monostate&) -> std::uint32_t { return 0; },
    [=](const auto& c) -> std::uint32_t { return c.row_anchors[i]; }
    }, config);
  // clang-format on
}

constexpr std::uint32_t RowAnchorCount(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [](const std::monostate&) -> std::uint32_t { return 0; },
    [](const auto& c) -> std::uint32_t { return static_cast<std::uint32_t>(c.row_anchors.size()); }
    }, config);
  // clang-format on
}

constexpr std::uint32_t RowAnchorCellCount(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [](const std::monostate&) -> std::uint32_t { return 0; },
    [](const auto& c) -> std::uint32_t { return c.row_anchor_cell_count; }
    }, config);
  // clang-format on
}
}  // namespace ufld::v1::config
