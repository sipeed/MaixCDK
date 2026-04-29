## 1. Overview
This program is designed for hardware devices equipped with MaixCam series chips, implementing real-time object detection based on the YOLOv5 neural network model. It can access the device's camera to capture live frames, identify and frame objects in the画面, and display detection results in real time on the display screen. Additionally, it supports touchscreen operation—tapping the designated exit button area terminates program execution. The overall operation is simple with high real-time responsiveness for detection.

## 2. Key Features
1. **Real-time Object Detection**: Accesses the device's camera to acquire live video frames, identifies objects via the YOLOv5 model, and filters invalid detection results using a confidence threshold (0.5) and IoU (Intersection over Union) threshold (0.45).
2. **Visualized Display**: Marks object positions with red rectangular boxes and displays object category names on the detection screen. It also shows performance metrics including detection time consumption (camera capture, model inference, screen display), total time consumption, and real-time frame rate.
3. **Touch Exit Function**: An exit button (ret.png image) is displayed in the upper-left corner of the screen; touching this area immediately exits the program.
4. **Exception Handling**: Built-in exception detection and log output for camera reading, model loading, image resizing, and other processes to ensure stable program operation.

## 3. Usage Instructions
### 3.1 Running and Operation Steps
1. Upload the compiled program files to the specified directory on the device.
2. Execute the program startup command via the terminal or locally on the device. After startup, the camera activates automatically, the display screen shows real-time detection frames, and an exit button appears in the upper-left corner of the screen.
3. During detection, the screen real-time displays object bounding boxes, object category names, as well as time consumption for camera capture, model detection, screen display, total time consumption, and real-time frame rate.
4. Exiting the program:
   - Normal method: Touch the exit button area in the upper-left corner of the display screen with your finger to terminate the program immediately.
   - Emergency method: If running the program via the terminal, press the Ctrl + C shortcut to force exit.

## 4. Notes
1. **File Path**: Ensure the file path of `yolov5s.mud` matches the path specified in the program. Incorrect paths will cause program startup failure and output error logs.
2. **Hardware Compatibility**: The program automatically adapts font size and button dimensions based on camera resolution. For devices with special screen/camera resolutions, the button may display incompletely or the touch area may shift—adjust the button scaling parameters in the code if needed.
3. **Performance Metrics**: Time consumption, frame rate, and other performance data are for reference only and vary with different hardware configurations (chip model, memory size).
4. **Exception Handling**: If the program fails to start, check log information to locate issues (e.g., model loading failure, unrecognized camera, missing image files).
5. **Resource Release**: The program automatically releases resources (camera, display screen, images, detection results) upon exit—manual cleanup is unnecessary. Avoid forced power-off to prevent resource occupation.

## 5. Further Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_detector)

[MaixPy MaixCAM: Object Detection with YOLOv5 / YOLOv8 / YOLO11 Models](https://wiki.sipeed.com/maixpy/doc/en/vision/yolov5.html)
