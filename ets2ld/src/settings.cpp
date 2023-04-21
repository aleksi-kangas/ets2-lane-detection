#include "ets2ld/settings.h"

namespace {
ufld::Version ParseModelVersion(int32_t argc, char** argv, int32_t index) {
  if (index + 1 < argc) {
    const auto value = std::stoi(argv[index + 1]);
    if (value != 1) {
      std::cerr << "Invalid model version: " << value << std::endl;
      std::exit(1);
    }
    return ufld::Version::kV1;
  } else {
    std::cerr << "-v requires one argument." << std::endl;
    std::exit(1);
  }
}

std::variant<ufld::v1::ModelType> ParseModelVariant(int32_t argc, char** argv,
                                                    int32_t index,
                                                    ufld::Version version) {
  if (index + 1 < argc) {
    switch (version) {
      case ufld::Version::kV1:
        try {
          return ufld::v1::ModelTypeFromString(argv[index + 1]);
        } catch (const std::invalid_argument& e) {
          std::cerr << "Invalid model variant: " << argv[index + 1]
                    << std::endl;
          std::exit(1);
        }
      default:
        std::cerr << "Invalid model version: " << static_cast<int32_t>(version)
                  << std::endl;
        std::exit(1);
    }
  } else {
    std::cerr << "-mv requires one argument." << std::endl;
    std::exit(1);
  }
}

std::filesystem::path ParseModelDirectory(int32_t argc, char** argv,
                                          int32_t index) {
  if (index + 1 < argc) {
    auto path = std::filesystem::path{argv[index + 1]};
    if (!std::filesystem::exists(path)) {
      std::cerr << "Model directory does not exist: " << path << std::endl;
      std::exit(1);
    }
    return path;
  } else {
    std::cerr << "-d requires one argument." << std::endl;
    std::exit(1);
  }
}

}  // namespace

namespace ets2ld {
Settings Settings::ParseArgs(int argc, char** argv) {
  Settings settings{};
  for (int32_t i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "-v") {  // Model version
      settings.model.version = ParseModelVersion(argc, argv, i);
    }
    if (std::string(argv[i]) == "-mv") {  // Model variant
      settings.model.variant =
          ParseModelVariant(argc, argv, i, settings.model.version);
    }
    if (std::string(argv[i]) == "-d") {  // 'models' directory
      settings.model.directory = ParseModelDirectory(argc, argv, i);
    }
  }
  return settings;
}

}  // namespace ets2ld
