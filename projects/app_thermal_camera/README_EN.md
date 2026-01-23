## 1. Overview
This program is designed for devices equipped with MaixCam/Pro/2 hardware platforms. It primarily implements driver support and data visualization for the PMOD_Thermal32 infrared thermal imaging sensor, supporting fused display of thermal imaging and camera footage, and real-time display of temperature data (including extreme temperature annotations). It serves as an out-of-the-box solution for thermal imaging data collection and visualization.

## 2. Key Features
1. **Automatic Device Detection**: Upon startup, the program automatically scans the I2C bus (I2C5 for MAIXCAM, I2C7 for MAIXCAM2) to detect the connection of the PMOD_Thermal32 thermal imaging sensor;
2. **Thermal Imaging Visualization**: Converts the 32×24 pixel temperature matrix collected by the PMOD_Thermal32 into a visual image, scaled to fit the display resolution for presentation;
3. **Multi-Mode Display**: Supports switching between pure thermal imaging mode and thermal imaging + camera fusion mode;
4. **Temperature Annotation**: Real-time display of the maximum temperature, minimum temperature, and center position temperature in the frame, with corresponding positions marked by crosshairs;
5. **Interactive Support**: Adapted for touchscreen operation, supporting basic interactions such as program exit and mode switching;
6. **Hardware Adaptability**: Compatible with MAIXCAM/MAIXCAM2 hardware platforms, automatically adapting I2C pin configurations and driver loading logic.

## 3. Usage Instructions
### 3.1 Hardware Preparation
1. Connect the PMOD_Thermal32 sensor to the designated I2C bus of the device via the corresponding interface (I2C5 for MAIXCAM, I2C7 for MAIXCAM2);
2. Ensure the device's display, touchscreen, and camera (if fusion mode is required) are properly installed and wired.

### 3.2 Running the Program
3. Upon startup, the program automatically detects the PMOD_Thermal32 device:
   - Device detected: Automatically enters the thermal imaging display interface, with real-time refresh of the thermal imaging frame;
   - Device not detected: The screen displays the prompt "Devices Not Found!", and the program can be exited by tapping any position on the touchscreen.

### 3.3 Operation Instructions
1. **Mode Switching**: Supports switching between pure thermal imaging mode and thermal imaging + camera fusion mode (the specific switching method depends on the device's UI interaction logic, e.g., tapping a designated area on the touchscreen);
2. **Exiting the Program**: Tap the touchscreen (when no device is detected) or use the device's default exit commands (e.g., shortcut keys, serial port commands) to exit;
3. **Frame Refresh**: The program cyclically collects sensor data in the background, with real-time frame refresh (PMOD_Thermal32 operates at 32FPS).

## 4. Notes
1. **Sensor Wiring**: The MLX90640 (core component of PMOD_Thermal32) has a fixed I2C address of 0x33; ensure correct wiring and no address conflicts;
2. **Camera Reset**: If the camera malfunctions in fusion mode, the program automatically attempts to reset the camera. A brief interruption of the frame during reset is normal;
3. **Resource Occupancy**: The program occupies hardware resources such as the display, I2C bus, and camera during operation. Do not manually occupy these resources before exiting the program to avoid conflicts;
4. **Temperature Parameters**: The default temperature range adapted by the program can be adjusted via code (currently set to dynamically adapt to temperature extremes, or fixed ranges such as 5.0℃~60.0℃ can be manually configured);
5. **Color Mapping**: Supports switching between different color mapping schemes (cmap) to adjust the color display effect of the thermal imaging frame as needed;
6. **Frame Scaling**: The original thermal imaging data is 32×24 pixels; the program uses scaling algorithms (BICUBIC/BILINEAR) to adapt to the display resolution, ensuring clear frame presentation;
7. **Extended Development**: The program reserves temperature data interfaces (e.g., `matrix()` to retrieve temperature matrix, `max_temp_point()` to retrieve coordinates and values of the maximum temperature), which can be used to extend functions such as temperature alarm and data storage.

## 5. Further Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_thermal_camera)