# Build

## Requirements

- Windows
- [CMake 3.26](https://cmake.org/download/)
- [MSVC 17.6.5+ with C++ modules support](https://visualstudio.microsoft.com/downloads/)
- [CUDA 11.8](https://developer.nvidia.com/cuda-downloads)
- [TensorRT 8.5](https://developer.nvidia.com/tensorrt)

Dependencies are managed by *vcpkg* (as a Git submodule) and are specified
in `vcpkg.json`.

### Models

One needs to download at least one lane detection model.
Please refer to the [README](../models/README.md) in `models` directory for
details.

#### Cache

Caching is enabled for models, which speeds up startup after the first run. The
model graph optimization during the
first run may take multiple minutes, but subsequent runs should take roughly
tens of seconds. The cache is located in
`models/cache`. Note that, whenever *OnnxRuntime* or *TensorRT* version is
bumped, or the hardware changes, the cache
must be deleted manually for it to be regenerated.

## CMake Configuration

Unfortunately, at the time of writing, the `onnxruntime-gpu` library from
*vcpkg* does not support CMake's `find_package` command yet (see
e.g. [issue](https://github.com/microsoft/onnxruntime/issues/7150)).
I've included custom `onnxruntimeConfig.cmake` and `onnxruntimeVersion.cmake` to
achieve such functionality. The files are automatically copied
to `<cmake-build>/vcpkg_installed/x64-windows/share/onnxruntime-gpu/`
at configuration time. Shall there be any issues, the files are found in this
directory.

## Building

As dependencies are managed via [vcpkg](https://vcpkg.io/en/), the first step is
to initialize the Git submodule:

```powershell
# Install vcpkg
git submodule update --init
.\vcpkg\bootstrap-vcpkg.bat
```

The provided `CMakePresets.json` contains presets for building (x64) in `debug`
and `release` modes. The presets have been tested with *JetBrains CLion*.

Building from command line follows the usual CMake workflow:

```powershell
# For the environment variables related to MSVC to be present, open 'Developer Command Prompt for VS 2022' (or run the vcvarsall.bat script).

# Navigate to the repository root
cd <repository_root>
# Generate the build files
cmake --preset x64-release -S . -B .\cmake-build-x64-release
```

```powershell
# Build the project
cmake --build .\cmake-build-x64-release --config Release
```

```cmd
# Run the executable
.\cmake-build-x64-release\ETS2_Lane_Detection.exe
```

### OnnxRuntime Providers

The `onnxruntime` library supports multiple providers for execution.
The build system should automatically copy the necessary DLLs next to the
executable.
The following DLLs shall be available:

- `onnxruntime_providers_cuda.dll`
- `onnxruntime_providers_shared.dll`
- `onnxruntime_providers_tensorrt.dll`
