# Models

All models are available
from [PINTO Model Zoo](https://github.com/PINTO0309/PINTO_model_zoo)
by [PINTO0309](https://github.com/PINTO0309).
Download the desired models, copy and rename the applicable `.onnx` files to
this directory.

#### Cache

Caching is enabled for models, which speeds up startup after the first run. The model graph optimization during the
first run may take multiple minutes, but subsequent runs should take roughly tens of seconds. The cache is located in
`models/cache`. Note that, whenever *OnnxRuntime* or *TensorRT* version is bumped, or the hardware changes, the cache
must be deleted manually for it to be regenerated.

## [Ultra-Fast-Lane-Detection V1](https://github.com/PINTO0309/PINTO_model_zoo/tree/main/140_Ultra-Fast-Lane-Detection)

### CULane

- `ufld_v1_culane_288x800.onnx`

### TuSimple

- `ufld_v1_tusimple_288x800.onnx`
