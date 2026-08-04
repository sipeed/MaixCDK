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

Real-time camera inference, display heatmap on screen (exit with SIGINT):

```bash
./nn_yolo26_depth /tmp/yolo26n-depth.mud
```

## Model files

Put the model on the device first:

```bash
scp yolo26n-depth.mud yolo26n-depth_w8a8_mix.axmodel root@<device-ip>:/tmp/
```

Build method please visit [MaixCDK](https://github.com/sipeed/MaixCDK).
