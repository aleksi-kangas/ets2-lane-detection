module;

#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

#include <xtensor/xarray.hpp>
#include <xtensor/xbuilder.hpp>

export module ufld:v2.config;

import :core;

namespace ufld::v2::config {
template <class C>
struct Config {
  std::uint32_t lane_count{};
  std::vector<std::uint32_t> row_lane_indices{};
  std::vector<std::uint32_t> column_lane_indices{};
  std::int32_t input_width{};
  std::int32_t input_height{};
  std::uint32_t row_anchor_cell_count{};
  std::vector<float> row_anchors{};
  std::uint32_t column_anchor_cell_count{};
  std::vector<float> column_anchors{};
  float crop_ratio{1.0f};

  [[nodiscard]] constexpr std::uint32_t RowAnchorCount() const;
  [[nodiscard]] constexpr std::uint32_t ColumnAnchorCount() const;

 private:
  Config(std::uint32_t lane_count, std::vector<std::uint32_t>&& row_lane_indices,
         std::vector<std::uint32_t>&& column_lane_indices, std::int32_t input_width, std::int32_t input_height,
         std::uint32_t row_anchor_cell_count, std::uint32_t column_anchor_cell_count, float crop_ratio)
      : lane_count{lane_count},
        row_lane_indices{std::move(row_lane_indices)},
        column_lane_indices{std::move(column_lane_indices)},
        input_width{input_width},
        input_height{input_height},
        row_anchor_cell_count{row_anchor_cell_count},
        column_anchor_cell_count{column_anchor_cell_count},
        crop_ratio{crop_ratio} {}
  friend C;
};

template <class C>
constexpr std::uint32_t Config<C>::RowAnchorCount() const {
  return static_cast<std::uint32_t>(row_anchors.size());
}

template <class C>
constexpr std::uint32_t Config<C>::ColumnAnchorCount() const {
  return static_cast<std::uint32_t>(column_anchors.size());
}

struct CULane18Config : Config<CULane18Config> {
  CULane18Config() : Config{4, {1, 2}, {0, 3}, 1600, 320, 200, 100, 0.6f} {
    const xt::xarray<float> r_anchors = xt::linspace(0.42f, 1.0f, 72);
    const xt::xarray<float> c_anchors = xt::linspace(0.0f, 1.0f, 81);
    row_anchors = std::vector(r_anchors.begin(), r_anchors.end());
    column_anchors = std::vector(c_anchors.begin(), c_anchors.end());
  }
};

struct CULane34Config : Config<CULane34Config> {
  CULane34Config() : Config{4, {1, 2}, {0, 3}, 1600, 320, 200, 100, 0.6f} {
    const xt::xarray<float> r_anchors = xt::linspace(0.42f, 1.0f, 72);
    const xt::xarray<float> c_anchors = xt::linspace(0.0f, 1.0f, 81);
    row_anchors = std::vector(r_anchors.begin(), r_anchors.end());
    column_anchors = std::vector(c_anchors.begin(), c_anchors.end());
  }
};

struct CurveLanes18Config : Config<CurveLanes18Config> {
  CurveLanes18Config() : Config{10, {3, 6}, {4, 5}, 1600, 800, 200, 100, 0.8f} {
    const xt::xarray<float> r_anchors = xt::linspace(0.4f, 1.0f, 72);
    const xt::xarray<float> c_anchors = xt::linspace(0.0f, 1.0f, 41);
    row_anchors = std::vector(r_anchors.begin(), r_anchors.end());
    column_anchors = std::vector(c_anchors.begin(), c_anchors.end());
  }
};

struct CurveLanes34Config : Config<CurveLanes34Config> {
  CurveLanes34Config() : Config{10, {3, 6}, {4, 5}, 1600, 800, 200, 100, 0.8f} {
    const xt::xarray<float> r_anchors = xt::linspace(0.4f, 1.0f, 72);
    const xt::xarray<float> c_anchors = xt::linspace(0.0f, 1.0f, 81);
    row_anchors = std::vector(r_anchors.begin(), r_anchors.end());
    column_anchors = std::vector(c_anchors.begin(), c_anchors.end());
  }
};

struct TuSimple18Config : Config<TuSimple18Config> {
  TuSimple18Config() : Config{4, {1, 2}, {0, 3}, 800, 320, 100, 100, 0.8f} {
    const xt::xarray<float> r_anchors = xt::linspace(160, 710, 56) / 720.0f;
    const xt::xarray<float> c_anchors = xt::linspace(0.0f, 1.0f, 41);
    row_anchors = std::vector(r_anchors.begin(), r_anchors.end());
    column_anchors = std::vector(c_anchors.begin(), c_anchors.end());
  }
};

struct TuSimple34Config : Config<TuSimple34Config> {
  TuSimple34Config() : Config{4, {1, 2}, {0, 3}, 800, 320, 100, 100, 0.8f} {
    const xt::xarray<float> r_anchors = xt::linspace(160, 710, 56) / 720.0f;
    const xt::xarray<float> c_anchors = xt::linspace(0.0f, 1.0f, 41);
    row_anchors = std::vector(r_anchors.begin(), r_anchors.end());
    column_anchors = std::vector(c_anchors.begin(), c_anchors.end());
  }
};

using config_t = std::variant<std::monostate, CULane18Config, CULane34Config, CurveLanes18Config, CurveLanes34Config,
                              TuSimple18Config, TuSimple34Config>;

constexpr std::uint32_t LaneCount(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const auto& c) -> std::uint32_t { return c.lane_count; },
    [=](const std::monostate&) -> std::uint32_t { return 0; }
    }, config);
  // clang-format on
}

