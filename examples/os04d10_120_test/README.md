# OS04D10 120 FPS test

This MaixCAM2-only test requests the OS04D10 native 640x360@120 sensor mode by
default. It does not open the display by default. It counts successful camera
reads and prints AXERA VIN device, pipe, and channel frame rates and loss
counters. It also reports the actual AE shutter, gain, FPS and measured image
luma once per second. AI-ISP is disabled; the regular ISP remains enabled
because OS04D10 outputs RAW10 and RGB/YUV frames need the regular ISP pipeline.

An explicit `fps=120` request selects the native 640x360@120 sensor mode for any
requested output size. A 640x360 output preserves the full 16:9 sensor frame.
Other aspect ratios, such as 640x480, use the existing `fit=2` center-crop and
scale path, so they keep the requested output dimensions but lose some field of
view and detail. For output sizes up to 1280x720, automatic FPS (`fps=-1`)
remains 60 FPS and does not select the 120 FPS mode, including for a 640x360
output. Larger automatic-FPS requests retain the existing 1440p30 selection.

Build:

    maixcdk build -p maixcam2 --toolchain-id default

The complete positional interface is:

    ./os04d10_120_test [seconds [format [display [exposure_us [again_u22_10 [width [height [fps]]]]]]]]

Run NV21 for the default 10 seconds, or pass a duration in seconds:

    ./os04d10_120_test
    ./os04d10_120_test 30

Pass `rgb` as the second argument to request RGB888 frames:

    ./os04d10_120_test 30 rgb

Pass `display` as the third argument to show every frame and measure the display
call overhead and resulting loop rate:

    ./os04d10_120_test 30 rgb display
    ./os04d10_120_test 30 nv21 display

The 120 FPS mode constrains automatic exposure to 361-8000 us, uses shutter
priority, and limits system gain to 16x so AE does not oscillate between its
minimum and the 720p60 tuning file's original 2048x limit. Automatic exposure
is enabled by default. The optional fourth and fifth arguments switch to manual
exposure in microseconds and analog gain in U22.10 units (`1024` is 1x). Keep
exposure below one 120 FPS frame period:

    ./os04d10_120_test 10 nv21 display 7500 1024

Pass `0 0`, or omit both values, to use automatic exposure:

    ./os04d10_120_test 10 nv21 display 0 0

Optional sixth, seventh, and eighth arguments set the requested output width,
height, and FPS. Specify the earlier optional arguments as placeholders when
setting them. Use `headless` as the third argument to keep the display closed:

    # Full 16:9 frame at an explicit 120 FPS
    ./os04d10_120_test 10 nv21 headless 0 0 640 360 120

    # 4:3 output through the center-crop/scale path at an explicit 120 FPS
    ./os04d10_120_test 10 nv21 headless 0 0 640 480 120

    # Automatic FPS remains 60 FPS for both output sizes
    ./os04d10_120_test 10 nv21 headless 0 0 640 360 -1
    ./os04d10_120_test 10 nv21 headless 0 0 640 480 -1

On the tested MaixCAM2, `Display::show()` accepts about 120 input frames per
second without blocking the camera loop to the panel refresh rate. The built-in
panel runs at about 59.27 Hz, so VO replaces intermediate frames and physically
outputs about 59 frames per second even though the application loop stays near
120 FPS.

## Known limitations

The closed-source OS04D10 sensor object does not expose a public 640x360@120
entry, so this mode applies a verified register override after the vendor's
normal initialization. It also reuses the official 720p60 ISP tuning file.
Consequently, image quality can be lower than 720p60 and artificial lighting can
show visible flicker. Manual exposure can reduce the effect for a specific light
source, but it is not a general anti-flicker calibration for this sensor mode.
