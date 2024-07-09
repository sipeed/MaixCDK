MaixCDK 开发准则和指导
===

## 总体准则

* 简单简洁易用。 不假设用户已经有特定的开发经验，尽量少操作，API 尽量简单，有详细指导文档。
> 比如 相比 让用户自己选择并下载工具链，根据用户选择的平台自动选择并下载工具链更好，入门门槛更低。
* 通用性。API 在设计时应该保证能在不同平台通用和统一抽象，如果做不到，那么这个功能可能得再三斟酌是否应该增加。
* 统一性。API 风格统一，代码风格统一。
* 扩展性。 保证核心功能的同时，尽量提供扩展接口，方便用户扩展。
* 深度性。API 足够简单，文档写明原理，源码尽量开源，方便用户深入研究。

## 源码和二进制文件

* **不用 git 子模块**：方便中国用户下载，因为如果子模块在 github，就算对仓库做了镜像，子模块始终还是要去 github 拉取，中国用户会非常缓慢。
* **不要往源码仓库放大文件和二进制文件**：这会大大增加仓库 git 的大小， git 只擅长管理文本文件，二进制文件采用下一条的方式管理。
* **对于第三方库和二进制文件，在编译前自动下载到本地**：在 `CMakeLists.txt`中填写`ADD_FILE_DOWNLOADS` 变量，这样在编译开始前会自动下载文件到`dl/pkgs`，然后自动解压到`dl/extracted`目录下，这样就能直接在`CMakeLists.txt`中引用源码了，比如`list(APPEND ADD_INCLUDE "${DL_EXTRACTED_PATH}/sunxi_mpp/sunxi-mpp-1.0.0/include")`。
> 每次编译时会将所有需要下载的文件列表写入到`dl/pkgs_info.json`，用户有网速问题的可以到官方或者第三方提供的网盘手动下载放到`dl/pkgs`目录即可，这样可以解决网络环境导致的下载速度慢问题。
* **不修改第三方库源码**：不修改第三方源码，方便升级第三方库。（如果以后发现有这样的需求可以考虑在解压后增加自动打 patch 的功能）

## MaixCDK 架构简介

对于普通应用开发者， 必要知识：

* 主要由`MaixCDK` 和 `project`两大部分组成，前者是`库`存放的地方，后者是应用代码存放的地方。
官方的例程和`APP`都直接放在`MaixCDK/examples` 和 `MaixCDK/projects`目录下，你也可以直接在`MaixCDK/projects`创建你的应用项目。

另外也可以两者分开放，在系统环境变量中加一个变量`MAIXCDK_PATH`，值为`MaixCDK`目录比如`/home/xxx/MaixCDK`，比如编辑`~/.bashrc`添加`export MAIXCDK_PATH=/home/xxx/MaixCDK`。
这样项目就可以放其它位置了比如`/home/xxx/projects`

