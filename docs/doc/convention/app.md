MaixCDK APP framework guide
=======

## Introduction

User use steps:
* When device boot up, will automatically start `launcher`.
* Use select one APP to start.
* Run selected APP.
* User interact with APP.
* User exit APP.
* `launcher` will start again and wait for user to select APP.

User install new APP:
* Ensure device is connected to internet(can connect WiFi in `app_settings` APP).
* Open [maixhub.com/app](https://maixhub.com/app) to find APP, click `download` button, and a `QR code` and a `install code` will be shown.
* Open `app_store` APP in device to open camera to scan QR code, or input install code to install the APP.
* Return to launcher, the new APP will be shown in the list.

## Pack APP

If use `MaixCDK`:
* Create a `app.yaml` in your project folder, see below for format.
* Execute `maixcdk release -P maixcam` to pack APP for `maixcam` platform.
* Then you will find a `app_store_v1.0.0.zip` in `dist` folder, this is the APP package.
* You can upload this package to [maixhub.com/app](https://maixhub.com/app) to share with others.
* Or you can execute `maixcdk deploy -P maixcam` to serve a local server and a QR code will be shown.
* Or you can upload this app file to device and execute `app_store install app_path.zip` to install by command.

If use `MaixPy`, you can use `MaixVision Workstation` to release APP, or use `maixtool` to release manually:
* Create a `app.yaml` in your project folder, see below for format.
* Create a `main.py` in your project folder, this is the entry of your APP.
* Execute `maixtool release` in this folder, `dist/app_id_vx.x.x.zip` will be generated.
* You can upload this package to [maixhub.com/app](https://maixhub.com/app) to share with others.
* Or you can execute `maixtool deploy` to serve a local server and a QR code will be shown.


`app.yaml` format:

```yaml
id: my_app                         # unique id, use lowercase and `_` to separate words
name: My APP
name[zh]: 我的应用                  # Chinese name
version: 1.0.0                     # version number, major.minor.patch
icon: assets/my_app.png            # can be png or lottie json file, or empty
author: Sipeed Ltd
desc: My APP description
desc[zh]: 我的应用描述

#### Include files method 1:
#         By default will include all files in project dir except exclude files
exclude:       # not support regular expression, .git and __pycache__ is always exclude
  - .vscode
  - compile
  - build
  - dist
# extra_include:
#   src: dst
#   build/filename123: filename123

#### Include files method 2:
#         White list mode, only include files in files dict.
#         If no this key or value is empty, will use method 1.
# files:
#   - assets
#   - hello.py
#   - main.py


#### Include files method 2.1:
#         White list mode, only include files in files dict.
#         If no this key or value is empty, will use method 1.
# files:
#   assets: assets

```

`exclude` is blacklist mode, `files` is whitelist mode, you can use one of them.


## Files convention

* All app data is stored in `/maixapp`.
* Apps are stored in `/maixapp/apps`.
* There is a `/maixapp/apps/app.info` INI file for simple description of installed apps. Install and uninstall APP will update this file.
> developer or use can manually copy APP directories here and execute `python gen_app_info.py` will generate `app.info` file.
* APP store in `/maixapp/apps/app_id` folder, every app must contain `app_id` executable file, or `main.sh` shell script, or `main.py` python script.
* When boot up APP, the launcher will find file in app_id folder: `main.sh` -> `main.py` -> `app_id`. The `main.sh` will be executed by `sh`, `main.py` will be executed by `python3`, `app_id` will be directly executed.
* All shared data is stored in `/maixapp/share`.
* All pictures are stored in `/maixapp/share/picture`.
* All video files are stored in `/maixapp/share/video`.
* Temporary data can be stored in `/maixapp/tmp`. Note that this directory is located on the file system (SD card), which differs from the system's `/tmp` directory. The `/tmp` directory on the system is a virtual file system in memory, offering faster read/write speeds but with limited memory size. Large files and log files that need to be stored long-term (which may grow over time) are recommended to be placed in the `/maixapp/tmp` directory.
* Font files are stored in `/maixapp/share/font`.
* Icon files are stored in `/maixapp/share/icon`.
* APP's data files created at runtime can be stored in `/maixapp/apps/app_id/data`.
* **All this path can be get by API `maix.app.get_xxx_path`, more detail see API doc or [maix_app.hpp](https://github.com/sipeed/MaixCDK/blob/main/components/basic/include/maix_app.hpp) file.**


## Switch from APP to anothor APP

Use `void maix::app::switch_app(const string &app_id, int idx = -1, const std::string &start_param = "")` function to switch APP.

This will exit current APP and start another APP, and parse `start_param` string to the second APP, the second APP can get this param by `maix::app::get_start_param()`.

