## 1. Introduction
This program is a **real-time line tracking application** developed based on the MaixCam hardware platform and the LVGL graphical interface. It utilizes LAB color space threshold filtering to detect lines in images. It supports user-defined LAB threshold parameters, real-time viewing of pixel color information, visualization of tracking results, and persistent saving of user configurations. Additionally, it reports detection data via a serial port protocol, making it suitable for the development and verification of basic line tracking functions in scenarios such as line-following robots and visual inspection systems.

## 2. Main Features
1.  **LAB Threshold Configuration**: Supports customizing the threshold ranges for L (Lightness), A (Red-Green), and B (Blue-Yellow) in the LAB color space. The default configuration is L(0-27), A(-128-127), B(-128-127), and parameters are saved automatically after modification.
2.  **Pixel Color Sampling**: Clicking anywhere on the screen captures the RGB and LAB values of the corresponding pixel and displays them on the interface.
3.  **Line Detection**: Performs line detection on the camera feed based on the set LAB thresholds to identify line coordinates, angles, and other information.
4.  **Visualization**:
    *   Detected lines are marked with green lines on the screen.
    *   Direction indicators (Left/Right) are displayed based on the line angle.
    *   Supports a "Binary Mode" to visually inspect the black-and-white image after threshold filtering.
5.  **Data Reporting**: Reports detected line coordinates and angles via a serial communication protocol, supporting interaction with external devices (such as MCUs or host computers).
6.  **Parameter Persistence**: Automatically saves user-configured LAB thresholds when the program exits and loads them on the next startup.

## 3. User Guide
### 3.1 Program Operation Flow
#### 3.1.1 First Launch
1.  After running the program, it automatically initializes and loads the default LAB thresholds (L: 0-27, A: -128-127, B: -128-127), and the interface displays the real-time camera feed.
2.  If the configuration file does not exist, the program automatically creates it and writes the default parameters. The log will prompt "use default lab config for user".

#### 3.1.2 Adjusting Thresholds
1.  Find the adjustment controls (such as sliders or input boxes) for Lmin/Lmax, Amin/Amax, and Bmin/Bmax on the interface.
2.  Adjust the corresponding values. The program updates the detection thresholds in real-time, and the log prints the current configuration (format: {Lmin, Lmax, Amin, Amax, Bmin, Bmax}).
3.  No manual saving is required after adjustment; the values are persisted automatically when the program exits.

#### 3.1.3 Pixel Color Sampling
1.  Click anywhere on the screen to capture the RGB and LAB values of that pixel.
2.  The "Color Box" on the interface sequentially displays the sampled color and its corresponding values, while the log prints the touch coordinates (touch (x, y)).

#### 3.1.4 Line Detection and Visualization
1.  The program performs real-time line detection by default. When a line is detected, it is displayed as a green line on the screen.
2.  A right direction indicator is shown if the line angle is > 0° and < 90°; otherwise, a left direction indicator is shown.
3.  Enabling "Binary Mode" (via the "Eye" button on the interface):
    *   Click "Eye Open" to switch the view to a binary image (pixels within the threshold are white, others are black).
    *   Click "Eye Close" to revert to the original camera feed.

#### 3.1.5 Exiting the Program
1.  Trigger the exit via the "Exit" button on the interface; the log will prompt "exit!".
2.  The program automatically saves the current LAB thresholds, and the log prints "save user's lab config" along with the saved parameters.

### 3.2 Custom Extension
1.  **Adjusting Detection Precision**: Modify the following parameters in `app_loop` to optimize detection performance:
    *   `x_stride/y_stride`: Detection step size (smaller values mean higher precision but higher performance cost).
    *   `area_threshold/pixels_threshold`: Area/pixel thresholds (filters out small noise lines).
    *   `roi`: Region of Interest (default is full screen; reducing the area reduces computation).
2.  **Communication Protocol Extension**: Modify parameters in `APP_CMD_REPORT_FIND_LINES` (Command Code 9) or `priv.ptl->report()` to adapt to the communication format of external devices.
3.  **Visualization Optimization**: Adjust the position (x/y) and scale (`transform_scale`) in `ui_show_left_or_right` to fit different screen sizes.

## 4. Serial Communication Protocol (UART Protocol)
When the program detects a line (or color block), it reports the relevant coordinate information via the serial port protocol for parsing and interaction by external devices.
### 4.1 Data Format Example
Example Report: `AA CA AC BB 14 00 00 00 E1 08 EE 00 37 00 15 01 F7 FF 4E 01 19 00 27 01 5A 00 A7 20`
### 4.2 Field Parsing
*   `08`: The command code for this message (Hex 0x08), indicating that line tracking/color block detection data is being reported.
*   `EE 00 37 00 15 01 F7 FF 4E 01 19 00 27 01 5A 00`: Coordinate values of 4 vertices, arranged sequentially. Each vertex consists of a 2-byte x-coordinate + 2-byte y-coordinate (Little Endian):
    *   `EE 00` (x) + `37 00` (y) → First vertex coordinates (238, 55).
    *   `15 01` (x) + `F7 FF` (y) → Second vertex coordinates (277, -9).
    *   `4E 01` (x) + `19 00` (y) → Third vertex coordinates (334, 25).
    *   `27 01` (x) + `5A 00` (y) → Fourth vertex coordinates (295, 90).

### 4.3 Parsing Notes
*   Coordinate values are in **Little Endian** format (Least Significant Byte first). When parsing, concatenate the high and low bytes before converting to decimal.
*   Negative coordinates are represented in two's complement (e.g., `F7 FF` converts to decimal -9).
*   Serial port parameters such as baud rate and parity must match those of the external device (refer to Maix platform serial port configuration for defaults).

## 5. Precautions
1.  **Threshold Reasonableness**:
    *   The valid range for L is 0-100, and for A/B it is -128-127. Exceeding these ranges may cause detection failure.
    *   When adjusting thresholds, it is recommended to first sample the LAB value of the target line and then adjust the range slightly around that value.
2.  **Hardware Performance**: Enabling Binary Mode or reducing the detection step size increases computational load, which may cause a drop in frame rate.
3.  **Touch Sampling Accuracy**: Inaccurate touchscreen calibration leads to deviations in sampled pixel coordinates. It is recommended to calibrate the touchscreen first.
4.  **Configuration File Issues**: If the configuration file is corrupted, the program loads default parameters. Recovery can be done by deleting the configuration file and restarting.
5.  **Resource Release**: The communication protocol object (`priv.ptl`) is released automatically on exit. Do not force-terminate the program to avoid memory leaks.
6.  **Serial Communication**:
    *   Ensure the serial port peripheral is not occupied by other programs, as this causes reporting failure.
    *   External devices must parse coordinates strictly in Little Endian mode to avoid value errors.
7.  **Log Troubleshooting**: Check logs for exceptions, focusing on:
    *   "protocol init failed!": Communication protocol initialization failed; check for sufficient memory.
    *   "camera read failed": Camera read failed; check hardware connections.
    *   Threshold print information: Verify it matches the interface configuration.

## 6. More Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_line_tracking)

[MaixCAM MaixPy Line Tracking Robot (/Car)](https://wiki.sipeed.com/maixpy/doc/en/projects/line_tracking_robot.html)