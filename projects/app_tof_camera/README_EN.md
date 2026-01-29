## 1. Introduction
This application is developed based on the MaixCam series hardware platform. It integrates the TOF100 depth sensor with a camera module to achieve functions such as depth data acquisition, visual image fusion, and data visualization. The application can collect depth matrix data from the TOF100 in real-time and supports fusing this depth data with camera images for intuitive display on a screen. It is suitable for scenario verification and functional demonstration of short-range (4cm-75cm) depth perception and visual fusion.

## 2. Main Features
1.  **Depth Data Acquisition**: Collects depth matrix data at resolutions of 25×25, 50×50, or 100×100 via the TOF100 sensor, supporting an effective detection range of 4cm to 75cm.
2.  **Visual Fusion Processing**: Fuses images captured by the camera with TOF depth data to generate visual images containing depth information.
3.  **Multi-Resolution Adaptation**: Supports switching between different depth data resolutions (25×25/50×50/100×100) and automatically adapts the display output.
4.  **Real-Time Visualization**: Scales depth data or fused images to fit the display resolution for real-time viewing. Supports crosshair annotations for depth extrema (minimum, maximum, and center points).
5.  **Touch Interaction**: Enables basic interaction via the touchscreen, such as exiting the application if the device is not detected.

## 3. User Guide
### 3.1 Hardware Preparation
1.  Confirm the hardware platform is MaixCAM or MaixCAM2 (SPI ports are automatically adapted for different platforms).
2.  Connect the TOF100 sensor to the corresponding SPI interface (SPI4 for MaixCAM, SPI2 for MaixCAM2).
3.  Connect the camera module (defaults to 50×50 resolution, supports dynamic adjustment).
4.  Connect the display and touchscreen, ensuring hardware drivers are functioning correctly.

### 3.2 Application Startup
1.  Deploy the compiled application to the Maix hardware platform.
2.  Execute the application; no additional command-line parameters are required. The program will automatically initialize the hardware and start.
3.  Upon startup, the program automatically detects the TOF100 sensor. If not detected, the screen displays a "Devices Not Found!" message, and you can exit by touching the screen.

### 3.3 Operation
1.  **Default Mode**: Automatically enters depth data display mode upon startup, showing a real-time visualization of depth data collected by the TOF100.
2.  **Fusion Mode Switch**: When fusion mode is enabled, the program automatically captures camera images and displays them fused with depth data.
3.  **Resolution Switch**: The program supports dynamic switching of depth data resolution (25×25/50×50/100×100). After switching, it automatically re-initializes the depth mapping table and adapts the display.
4.  **Exit Application**: The application can be exited via a system signal (e.g., Ctrl+C) or touchscreen operation (when the device is not detected).

### 3.4 Exception Handling
1.  If hardware initialization fails (display/camera/touchscreen/TOF100), the program outputs an error log and exits.
2.  If depth matrix data is empty during operation, the program skips the current frame and continues running without interrupting the overall process.
3.  When adjusting the camera resolution, the program automatically resets and re-initializes the camera, skipping some frames briefly during this period.

## 4. Notes
1.  **Hardware Compatibility**: SPI ports differ between Maix platforms (MaixCAM/MaixCAM2). The program automatically adapts, requiring no manual modification, but ensure hardware wiring corresponds correctly.
2.  **Detection Range**: The effective detection range of the TOF100 is 4cm-75cm. Data outside this range may be invalid, and the program applies threshold filtering.
3.  **Resolution Limitation**: Visual fusion mode only supports depth data at 50×50 resolution; switching to other resolutions will automatically revert to 50×50.
4.  **Resource Release**: The program automatically releases hardware resources (display, camera, TOF100) upon exit to prevent resource leaks caused by forced power-off.
5.  **Performance Tips**: If the frame rate is too low during operation, reduce the depth data resolution or disable visual fusion mode.
6.  **Data Visualization Logic**: Depth data is converted to an RGB image using a Color Map (cmap), where different colors correspond to different depth values. In fusion mode, brightness information from the camera image is overlaid.
7.  **Display Adaptation**: The program automatically adjusts the image position based on the display aspect ratio to ensure the image is centered.
8.  **Debugging Reserves**: The program retains some debugging code (e.g., frame rate printing, coordinate printing, test image drawing), which can be enabled via compilation macros to facilitate functional debugging and customized development.
9.  **Expandability**: Parameters such as depth thresholds (OPNS_MIN_DIS_MM/OPNS_MAX_DIS_MM) and fusion weights can be modified to adapt to different application scenarios.

## 5. More Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_tof_camera)