constexpr std::vector<std::uint32_t> RowLaneIndices(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const auto& c) -> std::vector<std::uint32_t> { return c.row_lane_indices; },
    [=](const std::monostate&) -> std::vector<std::uint32_t> { return {}; }
    }, config);
  // clang-format on
}

constexpr std::vector<std::uint32_t> ColumnLaneIndices(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const auto& c) -> std::vector<std::uint32_t> { return c.column_lane_indices; },
    [=](const std::monostate&) -> std::vector<std::uint32_t> { return {}; }
    }, config);
  // clang-format on
}

constexpr std::int32_t InputWidth(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const auto& c) -> std::int32_t { return c.input_width; },
    [=](const std::monostate&) -> std::int32_t { return 0; }
    }, config);
  // clang-format on
}

constexpr std::int32_t InputHeight(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> std::int32_t { return 0; },
    [=](const auto& c) -> std::int32_t { return c.input_height; }
    }, config);
  // clang-format on
}

constexpr float RowAnchor(const config_t& config, std::uint32_t i) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> float { return 0.0f; },
    [=](const auto& c) -> float { return c.row_anchors[i]; }
    }, config);
  // clang-format on
}

constexpr float RowAnchorFirst(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> float { return 0.0f; },
    [=](const auto& c) -> float { return c.row_anchors.front(); }
    }, config);
  // clang-format on
}

constexpr float RowAnchorLast(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> float { return 0.0f; },
    [=](const auto& c) -> float { return c.row_anchors.back(); }
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

constexpr float ColumnAnchor(const config_t& config, std::uint32_t i) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> float { return 0.0f; },
    [=](const auto& c) -> float { return c.column_anchors[i]; }
  }, config);
  // clang-format on
}

constexpr float ColumnAnchorFirst(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> float { return 0.0f; },
    [=](const auto& c) -> float { return c.column_anchors.front(); }
    }, config);
  // clang-format on
}

constexpr float ColumnAnchorLast(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [=](const std::monostate&) -> float { return 0.0f; },
    [=](const auto& c) -> float { return c.column_anchors.back(); }
    }, config);
  // clang-format on
}

constexpr std::uint32_t ColumnAnchorCount(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [](const std::monostate&) -> std::uint32_t { return 0; },
    [](const auto& c) -> std::uint32_t { return static_cast<std::uint32_t>(c.column_anchors.size()); },
    }, config);
  // clang-format on
}

constexpr std::uint32_t ColumnAnchorCellCount(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [](const std::monostate&) -> std::uint32_t { return 0; },
    [](const auto& c) -> std::uint32_t { return c.column_anchor_cell_count; }
    }, config);
  // clang-format on
}

constexpr float CropRatio(const config_t& config) {
  // clang-format off
  return std::visit(Overloaded{
    [](const std::monostate&) -> float { return 0.0f; },
    [](const auto& c) -> float { return c.crop_ratio; }
    }, config);
  // clang-format on
}
}  // namespace ufld::v2::config
