# Usage

## Download Release

Download a release from [Releases](https://github.com/aleksi-kangas/ets2-lane-detection/releases) and extract anywhere.
It is recommended to obtain the latest release.

## Download Lane Detection Model

Download at least one lane detection model.
Follow the instructions in [models/README.md](../models/README.md).

## Download NVIDIA Runtime Libraries

GPU accelerated lane detection requires a few runtime libraries from NVIDIA.
Due to size limitations in GitHub, these runtime libraries are redistributed in a separate .zip file along with the main
application release.
An easy way to download the libraries is to execute the distributed .bat file, which downloads the .zip and extracts it
into `third-party/NVIDIA`:

```cmd
cd third-party
.\download_nvidia.bat
```

**To enable GPU accelerated lane detection, copy/move the `.dll` files from `third-party/NVIDIA` next to
the `ets2ld.exe` executable.**

## Run Application

Run the `ets2ld.exe` executable. Choose the desired model and capture settings.
Toggle 'Enable' to begin lane detection. Note that first loading of a model will build some cache files, which may take
a bit longer. Subsequent launches should be much faster.
