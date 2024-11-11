---
title: Use LVGL in MaixCDK
---

## run gui_lvgl demo

* Go to `examples/gui_lvgl` in MaixCDK.
* Execute `maixcdk build` to build the project for PC.
* Execute `maixcdk run` to run the project on PC.

You can also execute `maixcdk menuconfig` to change platform to build for other platform.

## Custom your own UI

* Download [squareline](https://squareline.io/) and install first.
* Create a new UI project in squareline.
* Edit your UI.
* Export UI files, then you will get a `ui` folder.
* Copy all ui files to `examples/gui_lvgl/ui` folder.
* Run `maixcdk build` to build.

Or you can mannually write code and not use squareline.
