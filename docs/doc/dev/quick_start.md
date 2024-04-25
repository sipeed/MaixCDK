---
title: MaixCDK 快速开始
---


仅支持 Linux 系统，推荐使用 **Ubuntu >= 20.04**。
> 也可以使用 `Docker` 环境进行构建，参考 [Docker 环境使用方法](./docker/README.md)。
> `Docker` 环境准备好了 `ubuntu20.04` 系统和依赖，可以直接获取开始编译，对 `Docker` 熟悉或者搭建本地开发环境遇到问题可以参考。

* 本地编译环境准备
```
sudo apt update
sudo apt install git cmake build-essential python3 python3-pip
cmake --version # cmake 版本应该 >= 3.13, 或者也可以使用 `pip install cmake -U` 来安装最新版本的 cmake
```
> 如果你希望编译出来到 Linux PC 上跑，而不是交叉编译到开发板，如果是 `Ubuntu`，请使用系统版本`>=20.04`，否则有些依赖包可能会版本太旧无法编译通过，并且按照[Dockerfile](./docker/Dockerfile)里面的安装依赖的命令来安装依赖。
> 如果编译仍然报错，请使用`Docker`环境进行编译。

* 下载代码

```
git clone https://github.com/Sipeed/MaixCDK
```

稳定的 Release 版本请到 [release 页面](https://github.com/Sipeed/MaixCDK/releases) 下载。

> 中国国内用户可能克隆速度比较慢，可以使用`git clone https://gitee.com/Sipeed/MaixCDK`

* 安装依赖

```
cd MaixCDK
pip install -U pip                     # 更新 pip 到最新版本
pip install -U -r requirements.txt     # 安装依赖
```
> 中国国内可以加`-i https://pypi.tuna.tsinghua.edu.cn/simple`参数来使用清华源。
> `Docker`环境里面已经装好了，不过也可以在`Docker`容器里面执行命令更新到最新版本。

此时在终端执行`maixtool`和`maixcdk`命令可以看到帮助信息。
> 如果报错找不到命令，可以尝试重启终端，或者通过`find / -name "maixtool"`来找到`maixtool`命令的位置，然后通过`export PATH=maixtool所在目录;$PATH`来设置系统环境变量，重启终端就可以执行`maixtool`命令了。

* 编译

```shell
cd projects/hello_world
maixcdk menuconfig
```
然后在选项`Platform`中选择你的设备名，然后按`ESC`按键，再按`Y`保存退出。

```shell
maixcdk build
```
第一次执行这一步会根据设备下载编译工具链，下载如果太慢，可以根据提示手动下载到提示的目录，然后再执行编译。

编译完后在`build`目录下可以看到二进制程序文件，以及`build/dl_lib`下有依赖的`so`文件。

修改了代码后，再次执行`maixcdk build`即可编译。

> 因为使用了`cmake`作为编译工具，`build` 命令默认行为会重新执行`cmake`命令，如果希望不执行`cmake`命令只编译，可以使用`maixcdk build --no-rebuild`命令。

也可以执行`maixcdk distclean` 清除所有编译产生的临时文件以从一个干净的环境开始编译（但是这样构建时间会很长，一般编译出现奇怪的问题可以先尝试这样解决）。

* 上传程序到设备

拷贝 可执行文件 和 `dl_lib`文件夹 到设备运行。

* 运行

通过串口终端或者 ssh 终端 或者 ADB 等方式在终端运行程序。

* 打包发布

在工程目录下，使用 `maixcdk -P maixcam release` 可以为 `maixcam` 打包一个程序包并且存放在`dist`目录，可以上传发布到[MaixHub 应用商店](https://maixhub.com/app)。

安装包使用方法：
* 方法一： 直接解压拷贝到设置执行`chmod +x main.sh && ./main.sh`即可运行。
* 方法二： 在设备运行[应用商店]()应用，在 [MaixHub 应用商店](https://maixhub.com/app) 扫码安装。
* 方法三： 对于开发者，保证设备已经连到了和电脑所在局域网，在工程目录执行`maixcdk deploy` 会出现一个二维码， 在设备运行[应用商店]()应用扫码安装。

这里只是简单提及一下，发布的应用需要遵循[APP 开发准则](../convention/app.md)。

* 新建工程

使用命令创建工程：
```shell
maixcdk new
```