* 组件：每个功能模块可以封装成一个组件，方便不同应用选择性地使用。
  * 可以看到[examples/hello_world](./examples/hello_world)中有个`main`组件，[components](./components)中有很多组件，还可以自己添加组件，比如`hello_world/component1`或者`hello_world/compoents/component2`。
  * 也可以设置环境变量`MAIXCDK_EXTRA_COMPONENTS_PATH`来指定其它额外的组件库。
  * 每个组件包含一个`CMakeLists.txt`来描述组件内容，比如`list(APPEND ADD_INCLUDE "include")`来指定包含的头文件路径，`list(APPEND ADD_SRCS "src/hello.c")`来包含源文件，`list(APPEND ADD_REQUIREMENTS basic)`来依赖其它组件等。
  * 另外，默认也会到 python `site-packages` 目录寻找，也就是说，如果你了解 python 库打包，你的组件包可以直接发布到 [pypi.org](https://pypi.org), 这样用户通过`pip install maixcdk-xxx` 就可以快速安装你的组件包了！ 可以参考[examples/maixcdk-example](./examples/maixcdk-example)组件。

* Kconfig: 带终端界面的可配置项。每个组件下面都可以有一个`Kconfig`文件，里面可以设置一些配置项，在执行`maixcdk menuconfig`时就可以看到选项了，保存后会在`build/config`目录下生成`global_config.cmake`,`global_config.h`文件，可以直接在组件的`CMakeLists.txt`和`c/cpp`文件中使用。`Kconfig`的语法可以参考其它组件，活话则自行搜索、参考[这里](https://github.com/ulfalizer/Kconfiglib/tree/master/tests)等。

* 依赖的第三方库： 两种方式，开发者可以自行选择，推荐集成到`MaixCDK`中的使用方式一，个人开发者发布的组件可以使用方式二。
  * 方式一：依赖的第三方库在编译时会自动被下载到`dl`文件夹，都在组件`CMakeLists.txt`中指定需要下载的文件，编译时会将所有需要下载的文件列表写入到`dl/pkgs_info.json`，这样的好处是遇到网络问题可以手动下载放到对应位置。
  * 方式二：使用 python package 的方式将源码和资源文件都打包发布到 [pypi.org](https://pypi.org)，取名为`maixcdk-xxx`方便大家搜索到，这样用户通过`pip install maixcdk-xxx` 就可以快速安装你的组件包了，而且用户也可以在安装时通过`-i`参数来设置镜像源。可以参考[examples/maixcdk-example](./examples/maixcdk-example)组件。

* 文档：在[docs](./docs/)目录下包含了应用文档和 API 文档， API 文档是从代码自动生成的，不要手动修改，应用文档是具体功能的入门指导文档。


## 代码风格

为了保持 MaixCDK 和 MaixPy API 的统一性，以及方便用户和开发者阅读，简单规范了以下代码风格：

* **函数名**：全小写，单词之间用下划线隔开，比如`get_name`。
* **变量名**：全小写，单词之间用下划线隔开，比如`file_path`。
* **类名**：大驼峰，某些简单的缩写词可以全大写，比如 `Person`, `YOLOv2`, `UART`, `PWM`。
* **宏**：全大写，单词之间用下划线隔开，比如`MAIXPY_VERSION`。
* **类成员变量**：不用加`m_`前缀，全小写+下划线，比如`name`，这样方便`Python`里的`API`足够简洁。
* **类私有成员变量**：前面加`_`，比如`_name`。
* **使用类成员变量**：因为成员变量没有类似`m_`开头的明显标识，所以显示地使用`this->`，比如`this->name`，而不是`name`来提高可阅读性。
* **namespace**：所有`MaixCDK`官方的`API`都在`maix`命名空间下，`maix`下再有一个`namespace`用来区分不同的功能，可以是一个头文件一个`namespace`，也可以是一个目录一个`namespace`，比如比如`maix::thread`、`maix::peripheral`、`maix::nn`、`maix::peripheral::uart`等。
在生成`MaixPy` `API` 时，也会自动生成相应的模块，比如`maix.thread`、`maix.peripheral`、`maix.nn`、`maix.peripheral.uart`等。
* **源文件命名**：全小写，单词之间用下划线隔开，比如`maix_peripheral_uart.cpp`， `C++` 头文件使用`.hpp`后缀。
* **注释**：`API`的注释使用`doxygen`风格，参考[components/basic/include/maix_api_example.hpp](../../../components/basic/include/maix_api_example.hpp)中的使用方法。
* **API 注释文档**：一般需要写以下关键字
  * **brief**: 一句话描述功能
  * **param**: 参数说明，一定要详细说明参数的取值要求（比如取值范围），注意点，不能是简单的参数名称复读，那文档就没什么意义了，还不如不写。也可以加数据方向，比如`param[in] a`表示`a`是输入参数，`param[out] b`表示`b`是输出参数，`param[in,out] c`表示`c`是输入输出参数。
  * **return**：返回值说明，一定要详细说明返回值的取值情况（比如取值范围），注意点，同样不能是简单的返回值名称复读。
  * **retval**：出了使用`return`统一说明返回值，也可以用过过个这个关键字来阐述不同返回值的意义，比如`@retval 0 success`。
  * **attention**： API 使用的注意点。
  * **maixpy**：标识这是一个 `MaixPy` `API`，内容为在`MaixPy`中的包名+变量名，比如`maix.example`、`maix.example.hello`、`maix.example.Example.name`等。注意加了这个关键词不仅会生成`MaixPy` `API`和文档，如果没有同时指定`maixcdk`关键字，还会用这个名称生成`MaixCDK`的`API`文档
  * **maixcdk**： 标识这是一个 `MaixCDK` `API`，如果同时存在`maixpy`关键字则优先使用这个作为`MaixCDK`的`API`名称。比如`* @maixcdk maix.example.hello_maixcdk`。

另外，对于本 SDK 中特殊的地方，比如为了能正确准确地生成 `MaixPy API`和文档，需要遵循：
* **API 中的类型和默认值需要完整的定义，不省略**，`maix`明明空间可以省略，具体地：
  * **命名空间写全**， 比如`image::Image`，而不是`Image`。
  * **枚举类型写全**，比如`image::Format::FMT_RGB888`，而不是`Format::FMT_RGB888`或者`image::FMT_RGB888`或者`FMT_RGB888`。


## API 规范化建议

* 如果 MaixCDK 有的 API， 尽量使用，而不是直接使用第三方 API，方便不同平台移植，比如文件操作用`maix_fs.hpp`中的 API，而不是直接使用 C++ 的`fstream`。
* 打印日志使用`maix_log.hpp`中的`log::info` `log::debug`等 API， 这样在`release`版本中`debug`日志会被自动去掉，而且可以通过`maixcdk menuconfig`来设置日志级别。也方便不同平台移植。
* 错误代码使用`maix_err.hpp`中的`err::Err`，统一管理所有错误代码，方便用户查找错误原因。
* 抛出异常使用`maix_err.hpp`中的`err::Exception`，统一管理所有异常，方便用户查找错误原因， 并且**需要在 API 注释中说明可能抛出的异常和类型**。
* 对于多个模块有共性的 API，尽量有一个共同特性的基类，其它类继承并实现，比如`maix_peripheral_uart.hpp`中的`UART`继承`maix_comm.hpp`中的通信基类`CommBase`，这样方便 C++ 开发，也方便编写`MaixPy`的 API。
> 比如 `Display` 类，可以直接暴露给 MaixPy 作为 API，由于不同平台实现可能不同，定义一个`DisplayBase`基类，不同平台实现自己的`Display`类比如`SDL_Display`，然后在`Display`类里面统一调用`DisplayBase`的 API，这样就可以保证`MaixPy`的 API 通用性，用户只需要知道`Display`类，不需要知道下面用了什么具体的实现方法，提高移植性的同时用户使用也更加简单。

## 如何添加 API

* 确认功能，需要添加的 API 的作用，并且在[docs/doc/application](../application) 目录下找到合适的地方添加一份**应用文档**，用来阐述功能和使用方法等，相比于 API 文档，用户和开发者通过这个文档可以了解到更多原理和使用方法。
文件名全小写，单词使用下划线隔开比如`peripheral/uart.md` 或者`ai/yolov2.md`。
* 将开发这个模块的目的和功能详细写出来
* 注明 API 设计参考了哪些资料或者开源项目，方便大家审阅 API 的通用性和合理性，会更快通过审阅。
* 参考[components/basic/include/maix_api_example.hpp]，在合适的 `component`中添加 API，如果是新的组件，尽量先在`issue`中讨论合理性。
> 注意这里使用了注释来标识 `API`，方便自动生成文档和`MaixPy`源码，所以要十分小心注释的时候，具体请看上面的注释规范说明以及`maix_api_example.hpp`文件。
> 另外为了让在`MaixCDK`中写的 `API`能顺利生成 `MaixPy` 的 `API`，而且因为`MaixCDK`中不包含 `Python.h` 和 `Pybind11.h`等跟 `Python`相关的定义，
> 语言自带的类型都是最终由`pybind11`自动转换的，比如`void hello(std::string a, std::vector<int> b)` 最终等价 `MaixPy` 中的`def hello(a: str, b: list)`。
> 常见的比如 `std::vector`对应`list`， `std::map`对应`dict`, `std::valarray`(作为参数可以接受`list`和`bytes`, 返回值会变成 `list`) 和 `maix::Bytes`(作为参数和返回值都是`bytes`)对应 `bytes`，`std::function` 对应 MaixPy 中的函数等，具体更多可以参考[pybind11 文档](https://pybind11.readthedocs.io/en/stable/advanced/cast/overview.html#conversion-table)
* 在`projects`目录添加一个例程。
* 在`test/test_examples`中添加一个测试用例，并执行**全部**测试用例并保证在本地通过（即执行`test/test_examples/auto_test.sh`）。
* 编译会自动生成文档，检查 `docs/doc/api`下生成的文档是否有误，有误则检测修改代码。
* 编译测试[MaixPy](https://github.com/sipeed/MaixPy)工程基于更新后的 `MaixCDK` 能否通过，以及查看生成的`MaixPy`文档是否有误，有误则检测修改代码。
* `git`提交代码到`github`自己的仓库，等待`action`自动构建和测试，检查是否有错误，如果有错误，及时修改。
* 所有在线测试无误后，提交 `PR`(`Pull Request`)， 在 [github](https://github.com/sipeed/MaixCDK)请求合并到`master`分支。

## 手动添加 MaixPy API

上面的方法可以自动生成 MaixPy API，但是在某些场景下可能需要手动添加，比如 参数是特定的类型，以 `numpy.array`为参数举例：
* 在 `MaixPy` 项目中的`components/maix/include`下添加头文件和代码，可以使用和 `MaixCDK`相同的`namespace`，比如`maix.image.cv2image`函数是将`numpy`数组转化为`image.Image`对象，具体看`convert_image.hpp`文件中的定义。


## 其它规范

* APP 开发规范和文件存放规范参考[app.md](./app.md)。
* 国际化（多语言）规范参考[i18n.md](./i18n.md)。
* APP 串口通信协议规范参考[protocol.md](./protocol.md)。


