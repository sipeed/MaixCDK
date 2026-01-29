## 1. Introduction
This application is an integrated photo and video management and playback tool developed for the MaixCam platform. It supports managing image and video files by date, providing core features such as image viewing, video playback, and file deletion. It is optimized for the display and interaction logic of Maix series hardware, allowing for intuitive browsing and manipulation of local multimedia files.

## 2. Main Features
### 2.1 File Management
- Automatically scans multimedia files under specified paths (`/maixapp/share/picture` for images, `/maixapp/share/video` for videos) and organizes them by date directories.
- Automatically generates thumbnails (128x128 resolution) for images and videos to improve browsing efficiency.
- Supports batch deletion and single-file deletion. The interface updates automatically and local files are cleaned up after deletion.

### 2.2 Media Browsing and Playback
- Thumbnail mode allows for quick browsing of all images/videos; clicking a thumbnail switches to full-screen view/playback mode.
- **Full-screen Image Viewing:** Supports left/right swiping to switch between previous/next images.
- **Full-screen Video Playback:** Supports play/pause controls and progress bar dragging to adjust playback position. Automatically adapts to the display resolution.

### 2.3 Exception Handling
- Automatically ignores files that cannot be loaded and records them in an ignore list to avoid repeated loading failures.
- Automatically retries video decoding up to 3 times if it fails; displays a default blank screen if retries are unsuccessful.

## 3. User Guide
### 3.1 Launching the Application
Upon startup, the application automatically scans for image (`.jpg`/`.jpeg`/`.png`) and video (`.mp4`) files in the specified directories, generates a thumbnail list grouped by date, and defaults to the thumbnail browsing interface.

### 3.2 Thumbnail Browsing
- The interface displays all recognizable image/video thumbnails, grouped by date.
- **Click Image Thumbnail:** Enters full-screen image viewing mode.
- **Click Video Thumbnail:** Enters the video playback preparation interface and automatically loads the first frame of the video.

### 3.3 Full-screen Image Operations
- **View:** Displays the image in full screen, automatically adapting to the screen resolution.
- **Switch:** Click the left/right side of the screen to switch to the previous/next image.
- **Delete:** Triggers the delete operation (via the designated delete button on the interface). Deletes the current image and its corresponding thumbnail, then returns to the thumbnail list.

### 3.4 Full-screen Video Operations
- **Play/Pause:** Click the video area to toggle between Play (Icon 4) and Pause (Icon 5) states.
- **Adjust Progress:** Drag the progress bar to quickly jump to a specific position in the video; automatically seeks to the corresponding timestamp when the progress bar is released.
- **Exit Playback:** Automatically pauses on the last frame after playback ends; you can return to the thumbnail list.

### 3.5 File Deletion
- **Single File Deletion:** Trigger deletion while in full-screen view/playback mode to delete the current file.
- **Batch Deletion:** Enable batch delete in thumbnail mode, select multiple files to execute deletion. Local files and the interface list are cleaned up automatically.

## 4. Notes
### 4.1 File Path Requirements
- Images/videos must be stored in the specified directories: Images in `/maixapp/share/picture`, videos in `/maixapp/share/video`. Subdirectories must be named by date (e.g., `2024-01-01`), otherwise they may not be scanned or recognized.
- Only `.jpg`/`.jpeg`/`.png` image formats and H.264 encoded `.mp4` video formats are supported; other formats are automatically ignored.

### 4.2 Resolution and Format Limitations
- **Images:** It is recommended that the resolution does not exceed 2560x1440, and the width must be a multiple of 32.
- **Videos:** It is recommended that the resolution does not exceed 2560x1440, and the width must be a multiple of 32. Only H.264 encoded MP4 format is supported.
- Video playback relies on hardware decoding capabilities. High-resolution/high-bitrate videos may experience lag; it is recommended to use video files adapted to the screen resolution (e.g., 552x368).

### 4.3 Performance and Compatibility
- Thumbnail generation takes time. Interface response delays when loading a large number of files for the first time are normal.

### 4.4 Storage and Permissions
- The delete operation directly removes local files (including the original file and thumbnail) and is irreversible; please confirm before proceeding.
- Ensure the application has read/write permissions for the `/maixapp/share/picture` and `/maixapp/share/video` directories; otherwise, thumbnails cannot be created and files cannot be deleted.

### 4.5 Exception Handling
- If a video cannot play, check if the file is corrupted or if it is a standard H.264 encoded MP4 file.
- If an image cannot load, verify the file path and format are correct. Corrupted files are automatically added to the ignore list and need to be manually deleted before rescanning.

## 5. More Information
[Source Code](https://github.com/sipeed/MaixCDK/tree/main/projects/app_photos)