MaixCDK
===

English | [中文](./README_ZH.md)


MaixCDK (Maix C/CPP Development Kit) is a C/C++ development kit that integrates practical functions such as AI, machine vision, and IoT, providing easy-to-use encapsulation for quickly building your own projects in vision, artificial intelligence, IoT, robotics, industrial cameras, and more.

It is also the C/C++ version of [MaixPy](https://github.com/sipeed/MaixPy).

## Features

* Supports hardware-accelerated execution of AI models, including common classification, detection, and segmentation algorithms.
* Supports common vision algorithms such as color block detection, QR code recognition, apriltag detection, line following, etc.
* Supports OpenCV.
* Provides interfaces for common peripheral operations (UART, I2C, SPI, GPIO, PWM, ADC, camera, display, etc.).
* Cross-platform support, including Linux and Sipeed Maix series development boards (see table below for specifics).
* Easy-to-use API with commonly used function examples.
* Simple environment setup with one-click compilation.
* Supports online debugging of programs.
* Corresponding Python version (MaixPy) with automatically synchronized API updates.
* Complete ecosystem including MaixCDK + MaixPy + MaixVision (code writing, real-time image viewing, etc.) + MaixHub (application store, sharing, communication, etc.).

For more information, refer to the [MaixPy](https://github.com/sipeed/MaixPy) introduction and documentation.

## Supported Devices

| Device | Support Status |
| ------ | -------------- |
| Linux  | Partial        |
| [Sipeed MaixCAM](https://wiki.sipeed.com/maixcam) | Full |

## Quick Start

Click to view the [Quick Start](./docs/doc/dev/quick_start.md) document, which includes instructions on how to quickly download code, compile, and run.

## MaixCDK Development Guidelines

To help developers quickly understand MaixCDK and maintain the quality of long-term updates, please read the [MaixCDK Development Guidelines and Guidelines](./docs/doc/convention/README.md) before writing or contributing code.

## More Documentation

In addition to the development guidelines above, the documentation also includes application documents, development notes, and more for reference.

The documents are written in `Markdown` format under the [docs](./docs) directory. You can directly view the `.md` files or generate a web version of the documentation using the [teedoc](https://github.com/teedoc/teedoc) tool.

```shell
pip install teedoc -U  # Install teedoc via pip
cd docs                # Navigate to the docs directory
teedoc install -i https://pypi.tuna.tsinghua.edu.cn/simple  # Install teedoc plugins according to the documentation
teedoc serve           # Start a local web service to preview
```

Then, access the documentation via `http://127.0.0.1:2333` in your browser.

> If you need to generate offline documentation, use the `teedoc build` command. The generated documents will be in the `out` directory. Use `python -m http.server` in the `out` directory to start a local web service for preview.

## FAQ

If you encounter any issues during compilation or usage, check the [FAQ](./docs/doc/faq.md) for potential solutions.

## Open Source License

MaixCDK is licensed under the Apache 2.0 open source license. See the [LICENSE](./LICENSE) file for details.