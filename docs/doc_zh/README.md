---
title: MaixCDK 快速开始
---

## MaixCDK 基本介绍

MaixCDK 使用 C++ 封装了常用的 AI 图像、视觉、语音，外设相关的 API，使用 MaixCDK 可以快速验证产品原型和开发稳定的产品。

MaixCDK 不光是一套 C++ SDK，同时会自动生成 Python API 绑定，即可以使用 Python 进行开发，也就是[MaixPy](https://wiki.sipeed.com/maixpy/)。

使用 MaixCDK 需要基本的 Linux 使用经验和懂交叉编译的基本概念。

## 基础知识

要使用 MaixCDK，均假设你已经会以下知识，不会请先自行学习或者请转角使用[MaixPy](https://wiki.sipeed.com/maixpy/)：
* 熟练使用 Linux 进行开发，熟悉终端以及常见命令使用。
* 熟练掌握 C/C++ 其中一种语言，其中 C++ 可以不熟练但是必须了解基础语法和面向对象概念。
* 会主动看源码分析问题。
* 中国开发者须了解使用网络代理。
* 了解交叉编译。


## 如何找资料和解决问题

1. 多看 [MaixCDK 源码](https://github.com/sipeed/MaixCDK)。
2. 由于功能 / `API` 和 `MaixPy` 相同，`MaixCDK` 不再单独提供详细的教程，请参考[MaixPy 文档](https://wiki.sipeed.com/maixpy/)，原理和代码基本一致，稍微转换一下就能使用。
3. 建议不要直接从 `MaixCDK` 上手，先体验完基本的 `MaixPy` 使用再使用 MaixCDK 会轻松很多。
4. 多看仔细看官方文档，包括 MaixCDK、MaixPy、硬件文档。
5. 遇到问题先从 [MaixCDK FAQ](https://wiki.sipeed.com/maixcdk/doc/zh/faq.html), [MaixPy FAQ](https://wiki.sipeed.com/maixpy/doc/zh/faq.html), [MaixCAM 硬件 FAQ](https://wiki.sipeed.com/hardware/zh/maixcam/faq.html) 以及[源码 issues](https://github.com/sipeed/MaixCDK/issues) 等地方找答案。
6. [MaixHub 分享广场](https://maixhub.com/share) 看社区成员的经验。
7. **仔细 耐心** 看 错误`LOG`， 从上到下看日志，出错日志有时可能在中间，不要跳过着急！！


## 快速开始

### 准备系统和环境

两种方式：
#### 本机：

**仅支持 Linux 系统**，推荐使用 **Ubuntu >= 20.04**。
注意 MaixCAM 工具链只支持 x86_64 CPU，不支持 ARM 电脑，特别是如果你是 ARM MacOS 则无法编译过。


```
sudo apt update
sudo apt install git cmake build-essential python3 python3-pip autoconf automake libtool
cmake --version # cmake 版本应该 >= 3.13
```
> 如果你希望编译出来到 Linux PC 上跑，而不是交叉编译到开发板，如果是 `Ubuntu`，请使用系统版本`>=20.04`，否则有些依赖包可能会版本太旧无法编译通过，并且按照[Dockerfile](https://github.com/sipeed/MaixCDK/blob/main/docs/doc/dev/docker/Dockerfile)里面的安装依赖的命令来安装依赖。
> 如果编译仍然报错，请[使用 Docker 环境进行编译](./dev/docker/README.md)。

#### Docker：

`Docker` 环境准备好了 `ubuntu20.04` 系统和依赖，可以直接获取开始编译，**对 `Docker` 熟悉 或者 搭建本地开发环境遇到问题**可以参考。
详细请看 [Docker 环境使用方法](./dev/docker/README.md)。


### 获取源码

```
git clone https://github.com/Sipeed/MaixCDK
```

> * 稳定的 Release 版本可以到 [release 页面](https://github.com/Sipeed/MaixCDK/releases) 下载。
> * 另外也可以看 [MaixPy release](https://github.com/Sipeed/MaixPy/releases) assets 下的 `maixcdk_version_xxxxx.txt`， 这个`xxxxx` 就是 MaixPy 对应 release 使用的版本，可以使用`git checkout xxxx`来切换到对应的版本。

> 中国国内用户可能克隆速度比较慢，可以使用`git clone https://gitee.com/Sipeed/MaixCDK`

### 安装依赖

```
cd MaixCDK
pip install -U pip                     # 更新 pip 到最新版本
pip install -U -r requirements.txt     # 安装依赖
```
> 中国国内可以加`-i https://pypi.tuna.tsinghua.edu.cn/simple`参数来使用清华源。
> `Docker`环境里面已经装好了，不过也可以在`Docker`容器里面执行命令更新到最新版本。

此时在终端执行`maixtool`和`maixcdk`命令可以看到帮助信息。
> 如果报错找不到命令，可以尝试重启终端，或者通过`find / -name "maixtool"`来找到`maixtool`命令的位置，然后通过`export PATH=maixtool所在目录;$PATH`来设置系统环境变量，重启终端就可以执行`maixtool`命令了。

### 编译

```shell
cd examples/hello_world
maixcdk menuconfig
```
根据提示选择设备平台，会出来一个界面可以配置一些参数，初次使用用默认的参数即可（用默认的参数也可以不执行 menuconfig, 直接执行 build 命令），然后按`ESC`按键，再按`Y`保存退出。

```shell
maixcdk build
```
第一次执行这一步会根据设备下载编译工具链，下载如果太慢，可以根据提示手动下载到提示的目录，然后再执行编译。
>! 下载的资源基本都是 github 上的，中国国内下载速度可能会比较慢甚至失败（要下载的文件列表在`dl/pkgs_info.json`），有几种常见解决方法：
> 1. 终端设置代理（推荐）， 比如：`export http_proxy=http://127.0.0.1:8123 https_proxy=http://127.0.0.1:8123`，这里`http://127.0.0.1:8123`就是你的`http`代理的地址。
> 2. 手动下载到`dl/pkgs`下：下载时，会打印下载连接和下载到本地的路径，手动下载文件方到对应路径，再次执行编译就会直接使用本地准备好的（注意文件 sha256 校验值必须相同即同一个文件）。
> 3. 中国国内用户可以到 [首页 QQ 群](../) 群文件`MaixCDK`文件夹下载放到`MaixCDK/dl`目录下。


>! 常见错误和解决方法看[FAQ](./faq.md)

编译完后在`build`目录下可以看到二进制程序文件，以及`build/dl_lib`下有依赖的`so`文件。

修改了代码后，再次执行`maixcdk build`即可编译。

如果你**没有增删源码文件可以执行`maixcdk build2` 或者 `maixcdk build --no-gen` 编译会更快**（只编译修改了的文件）。
> 因为`build`命令会从头开始构建，扫描文件再编译，`build2`命令则不会扫描文件增删，直接编译编辑过的文件。
> 注意`build2`命令不会检测到文件增加或者删除，如果**增删了文件必须再执行一遍`build`命令**。

也可以执行`maixcdk distclean` 清除所有编译产生的临时文件以从一个干净的环境开始编译（但是这样构建时间会很长，一般编译出现奇怪的问题可以先尝试这样解决）。

### 上传程序到设备

拷贝 可执行文件 和 `dl_lib`文件夹 到设备运行。

可以通过 `scp`命令来拷贝，比如：
```shell
scp -r dist/ root@10.127.117.1:/root/
```
默认密码是`root`

### 运行

通过 ssh 终端运行程序。注意要保证没有程序正在运行（包括开机的应用选择界面(Launcher)）。
具体方法：
* MaixVision 连接设备，这会让 Launcher 退出。也可以在 ssh 终端手动`kill` `launcher_daemon`程序。（**重要!!**）
* ssh 连接设备，比如 `ssh root@192.168.0.123`， 密码是 `root`。
* 然后到可知性文件目录下执行文件， 比如 `cd /root/dist/camera_display_release && ./camera_display`

### 打包发布

在工程目录下，使用 `maixcdk -p maixcam release` 可以为 `maixcam` 打包一个程序包并且存放在`dist`目录，可以上传发布到[MaixHub 应用商店](https://maixhub.com/app)。

安装包使用方法：
* 方法一： ssh 终端，直接解压拷贝到设备，执行`chmod +x 程序名 && ./程序名`即可运行。
* 方法二： 在设备运行[应用商店](https://maixhub.com/app/12)应用，在 [MaixHub 应用商店](https://maixhub.com/app) 扫码安装。
* 方法三： 对于开发者，保证设备已经连到了和电脑所在局域网，在工程目录执行`maixcdk deploy` 会出现一个二维码， 在设备运行[应用商店](https://maixhub.com/app/12)应用扫码安装。
* 方法四： 拷贝安装包到设备，在设备执行`/maixapp/apps/app_store/app_store install xxx.zip` 即可安装应用。

这里简单提及一下，发布的应用需要遵循[APP 开发准则](./convention/app.md)。

### 新建工程

使用命令创建工程：
```shell
maixcdk new
```

## 为 MaixPy 添加 API

因为 MaixPy 底层大多数就是 MaixCDK，而为 MaixPy 添加 API 也非常简单，给函数加一个注释`@maixpy maix.xxx.xxxx` 就可以了。

比如要实现以下 API：
```python
from maix import example

result = example.hello("Bob")
print(result)
```

只需要在[maix_api_example.hpp](https://github.com/sipeed/MaixCDK/blob/main/components/basic/include/maix_api_example.hpp) 中添加声明：
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
然后编译`MaixPy`项目得到 MaixPy 安装包，安装到设备即可使用新的 API 了，简单吧！

更详细的文档请看 [添加 API](./convention/add_api.md)



## 开发准则

要开始使用 MaixCDK，请从阅读[MaixCDK 开发准则](./convention/README.md) 开始。


