MaixCDK 应用框架指南
=======

## 简介

用户使用步骤：
* 设备启动时会自动启动 `launcher`。
* 用户选择一个 APP 启动。
* 运行选中的 APP。
* 用户与 APP 交互。
* 用户退出 APP。
* `launcher` 会再次启动并等待用户选择 APP。

用户安装新的 APP：
* 确保设备已连接到互联网（可以在 `app_settings` APP 中连接 WiFi）。
* 打开 [maixhub.com/app](https://maixhub.com/app) 查找 APP，点击“下载”按钮，会显示一个二维码和安装码。
* 在设备上打开 `app_store` APP，使用摄像头扫描二维码，或输入安装码来安装 APP。
* 返回 `launcher`，新安装的 APP 会显示在列表中。

## 打包 APP

如果使用 `MaixCDK`：
* 在项目文件夹中创建一个 `app.yaml`，格式见下文。
* 执行 `maixcdk release -P maixcam` 来为 `maixcam` 平台打包 APP。
* 在 `dist` 文件夹中会找到一个 `app_store_v1.0.0.zip`，这是打包好的 APP 文件。
* 你可以将这个包上传到 [maixhub.com/app](https://maixhub.com/app) 与他人分享。
* 或者执行 `maixcdk deploy -P maixcam` 来启动本地服务器，并显示一个二维码。
* 你也可以将这个文件上传到设备，并执行 `app_store install app_path.zip` 来通过命令安装。

如果使用 `MaixPy`，你可以使用 `MaixVision Workstation` 打包 APP，或手动使用 `maixtool`：
* 在项目文件夹中创建一个 `app.yaml` 文件，格式见下文。
* 创建一个 `main.py` 文件，这是 APP 的入口。
* 在此文件夹中执行 `maixtool release`，将生成 `dist/app_id_vx.x.x.zip`。
* 你可以将这个包上传到 [maixhub.com/app](https://maixhub.com/app) 与他人分享。
* 或者执行 `maixtool deploy` 来启动本地服务器，并显示一个二维码。

`app.yaml` 格式：

```yaml
id: my_app                         # 唯一 ID，使用小写字母并用下划线分隔单词
name: My APP
name[zh]: 我的应用                  # 中文名称
version: 1.0.0                     # 版本号，格式为 major.minor.patch
icon: assets/my_app.png            # 图标文件，可以是 png 或 lottie json 文件，或为空
author: Sipeed Ltd
desc: My APP description
desc[zh]: 我的应用描述

#### 包含文件方法 1：
# 默认情况下，会包含项目目录中的所有文件，除了排除文件
exclude:       # 不支持正则表达式，.git 和 __pycache__ 总是会被排除
  - .vscode
  - compile
  - build
  - dist
# extra_include:
#   src: dst
#   build/filename123: filename123

#### 包含文件方法 2：
# 白名单模式，只包含 files 字典中的文件。
# 如果没有此键或值为空，将使用方法 1。
# files:
#   - assets
#   - hello.py
#   - main.py

#### 包含文件方法 2.1：
# 白名单模式，只包含 files 字典中的文件。
# 如果没有此键或值为空，将使用方法 1。
# files:
#   assets: assets

```

`exclude` 为黑名单模式，`files` 为白名单模式，你可以选择其中一种方式。

## 文件约定

* 所有应用数据存储在 `/maixapp`。
* 应用存储在 `/maixapp/apps`。
* `/maixapp/apps/app.info` 为描述已安装应用的 INI 文件。安装和卸载应用会更新此文件。
> 开发者或用户也可以手动复制应用目录到此，并执行 `python gen_app_info.py` 生成 `app.info` 文件。
* 应用存放在 `/maixapp/apps/app_id` 文件夹中，每个应用必须包含 `app_id` 可执行文件，或 `main.sh` 脚本，或 `main.py` 脚本。
* 启动应用时，launcher 会在 `app_id` 文件夹中寻找：`main.sh` -> `main.py` -> `app_id` 文件。`main.sh` 使用 `sh` 执行，`main.py` 使用 `python3` 执行，`app_id` 直接执行。
* 共享数据存储在 `/maixapp/share`。
* 图片文件存储在 `/maixapp/share/picture`。
* 视频文件存储在 `/maixapp/share/video`。
* 临时数据可以存储在 `/maixapp/tmp`，注意，和 Linux 本身的`/tmp` 目录不同的是这个目录是在文件系统（SD卡）上的，系统`/tmp`是在内存上虚拟的文件系统，`/tmp`读写速度更快但是内存大小受限，大文件以及需要长期记录的日志文件（随着时间推移可能变得比较大）建议放在`/maixapp/tmp`目录下。
* 字体文件存储在 `/maixapp/share/font`。
* 图标文件存储在 `/maixapp/share/icon`。
* 应用运行时创建的数据文件可以存储在 `/maixapp/apps/app_id/data`。
* **所有路径都可以通过 API `maix.app.get_xxx_path` 获取，更多详情请参考 API 文档或 [maix_app.hpp](https://github.com/sipeed/MaixCDK/blob/main/components/basic/include/maix_app.hpp) 文件。**

## 切换应用

使用 `void maix::app::switch_app(const string &app_id, int idx = -1, const std::string &start_param = "")` 函数来切换应用。

这将退出当前应用并启动另一个应用，并将 `start_param` 字符串传递给目标应用，目标应用可以通过 `maix::app::get_start_param()` 获取此参数。
