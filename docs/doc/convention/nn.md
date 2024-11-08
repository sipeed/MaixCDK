NN convention
======

## NN model file

Every AI model described by a `.mud` file, which is a INI format file.

MUD(Model Universal Description) file is a simple format to describe a model.
> The reason we create this file is that every hardware platform has its own model format, it's difficult to load them directly, and we have to write many different code to load them, now we use MUD file and wrote model info in it to make load model easier.

MUD file definition:

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

`basic` section is required, `extra` section is optional.
`basic` section describes model type and model path.
* `type` is model type, now we support `cvimodel` for `MaixCam`.
* `model` is model path relative to MUD file.

`extra` section describes model extra info, the application can get it by `model.extra_info()` method.
* `model_type` is model function type, like `classifier` and `yolov2`, it's optional for application.
* `input_type` is model input type, like `bgr` and `gray`, it's optional for application.
* `mean` is model input mean value, it's optional for application.
* `scale` is model input scale value, it's optional for application.
* `labels` is model labels file path, it's optional for application.


Current MaixCDK support:
* classifier
* classifier_no_top
* yolo11
* yolov8
* yolov5
* pp_ocr
* retinaface
* nanotrack
* retinaface
* face_detector
* speech
* and more ... please refer to [components/nn/include](https://github.com/sipeed/MaixCDK/tree/main/components/nn/include) souce code and search `model_type` keyword and see `load` method.

