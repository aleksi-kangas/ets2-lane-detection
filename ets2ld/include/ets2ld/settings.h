#pragma once

#include <filesystem>
#include <variant>

#include "ets2ld/utils.h"
#include "ufld/ufld.h"

namespace ets2ld {

struct CaptureSettings {
  int32_t x{0};
  int32_t y{0};
  int32_t width{utils::QueryPrimaryMonitorResolution().first};
  int32_t height{utils::QueryPrimaryMonitorResolution().second};
};

struct ModelSettings {
  std::filesystem::path directory{".\\models"};
  std::variant<ufld::v1::ModelVariant> variant{ufld::v1::ModelVariant::kCULane};
  ufld::Version version{ufld::Version::kV1};
};

struct Settings {
  bool enable_lane_detection{false};
  CaptureSettings capture{};
  ModelSettings model{};
};

}  // namespace ets2ld
