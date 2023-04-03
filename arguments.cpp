#include "arguments.h"

#include <string>

namespace ets2ld {
Arguments Arguments::Parse(int argc, char** argv) {
  Arguments arguments{};
  for (int32_t i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-mv") {  // Major version
      arguments.version = ParseVersion(argc, argv, i);
    }
    if (std::string(argv[i]) == "-mt") {  // Model type
      arguments.model_type = ParseModelType(argc, argv, i, arguments.version);
    }
    if (std::string(argv[i]) == "-md") {  // Model directory
      arguments.model_directory = ParseModelDirectory(argc, argv, i);
    }
  }
  return arguments;
}

ufld::Version Arguments::ParseVersion(int32_t argc, char** argv,
                                      int32_t index) {
  if (index + 1 < argc) {
    const auto value = std::stoi(argv[index + 1]);
    if (value != 1) {
      std::cerr << "Invalid model version: " << value << std::endl;
      std::exit(1);
    }
    return ufld::Version::kV1;
  } else {
    std::cerr << "-mv requires one argument." << std::endl;
    std::exit(1);
  }
}

std::variant<ufld::v1::ModelType> Arguments::ParseModelType(
    int32_t argc, char** argv, int32_t index, ufld::Version version) {
  if (index + 1 < argc) {
    switch (version) {
      case ufld::Version::kV1:
        try {
          return ufld::v1::ModelTypeFromString(argv[index + 1]);
        } catch (const std::invalid_argument& e) {
          std::cerr << "Invalid model type: " << argv[index + 1] << std::endl;
          std::exit(1);
        }
      default:
        std::cerr << "Invalid model version: " << static_cast<int32_t>(version)
                  << std::endl;
        std::exit(1);
    }
  } else {
    std::cerr << "-mt requires one argument." << std::endl;
    std::exit(1);
  }
}

std::filesystem::path Arguments::ParseModelDirectory(int32_t argc, char** argv,
                                                     int32_t index) {
  if (index + 1 < argc) {
    auto path = std::filesystem::path{argv[index + 1]};
    if (!std::filesystem::exists(path)) {
      std::cerr << "Model directory does not exist: " << path << std::endl;
      std::exit(1);
    }
    return path;
  } else {
    std::cerr << "-md requires one argument." << std::endl;
    std::exit(1);
  }
}

}  // namespace ets2ld
