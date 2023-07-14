module;

#include <cstdint>
#include <utility>
#include <vector>

module ufld:v1_config;

namespace ufld::v1 {
struct IConfig {
 public:
  // Number of cells in a single row anchor, i.e. the number of columns in the grid
  uint32_t row_anchor_cell_count;
  // Number of row anchors (= classes) per lane
  std::uint32_t row_anchor_count;
  // Each lane consist of row anchors, i.e. rows of cells
  std::vector<uint32_t> row_anchors;

 protected:
  IConfig(uint32_t row_anchor_cell_count, std::vector<uint32_t> row_anchors);
};

IConfig::IConfig(uint32_t row_anchor_cell_count,
                 std::vector<uint32_t> row_anchors)
    : row_anchor_cell_count{row_anchor_cell_count},
      row_anchor_count{static_cast<uint32_t>(row_anchors.size())},
      row_anchors{std::move(row_anchors)} {}

struct CULaneConfig : public IConfig {
  CULaneConfig()
      : IConfig{200 + 1,
                {121, 131, 141, 150, 160, 170, 180, 189, 199, 209, 219, 228,
                 238, 248, 258, 267, 277, 287}} {}
};

struct TuSimpleConfig : public IConfig {
  TuSimpleConfig()
      : IConfig{100 + 1,
                {64,  68,  72,  76,  80,  84,  88,  92,  96,  100, 104, 108,
                 112, 116, 120, 124, 128, 132, 136, 140, 144, 148, 152, 156,
                 160, 164, 168, 172, 176, 180, 184, 188, 192, 196, 200, 204,
                 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
                 256, 260, 264, 268, 272, 276, 280, 284}} {}
};

}  // namespace ufld::v1
