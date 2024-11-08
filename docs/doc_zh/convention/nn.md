# NN 规范
======

## 神经网络模型文件

每个 AI 模型都使用 `.mud` 文件进行描述，这是一种 INI 格式的文件。

MUD（Model Universal Description，模型通用描述）文件是一种简单的模型描述格式。
> 创建这个文件的原因在于，各种硬件平台的模型格式不同，无法直接加载，我们需要编写多种不同的代码来加载模型。而现在使用 MUD 文件，只需在其中写入模型信息，即可简化模型加载过程。

MUD 文件定义如下：

```ini
[basic]
type = cvimodel
model = model_path_relative_to_mud_file

[extra]
model_type = classifier
input_type = bgr
mean = 103.94, 116.78, 123.68
scale = 0.017, 0.017, 0.017
labels = labels.txt
```

`basic` 部分是必需的，`extra` 部分是可选的。
* `basic` 部分描述了模型的类型和模型路径。
  * `type` 表示模型类型，目前支持 `MaixCam` 的 `cvimodel` 类型。
  * `model` 表示模型的相对路径，相对于 MUD 文件所在位置。

* `extra` 部分描述了模型的额外信息，应用程序可以通过 `model.extra_info()` 方法获取。
  * `model_type` 表示模型的功能类型，如 `classifier`（分类器）和 `yolov2`（目标检测），此项为可选。
  * `input_type` 表示模型的输入类型，如 `bgr` 和 `gray`（灰度图），此项为可选。
  * `mean` 表示模型输入的均值，此项为可选。
  * `scale` 表示模型输入的缩放比例，此项为可选。
  * `labels` 表示模型标签文件的路径，此项为可选。

## 当前 MaixCDK 支持的模型类型：

* `classifier`
* `classifier_no_top`
* `yolo11`
* `yolov8`
* `yolov5`
* `pp_ocr`
* `retinaface`
* `nanotrack`
* `face_detector`
* `speech`
* 以及更多类型，详情请参考 [components/nn/include](https://github.com/sipeed/MaixCDK/tree/main/components/nn/include) 源代码，并搜索 `model_type` 关键字查看 `load` 方法。
