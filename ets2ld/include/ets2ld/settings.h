#pragma once

#include <filesystem>
#include <variant>

#include "ufld/ufld.h"
#include "ufld/v1.h"

namespace ets2ld {

struct CaptureSettings {
  // TODO
};

struct ModelSettings {
  std::filesystem::path directory{".\\models"};
  std::variant<ufld::v1::ModelType> variant{ufld::v1::ModelType::kCULane};
  ufld::Version version{ufld::Version::kV1};
};

struct Settings {
  bool enable_lane_detection{false};
  CaptureSettings capture{};
  ModelSettings model{};
};

}  // namespace ets2ld
