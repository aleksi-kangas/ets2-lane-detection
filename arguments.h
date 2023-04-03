#pragma once

#include <cstdint>
#include <filesystem>
#include <variant>

#include "ufld/v1.h"

namespace ets2ld {
/**
 * The following command line arguments are supported:
 * -mv: The version of UFLD model to use.
 *      Valid values are: 1
 *      Default is: 1
 * -mt: The model type to use, depends on -mv.
 *      Valid values are: V1 -> [CULane, TuSimple]
 *      Default is: V1 -> CULane
 * -md: Model directory
 *      Default is: ./models
 */
struct Arguments {
  ufld::Version version{ufld::Version::kV1};
  std::variant<ufld::v1::ModelType> model_type{ufld::v1::ModelType::kCULane};
  std::filesystem::path model_directory{"./models"};

  static Arguments Parse(int argc, char** argv);

 private:
  static ufld::Version ParseVersion(int32_t argc, char** argv, int32_t index);
  static std::variant<ufld::v1::ModelType> ParseModelType(
      int32_t argc, char** argv, int32_t index, ufld::Version version);
  static std::filesystem::path ParseModelDirectory(int32_t argc, char** argv,
                                                   int32_t index);
};

}  // namespace ets2ld
