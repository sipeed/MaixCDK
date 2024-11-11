---
title: 在 MaixCDK 中使用 LVGL
---

## 运行 gui_lvgl 示例

* 进入 MaixCDK 的 `examples/gui_lvgl` 目录。
* 执行 `maixcdk build` 来在 PC 上构建项目。
* 执行 `maixcdk run` 来在 PC 上运行项目。

你还可以执行 `maixcdk menuconfig` 来切换平台，构建适用于其他平台的项目。

## 自定义 UI

* 首先下载并安装 [Squareline](https://squareline.io/)。
* 在 Squareline 中创建一个新的 UI 项目。
* 编辑你的 UI。
* 导出 UI 文件，此时会得到一个 `ui` 文件夹。
* 将所有 UI 文件复制到 `examples/gui_lvgl/ui` 文件夹。
* 运行 `maixcdk build` 进行构建。


