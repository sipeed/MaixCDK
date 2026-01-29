## 1. Introduction
This tool is a camera UVC streaming application developed based on MaixCam series hardware. It can output video frames captured by the device's camera in MJPG format via the UVC (USB Video Class) protocol, supports local display screen status display and touchscreen quick exit operation, and is suitable for scenarios where the MaixCam device needs to be used as a USB camera.

## 2. Main Features
- Camera Capture: Automatically initializes camera frames with 1280×720 resolution and RGB888 format, with a maximum frame rate of 60fps;
- UVC Streaming: Automatically starts the UVC service when conditions are met, encapsulates camera frames into MJPG format for external streaming, and supports reception by third-party tools (e.g., Guvcview);
- Local Display: Displays UVC service status, enable prompts or error messages on the device's display screen;
- Quick Exit: The program can be quickly exited by clicking the upper left area of the touchscreen;
- Log Output: Real-time outputs running logs such as frame count, single-frame time consumption, frame rate, and data volume for easy debugging and status monitoring.

## 3. Usage Instructions
### 3.1 Preparations
1. Ensure the Maix device has the corresponding firmware installed, and the camera module is properly connected and driven;
2. Confirm that the UVC function is enabled in [App Settings/USB Settings/UVC].

### 3.2 Running the Program
1. Launch the program, which will automatically complete the following operations:
   - Load and display the initial image;
   - Initialize the camera and attempt to start the UVC service;
   - Display UVC startup success/unenabled prompt information on the display screen.

### 3.3 Receiving UVC Video Stream
1. If UVC starts successfully, connect the Maix device to the computer via USB;
2. Open a video capture tool that supports MJPG format on the computer (e.g., Guvcview);
3. Select the corresponding USB camera device in the tool and switch to the MJPG video channel to view the camera frames captured by the Maix device (frame size, frame count, etc., will be displayed on the screen).

### 3.4 Exiting the Program
- Touch the upper left corner of the device's display screen (within the range of 40+60 pixels in width and 40 pixels in height), and the program will automatically stop the UVC service and exit;

## 4. Notes
1. Compatibility: The Cheese tool on the Ubuntu system is incompatible with the UVC streaming of this program; it is recommended to use the Guvcview tool to receive the video stream;
2. Function Dependency: The UVC function must be enabled in [App Settings/USB Settings/UVC], otherwise the UVC service cannot be started, and the program will prompt to enable the UVC function first;
3. Frame Rate and Performance: The program is configured to capture camera frames at 60fps by default. If the device performance is insufficient, you can modify the frame rate parameter (60) in the camera initialization code to reduce the load.

1. Log Information: During program operation, logs such as frame count, single-frame time consumption, frame rate, and data bytes will be output, which can be viewed through the terminal to troubleshoot issues such as low frame rate and streaming stuttering;
2. Screen Customization: You can modify the `draw_string` related code in the program to customize the content, color, and size of the text displayed on the display screen or streaming frame;
3. Resolution Adjustment: If you need to adjust the streaming resolution, you can modify the width and height parameters (1280, 720) in the camera initialization code, but ensure that the camera supports the corresponding resolution;
4. Debug Mode: The code retains commented code (`#if 0` block) for saving frame data as JPG files. After uncommenting, the program will save each frame of MJPG data to the `/root/res/` directory, which is convenient for debugging video stream data issues (ensure the directory exists).

## 5. More Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_uvc_camera)