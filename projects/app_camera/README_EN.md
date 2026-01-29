## 1. Introduction
This application is a camera control program developed based on Maix series hardware (MaixCam/Pro/MaixCam2). It integrates core functions such as photo capture, video recording, and parameter adjustment. It is compatible with camera sensors of different resolutions and supports features like audio-video synchronization and custom parameter configuration, meeting diverse usage requirements such as daily shooting and time-lapse photography.

## 2. Main Features

| Feature Category | Capabilities |
|------------------|--------------|
| **Basic Shooting** | Supports one-click photo capture with configurable delay (in seconds). Photos are automatically categorized by date and thumbnails are generated for easy preview. |
| **Video Recording** | Supports H.264 format video recording with audio-video synchronization. Video bitrate can be customized, and recommended bitrates are automatically adapted based on resolution. |
| **Parameter Adjustment** | **Shutter:** Auto/manual mode with customizable shutter speed.<br>**ISO:** Auto/manual mode (range: 100~800).<br>**Exposure Compensation (EV):** Auto/manual adjustment.<br>**White Balance (WB):** Auto/manual adjustment.<br>**Resolution:** Switch between multiple presets (e.g., 3840×2160, 2560×1440, 1920×1080). |
| **Auxiliary Functions** | **Fill Light:** Hardware fill light on/off control.<br>**Time-lapse:** Configurable interval; audio is automatically disabled when enabled.<br>**Focus:** Manual focus area setting.<br>**Timestamp:** Display current time on the screen.<br>**RAW Format:** Option to save photos in RAW format. |

## 3. User Guide

### 3.1 Basic Operations

#### 3.1.1 Taking Photos
1. Upon entering the app, the preview screen is displayed by default.
2. (Optional) Set a capture delay (0 seconds for immediate capture).
3. Click the capture button. If a delay is set, a countdown animation will appear. The photo is taken automatically after the countdown.
4. Photos are automatically saved to `[Image Storage Directory]/[Current Date]/[Index].jpg`, and a thumbnail is generated for preview.

#### 3.1.2 Video Recording
1. Click the Record Start button. The app prepares the environment and starts recording; the recording duration is displayed on screen.
2. During recording, the elapsed time is shown. The bitrate is auto-adapted but can be manually modified (only when not recording).
3. Click the Stop button to end recording. The video is saved to `[Video Storage Directory]/[Current Date]/[Index].mp4`.

### 3.2 Parameter Configuration

#### 3.2.1 Resolution Switching
1. Select a resolution preset in the settings menu.
2. The app will automatically restart the camera module to apply the new resolution.
3. Note: Options are limited by the hardware sensor; presets exceeding the sensor's maximum capability are disabled.

#### 3.2.2 Shutter/ISO Adjustment
*   **Auto Mode:** Check the "Auto" option. The camera adjusts Shutter/ISO based on the environment.
*   **Manual Mode:** Uncheck "Auto" and input the target Shutter speed (seconds) or ISO value. Changes take effect immediately.

#### 3.2.3 Auxiliary Function Switches
*   **Fill Light:** Toggle the button to turn the hardware light on or off.
*   **Timestamp:** Toggle the button to display the current date and time in the bottom-left corner.
*   **Time-lapse:** Set the interval in seconds (0 to disable, >0 for fixed interval). Audio is automatically disabled in this mode.
*   **RAW Format:** Toggle the button to save an additional RAW format file (`.raw` suffix) when capturing photos.

### 3.3 Preview and Playback
1. After capturing a photo, the thumbnail and a larger preview are displayed automatically.
2. Click the "View Photos" button to browse the photo gallery.

## 4. Notes

1.  **Resolution Switching:** Switching resolution restarts the camera module, causing a brief interruption in the preview. This is normal behavior.
2.  **Bitrate Modification:** Bitrate can only be modified when not recording. Attempting to change it during recording will show a "Video Busy" warning and will not take effect.
3.  **Time-lapse Photography:** Audio is automatically disabled in time-lapse mode. You need to manually re-enable it if you switch back to normal recording.
4.  **RAW Format:** Enabling RAW format increases file size significantly, and dedicated software is required for viewing/processing.
5.  **Fill Light:** The default hardware pin for the light is GPIOB3. This can be modified via the device configuration file (`cam_light_io`).
6.  **Storage Path:** Files are stored in date-based directories by default. Ensure sufficient storage space is available to prevent save failures.
7.  **AV Sync:** If audio stutters during recording, check the audio sample rate configuration (default is 48000Hz).

## 5. More Information

### 5.1 Hardware Compatibility
*   Supports **MaixCam** and **MaixCam2** platforms.
*   **MaixCam2** supports higher bitrates (up to 100Mbps) and features AI-ISP.
*   The fill light pin is configurable via the `cam_light_io` parameter in the device config file (default: B3).
*   The available resolution options are determined by the camera sensor's maximum resolution.

### 5.2 Storage Details
*   **Default Photo Path:** Defined by the system interface (usually under the app's picture directory).
*   **Default Video Path:** Defined by the system interface (usually under the app's video directory).
*   A `sync` operation is performed after saving to ensure data is written to the storage device, preventing data loss in case of sudden power loss.

### 5.3 Performance
*   The default preview frame rate is 30fps. Higher resolutions consume more system resources.
*   In time-lapse mode, the output video remains at 30fps, but frames are pushed at the set interval. This is suitable for long-duration, low-frame-rate recording.

### 5.4 Source Code
*   [Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_camera)