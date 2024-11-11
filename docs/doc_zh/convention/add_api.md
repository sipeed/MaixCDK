---
title: Add API for MaixCDK / MaixPy
---

## 代码规范

请先看[代码规范](./README.md)


## 如何添加 API

在 [快速开始](../README.md) 中末尾我们提到过通过注释的方式给 MaixPy 添加一个新的 API，看起来非常简单，只需要给 API 函数添加一个注释即可，比如
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
然后就可以在 MaixPy 中调用了：
```python
from maix import example

result = example.hello("Bob")
print(result)
```

为了尽量保证我们添加的 API 对用户**可用**，我们需要保证以下几个特性：
* API 名字和参数设计合理，通用性、跨平台性强。
* API 有注释（编译时会自动生成 API 文档）。
* API 不只是 API，有使用介绍文档教程和例程代码。

这里更详细地阐述和规范流程：

* **确认功能，先在[MaixPy 文档源码](https://github.com/sipeed/MaixPy/tree/main/docs/doc)中添加一份使用**文档和例程**。相当于设计文档，这样做可以避免直接写代码考虑不全后期不停修改 API，同时也有了文档。**（这很重要！！！）
* 可以在[docs/doc/application](https://github.com/sipeed/MaixCDK/tree/main/docs/doc/application) 目录下找到合适的地方添加一份**应用文档**，用来记录开发时的一些细节。文件名全小写，单词使用下划线隔开比如`peripheral/uart.md` 或者`ai/yolov2.md`。
* 最好是在开发文档里面注明 API 设计参考了哪些资料或者开源项目，方便大家审阅 API 的通用性和合理性，会更快通过审阅。
* 参考[components/basic/include/maix_api_example.hpp]，在合适的 `component`中添加 API，如果是新的组件，尽量先在[issues](https://github.com/sipeed/MaixCDK/issues)中讨论合理性。
> 注意这里使用了注释来标识 `API`，方便自动生成文档和`MaixPy`源码，所以要十分小心，具体请看上面的注释规范说明以及`maix_api_example.hpp`文件。
> 另外为了让在`MaixCDK`中写的 `API`能顺利生成 `MaixPy` 的 `API`，而且因为`MaixCDK`中不包含 `Python.h` 和 `Pybind11.h`等跟 `Python`相关的定义，
> 语言自带的类型都是最终由`pybind11`自动转换的，比如`void hello(std::string a, std::vector<int> b)` 最终等价 `MaixPy` 中的`def hello(a: str, b: list)`。
> 常见的比如 `std::vector`对应`list`， `std::map`对应`dict`, `std::valarray`(作为参数可以接受`list`和`bytes`, 返回值会变成 `list`) 和 `maix::Bytes`(作为参数和返回值都是`bytes`)对应 `bytes`，`std::function` 对应 MaixPy 中的函数等，具体更多可以参考[pybind11 文档](https://pybind11.readthedocs.io/en/stable/advanced/cast/overview.html#conversion-table)
* 在`examples`目录添加一个 C++ 例程，保证它能编译运行。
* 编译会自动生成文档，检查 `docs/doc/api`下生成的文档是否有误，有误则检测修改代码。
* 编译测试[MaixPy](https://github.com/sipeed/MaixPy)工程基于更新后的 `MaixCDK` 能否通过，以及查看生成的`MaixPy`文档是否有误，有误则检测修改代码。
* `git`提交代码到`github`自己的仓库，等待`action`自动构建和测试，检查是否有错误，如果有错误，及时修改。
* 所有在线测试无误后，提交 `PR`(`Pull Request`)， 在 [github](https://github.com/sipeed/MaixCDK) 和 [github](https://github.com/sipeed/MaixCDK) 分别请求合并到`dev`分支。

## 手动添加 MaixPy API

上面的方法可以自动生成 MaixPy API，但是在某些场景下可能需要手动添加，比如 参数是特定的类型，以 `numpy.array`为参数举例：
* 在 `MaixPy` 项目中的`components/maix/include`下添加头文件和代码，可以使用和 `MaixCDK`相同的`namespace`，比如`maix.image.cv2image`函数是将`numpy`数组转化为`image.Image`对象，具体看`convert_image.hpp`文件中的定义。


更多参考：
* [构建 MaixPy](https://wiki.sipeed.com/maixpy/doc/zh/source_code/build.html)
* [给 MaixCAM MaixPy 添加一个 C/C++ 模块](https://wiki.sipeed.com/maixpy/doc/zh/source_code/add_c_module.html)

