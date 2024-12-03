---
title: Quick Start with MaixCDK
---

## Introduction to MaixCDK

MaixCDK provides C++ APIs for commonly used AI functions related to image processing, vision, audio, and peripherals, enabling quick prototyping and stable product development.

MaixCDK is not only a C++ SDK but also auto-generates Python API bindings, so development can be done in Python through [MaixPy](https://wiki.sipeed.com/maixpy/).

Basic Linux knowledge and a fundamental understanding of cross-compilation are required to use MaixCDK.

## Basic Knowledge

To use MaixCDK, it is assumed that you are already familiar with the following knowledge. If not, please learn them first or consider using [MaixPy](https://wiki.sipeed.com/maixpy/):

* Proficient in using Linux for development, familiar with the terminal and common commands.
* Proficient in either C or C++ programming language. C++ proficiency is not required, but you must understand the basic syntax and object-oriented concepts.
* Able to actively read and analyze source code to troubleshoot issues.
* Chinese developers should be familiar with using network proxies.
* Understand cross-compilation.


## How to Find Resources and Troubleshoot

1. Read [MaixCDK source code](https://github.com/sipeed/MaixCDK).
2. Since MaixCDK shares the same functionality/API as MaixPy, detailed tutorials are not provided separately. Refer to the [MaixPy documentation](https://wiki.sipeed.com/maixpy/); the principles and code are nearly identical, requiring only minor modifications.
3. It’s recommended to get hands-on experience with MaixPy before using MaixCDK for a smoother learning curve.
4. Carefully review the official documentation, including MaixCDK, MaixPy, and hardware documentation.
5. If issues arise, start by checking [MaixCDK FAQ](https://wiki.sipeed.com/maixcdk/doc/zh/faq.html), [MaixPy FAQ](https://wiki.sipeed.com/maixpy/doc/zh/faq.html), [MaixCAM Hardware FAQ](https://wiki.sipeed.com/hardware/zh/maixcam/faq.html), and the [source code issues](https://github.com/sipeed/MaixCDK/issues).
6. Check out the [MaixHub Share Plaza](https://maixhub.com/share) for community insights.
7. **Read error logs carefully and patiently**, following logs from top to bottom, as errors might appear in the middle—don’t skip ahead hastily!

## Quick Start

### Setting Up the System and Environment

Two options are available:
#### Local Machine:

**Only supported on Linux**, with **Ubuntu >= 20.04** recommended.
Note that the MaixCAM toolchain only supports x86_64 CPUs and is not compatible with ARM-based computers, such as ARM MacOS.

```shell
sudo apt update
sudo apt install git cmake build-essential python3 python3-pip autoconf automake libtool
cmake --version # cmake version should be >= 3.13
```
> To compile for a Linux PC (instead of cross-compiling for a dev board), if you’re using `Ubuntu`, ensure the system version is `>=20.04`, or some dependencies may be too outdated to compile. Install dependencies following the commands in the [Dockerfile](https://github.com/sipeed/MaixCDK/blob/main/docs/doc/dev/docker/Dockerfile).
> If compilation errors occur, consider [using Docker to compile](./dev/docker/README.md).

#### Docker:

The `Docker` environment includes `ubuntu20.04` and dependencies, making it ready for compilation. For users familiar with `Docker` or those facing issues with local setup, refer to the [Docker Usage Guide](./dev/docker/README.md).

### Obtaining the Source Code

```shell
git clone https://github.com/Sipeed/MaixCDK
```

> * Stable Release versions can be downloaded from the [release page](https://github.com/Sipeed/MaixCDK/releases).
> * Alternatively, check [MaixPy release](https://github.com/Sipeed/MaixPy/releases) assets for `maixcdk_version_xxxxx.txt`, where `xxxxx` indicates the MaixPy version in use. Use `git checkout xxxx` to switch to the corresponding version.

> For users in China experiencing slow clone speeds, use `git clone https://gitee.com/Sipeed/MaixCDK`.

### Installing Dependencies

```shell
cd MaixCDK
pip install -U pip                     # Update pip to the latest version
pip install -U -r requirements.txt     # Install dependencies
```
> Users in China can add the parameter `-i https://pypi.tuna.tsinghua.edu.cn/simple` to use the Tsinghua mirror.
> Dependencies are already installed in the `Docker` environment, but you can update them to the latest version within the Docker container.

Run `maixtool` and `maixcdk` commands in the terminal to view help information.
> If the commands are not found, try restarting the terminal or use `find / -name "maixtool"` to locate `maixtool`, then set it in the system path using `export PATH=directory_containing_maixtool:$PATH`.

### Compilation

```shell
cd examples/hello_world
maixcdk menuconfig
```
Follow the prompts to select the device platform. A menu appears to configure parameters. For first-time use, the default parameters are fine (or skip `menuconfig` and go straight to build), then press `ESC`, and `Y` to save and exit.

```shell
maixcdk build
```
The first time you run this step, the device will download the compilation toolchain. If the download is slow, you can manually download it to the specified directory as prompted, and then proceed with the compilation.

>! The resources are mostly downloaded from GitHub, and download speeds in China may be slow or even fail (the list of files to download is in `dl/pkgs_info.json`). There are several common solutions:
> 1. Set a proxy in the terminal (recommended), for example: `export http_proxy=http://127.0.0.1:8123 https_proxy=http://127.0.0.1:8123`. Here, `http://127.0.0.1:8123` is the address of your HTTP proxy.
> 2. Manually download to the `dl/pkgs` folder: During the download, the download URL and local path will be printed. You can manually download the files and place them in the corresponding directory. When you run the build command again, it will use the locally prepared files (Note: the file’s sha256 checksum must match, i.e., it must be the same file).
> 3. Users in China can go to the [QQ Group](../) on the homepage, find the `MaixCDK` folder, and download the files to the `MaixCDK/dl` directory.

>! For common errors and solutions, refer to the [FAQ](./faq.md).

After the compilation, you will find the binary program files in the `build` directory, and the dependent `.so` files in `build/dl_lib`.

After modifying the code, you can run `maixcdk build` again to compile.

If you **haven’t added or removed source files**, you can run `maixcdk build2` or `maixcdk build --no-gen` for faster compilation (it will only compile the modified files).  
> This is because the `build` command starts the entire build process from scratch, scanning files and recompiling. In contrast, the `build2` command will not scan for file additions or deletions and will only compile the edited files.  
> Note: The `build2` command will not detect file additions or deletions, so if you **add or remove files**, you must run the `build` command again.

Run `maixcdk distclean` to clear all temporary build files and start fresh (this increases build time and is typically used to troubleshoot issues).

### Uploading to the Device

Copy the executable and `dl_lib` folder to the device for execution.

Use `scp` to copy files, for example:
```shell
scp -r dist/ root@10.127.117.1:/root/
```
The default password is `root`.

### Running the Program

Run the program via SSH. Ensure no other programs are running (including the startup application launcher).
Steps:
* Connect MaixVision to the device to close the Launcher, or use SSH to `kill` the `launcher_daemon`.
* SSH into the device, e.g., `ssh root@192.168.0.123`, password `root`.
* Navigate to the executable directory and run it, e.g., `cd /root/dist/camera_display_release && ./camera_display`.

### Packaging for Release

In the project directory, use `maixcdk -p maixcam release` to create a program package for `maixcam` in the `dist` folder. This package can be uploaded to the [MaixHub App Store](https://maixhub.com/app).

Usage:
* Method 1: Unzip and copy to the device, run `chmod +x program_name && ./program_name`.
* Method 2: Use the [App Store](https://maixhub.com/app/12) application on the device to install from MaixHub.
* Method 3: For developers, if the device is connected to the same LAN as the PC, use `maixcdk deploy` to generate a QR code, which the device can scan to install.
* Method 4: Copy the package to the device and install it by running `/maixapp/apps/app_store/app_store install xxx.zip`.

Application releases should follow the [App Development Guidelines](./convention/app.md).

### Creating a New Project

To create a project:
```shell
maixcdk new
```

## Development convention

To get started with MaixCDK, please begin by reading the [MaixCDK Development Guidelines](./convention/README.md).

## Adding an API to MaixPy

Since MaixPy’s core is largely MaixCDK, adding APIs to MaixPy is straightforward. Just annotate the function with `@maixpy maix.xxx.xxxx`.

For example, to implement the following API:
```python
from maix import example

result = example.hello("Bob")
print(result)
```

Simply add a declaration in [maix_api_example.hpp](https://github.com/sipeed/MaixCDK/blob/main/components/basic/include/maix_api_example.hpp):
```cpp
namespace maix::example
{
    /**
     * @brief say hello to someone
     * @param[in] name name of someone, string type
     * @return string type, content is hello + name
     * @maixpy maix.example.hello
     */
    std::string hello(std::string name);
}
```
Then compile the `MaixPy` project to get an installation package, which can be installed on the device to use the new API—simple, right?

For more detailed documentation, refer to [Add API](./convention/add_api.md).

