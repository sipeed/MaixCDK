## 1. Overview
This program is developed based on the MaixCam series hardware platform and serves as a real-time image classification and recognition application. It can invoke the device's camera to capture frames, perform classification recognition on the main subject in the frames using a preloaded MobilenetV2 neural network model, and display recognition results (category name, confidence level), performance time consumption, and other information on the screen in real time. It also supports touch-screen exit operation, adapting to the interactive features of Maix hardware.

## 2. Main Features
1. **Real-time Image Capture**: Invokes the device's camera to acquire real-time frames and automatically adjusts capture parameters to match the screen resolution;
2. **AI Classification Recognition**: Performs fast classification on camera frames based on the MobilenetV2 model and outputs the recognized category and confidence level (accuracy rate);
3. **Visual Display**: Real-time display on the screen includes:
   - Recognition frame (white rectangular frame at the center of the screen);
   - Recognition results (category name + confidence percentage);
   - Performance time consumption (total time, camera capture time, recognition time, display time);
   - Return icon (top-left corner of the screen);
4. **Quick Exit**: Touch the designated area in the top-left corner of the screen to exit the program quickly;
5. **Exception Handling**: Includes exception detection and log output for key links such as camera reading, image scaling, and model loading, facilitating problem troubleshooting.

## 3. Usage Instructions
1. Before running, ensure that the Maix series development board (equipped with a camera, touch screen, and display screen) is functioning properly, the camera is installed correctly and aimed at the object to be recognized, and place the classification model file `mobilenetv2.mud` in the `/root/models/` directory of the device. After starting the program, it will output log information such as "Program start" and "open camera success" and automatically enter the real-time recognition mode, with the camera frame displayed on the screen in real time. When using, aim the object to be recognized at the camera; the white rectangular frame at the center of the screen is the recognition area, and the recognition results (category name + confidence percentage) will be displayed below the frame. You can view the time consumption of each link (in ms) at the top of the screen, and touch the "return icon" area (approximately 100x100 pixels) in the top-left corner of the screen to stop and exit the program.

## 4. Notes
1. Abnormal Exit: If the program terminates abnormally, check the "error" information in the terminal log to locate the problem (e.g., camera reading failure, model loading failure, etc.);
2. Resource Release: The program will automatically release resources such as the camera, images, and models when exiting. Do not force termination (e.g., unplugging the power supply) to avoid resource occupation.

## 5. More Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_classifier)

[MaixCAM MaixPy Use AI Model for Object Classification](https://wiki.sipeed.com/maixpy/doc/en/vision/classify.html)