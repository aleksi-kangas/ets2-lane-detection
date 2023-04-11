# Build

## Requirements

- Windows
- [MSVC](https://visualstudio.microsoft.com/downloads/)
- [vcpkg](https://vcpkg.io/en/)
- [CUDA](https://developer.nvidia.com/cuda-downloads)
- [TensorRT](https://developer.nvidia.com/tensorrt)

Dependencies are managed by *vcpkg* and are specified in `vcpkg.json`.

### CMake Configuration

Unfortunately, at the time of writing, the `onnxruntime-gpu` library from
*vcpkg* does not support CMake's `find_package` command yet (see
e.g. [issue](https://github.com/microsoft/onnxruntime/issues/7150)).
I've included custom `onnxruntimeConfig.cmake` and `onnxruntimeVersion.cmake` to
achieve such functionality. The files are automatically copied
to `<cmake-build>/vcpkg_installed/x64-windows/share/onnxruntime-gpu/`
at configuration time. Shall there be any issues, the files are found in this
directory.

### Models

One needs to download at least one lane detection model.
Please refer to the [README](../models/README.md) in `models` directory for
details.

## Building

The provided `CMakePresets.json` contains presets for building (x64) in `debug`
and `release` modes.
The presets have been tested with *JetBrains CLion*.

### OnnxRuntime Providers

The `onnxruntime` library supports multiple providers for execution.
The build system should automatically copy the necessary DLLs next to the
executable.
The following DLLs shall be available:

- `onnxruntime_providers_cuda.dll`
- `onnxruntime_providers_shared.dll`
- `onnxruntime_providers_tensorrt.dll`
