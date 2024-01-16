# ETS2 Lane Detection

![GitHub release (latest by date)](https://img.shields.io/github/v/release/aleksi-kangas/ets2-lane-detection)

> Lane detection for Euro Truck Simulator 2 using Ultra-Fast-Lane-Detection V1 & V2

![Preview](docs/preview.gif)

## Usage

Requires Windows 10/11 and optionally an NVIDIA GPU with support for TensorRT.
For instructions, refer to the [documentation](docs/USAGE.md) and [models](models/README.md).

## Architecture
- `capture`-library for capturing frames using desktop duplication API
- `ets2ld`-application library combining `capture` and `ufld`-libraries in a multithreading fashion
- `models`-directory for storing the UFLD `.onnx` model files
- `third-party`-directory for storing third-party (NVIDIA) runtime libraries
- `ufld`-library for lane detection inference using UFLD models and OnnxRuntime

## Building

Instructions for building the application can be found in the [documentation](docs/BUILD.md).
The build follows the usual CMake workflow.
