# YOLO26-depth example

YOLO26-depth (monocular depth estimation) example for MaixCDK, based on the header-only class `maix_nn_yolo26_depth.hpp`.

## Build

```bash
cd examples/nn_yolo26_depth
export CMAKE_POLICY_VERSION_MINIMUM=3.5   # required on CMake >= 4.x, see FAQ
maixcdk build -p maixcam2
```

## Usage

Single image inference, save heatmap:

```bash
./nn_yolo26_depth /tmp/yolo26n-depth.mud /tmp/bus.jpg /tmp/bus_heatmap.jpg
```

Real-time camera inference with an interactive depth heatmap (exit with SIGINT):

```bash
./nn_yolo26_depth /tmp/yolo26n-depth.mud
```

### Touch distance probes

- Tap inside the displayed heatmap to add a distance probe.
- A probe stays at the selected image position and its value updates on every new frame.
- Up to five probes are kept. Adding a sixth removes the oldest probe.
- Tap within 20 screen pixels of an existing probe to remove it.
- Taps in black letterbox padding are ignored.
- Invalid model samples are displayed as `N/A`.

Distances are monocular depth estimates in meters. The released YOLO26-depth weights include global metric-scale calibration, but the values are not a replacement for a calibrated distance sensor.

The example runs one NPU inference per processed frame. Heatmap generation and all active probe values reuse that same depth tensor on the CPU, so adding probes does not add NPU inference calls.

The reusable C++ flow is:

```cpp
nn::YOLO26Depth model("/tmp/yolo26n-depth.mud");
tensor::Tensor *depth = model.get_depth(frame, image::FIT_CONTAIN); // NPU once
image::Image *heatmap = model.depth_to_image(
    *depth, frame.width(), frame.height(), image::FIT_CONTAIN, image::CMap::JET);
float meters = model.get_distance(
    *depth, x, y, frame.width(), frame.height(), image::FIT_CONTAIN); // CPU only
delete heatmap;
delete depth;
```

## Model files

Put the model on the device first:

```bash
scp yolo26n-depth.mud yolo26n-depth_w8a8_mix.axmodel root@<device-ip>:/tmp/
```

Build method please visit [MaixCDK](https://github.com/sipeed/MaixCDK).
