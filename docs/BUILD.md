# Build

## Requirements

- Windows
- [MSVC](https://visualstudio.microsoft.com/downloads/)
- [vcpkg](https://vcpkg.io/en/)
- [CUDA](https://developer.nvidia.com/cuda-downloads)
- [TensorRT](https://developer.nvidia.com/tensorrt)

Dependencies are managed by *vcpkg* and are specified in `vcpkg.json`.

### CMake Configuration

Copy the `onnxruntimeConfig.cmake` and `onnxruntimeVersion.cmake` to the
directory `<cmake-build>/vcpkg_installed/x64-windows/share/onnxruntime-gpu/`.
These files enable the use of `find_package`, which is not yet supported
properly in `onnxruntime` available from *vcpkg*. Credits to *jcarius*.

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
Copy the following DLLs to the build directory next to the executable (`<cmake-build>`):
- `<cmake-build>/vcpkg_installed/x64-windows/bin/onnxruntime_providers_cuda.dll`
- `<cmake-build>/vcpkg_installed/x64-windows/bin/onnxruntime_providers_shared.dll`
- `<cmake-build>/vcpkg_installed/x64-windows/bin/onnxruntime_providers_tensorrt.dll`
