/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.6.7: Add yolov8 support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"

namespace maix::nn
{
    class _KpInfo
    {
    public:
        _KpInfo(int idx, int ax, int ay, float stride)
            : idx(idx), anchor_x(ax), anchor_y(ay), stride(stride)
        {
        }
        int idx;
        int anchor_x;
        int anchor_y;
        float stride;
    };

    /**
     * YOLOv8 class
     * @maixpy maix.nn.YOLOv8
     */
    class YOLOv8
    {
    public:
    public:
        /**
         * Constructor of YOLOv8 class
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.YOLOv8.__init__
         * @maixcdk maix.nn.YOLOv8.YOLOv8
         */
        YOLOv8(const string &model = "")
        {
            _model = nullptr;
            _type_pose = false;
            _type = "detector";
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~YOLOv8()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.YOLOv8.load
         */
        err::Err load(const string &model)
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            _model = new nn::NN(model);
            if (!_model)
            {
                return err::ERR_NO_MEM;
            }
            _extra_info = _model->extra_info();
            if (_extra_info.find("model_type") != _extra_info.end())
            {
                if (_extra_info["model_type"] != "yolov8")
                {
                    log::error("model_type not match, expect 'yolov8', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: yolov8");
            if (_extra_info.find("input_type") != _extra_info.end())
            {
                std::string input_type = _extra_info["input_type"];
                if (input_type == "rgb")
                {
                    _input_img_fmt = maix::image::FMT_RGB888;
                    log::print("\tinput type: rgb\n");
                }
                else if (input_type == "bgr")
                {
                    _input_img_fmt = maix::image::FMT_BGR888;
                    log::print("\tinput type: bgr\n");
                }
                else
                {
                    log::error("unknown input type: %s", input_type.c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("input_type key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info.find("mean") != _extra_info.end())
            {
                std::string mean_str = _extra_info["mean"];
                std::vector<std::string> mean_strs = split(mean_str, ",");
                log::print("\tmean:");
                for (auto &it : mean_strs)
                {
                    try
                    {
                        this->mean.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("mean value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%f ", this->mean.back());
                }
                log::print("\n");
            }
            else
            {
                log::error("mean key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info.find("scale") != _extra_info.end())
            {
                std::string scale_str = _extra_info["scale"];
                std::vector<std::string> scale_strs = split(scale_str, ",");
                log::print("\tscale:");
                for (auto &it : scale_strs)
                {
                    try
                    {
                        this->scale.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("scale value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%f ", this->scale.back());
                }
                log::print("\n");
            }
            else
            {
                log::error("scale key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info.find("labels") != _extra_info.end())
            {
                // if "," in labels, will treat as label list, else will treat as label file path
                std::string &labels_str = _extra_info["labels"];
                if (labels_str.find(",") != std::string::npos)
                {
                    split0(labels, labels_str, ",");
                }
                else if (labels_str.find(".") != std::string::npos)
                {
                    label_path = fs::dirname(model) + "/" + _extra_info["labels"];
                    err::Err e = _load_labels_from_file(labels, label_path);
                    if (e != err::ERR_NONE)
                    {
                        log::error("Load labels file %s failed", label_path.c_str());
                        return e;
                    }
                }
                else
                {
                    labels.clear();
                    labels.push_back(labels_str);
                }
                log::print("\tlabels num: %ld\n", labels.size());
            }
            else
            {
                log::error("labels key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info.find("type") != _extra_info.end())
            {
                _type = _extra_info["type"];
                if (_type == "pose")
                {
                    _type_pose = true;
                }
                else if (_type != "detector")
                {
                    log::error("type [%s] not support, suport detector and pose", _type.c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                _type = "detector";
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print("\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());
            return err::ERR_NONE;
        }

        err::Err _load_labels_from_file(std::vector<std::string> &labels, const std::string &label_path)
        {
            // load labels from labels file
            labels.clear();
            fs::File *f = fs::open(label_path, "r");
            if (!f)
            {
                log::error("open label file %s failed", label_path.c_str());
                return err::ERR_ARGS;
            }
            std::string line;
            while (f->readline(line) > 0)
            {
                // strip line
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                labels.push_back(line);
            }
            f->close();
            delete f;
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Confidence threshold, default 0.5.
         * @param iou_th IoU threshold, default 0.45.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @param keypoint_th keypoint threshold, default 0.5, only for yolov8-pose model.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         *         If model is yolov8-pose, object's points have value, and if points' value < 0 means that point is invalid(conf < keypoint_th).
         * @maixpy maix.nn.YOLOv8.detect
         */
        std::vector<nn::Object> *detect(image::Image &img, float conf_th = 0.5, float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN, float keypoint_th = 0.5)
        {
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            this->_keypoint_th = keypoint_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false);
            if (!outputs)
            {
                throw err::Exception("forward image failed");
            }
            std::vector<nn::Object> *res = _post_process(outputs, img.width(), img.height(), fit);
            delete outputs;
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.YOLOv8.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.YOLOv8.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.YOLOv8.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLOv8.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Draw pose keypoints on image
         * @param img image object, maix.image.Image type.
         * @param points keypoits, int list type, [x, y, x, y ...]
         * @param radius radius of points.
         * @param color color of points.
         * @param body true, if points' length is 17*2 and body is ture, will draw lines as human body, if set to false won't draw lines, default true.
         * @maixpy maix.nn.YOLOv8.draw_pose
         */
        void draw_pose(image::Image &img, std::vector<int> points, int radius = 4, image::Color color = image::COLOR_RED, bool body=true)
        {
            if (points.size() < 2 || points.size() % 2 != 0)
            {
                throw std::runtime_error("keypoints size must >= 2 and multiple of 2");
                return;
            }
            if(points.size() == 17 * 2 && body)
            {
                int pos[] = {9, 7, 7, 5, 6, 8, 8, 10, 5, 11, 6, 12, 5, 6, 11, 12, 11, 13, 15, 13, 14, 12, 14, 16};
                for (int i = 0; i < 12; ++i)
                {
                    int x1 = points[pos[i * 2] * 2];
                    int y1 = points[pos[i * 2] * 2 + 1];
                    int x2 = points[pos[i * 2 + 1] * 2];
                    int y2 = points[pos[i * 2 + 1] * 2 + 1];
                    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
                        continue;
                    img.draw_line(x1, y1, x2, y2, image::COLOR_RED, 2);
                }
                int x = (points[5 * 2] + points[6 * 2]) / 2;
                int y = (points[5 * 2 + 1] + points[6 * 2 + 1]) / 2;
                if (!(points[5 * 2] < 0 || points[5 * 2 + 1] < 0 || points[6 * 2] < 0 || points[6 * 2 + 1] < 0 || x < 0 || y < 0 || points[0] < 0 || points[1] < 0))
                    img.draw_line(points[0], points[1], x, y, image::COLOR_RED, 2);
            }
            for (size_t i = 0; i < points.size() / 2; ++i)
            {
                int x = points[i * 2];
                int y = points[i * 2 + 1];
                if (x < 0 || y < 0)
                    continue;
                img.draw_circle(x, y, radius, color, -1);
            }
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLOv8.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.YOLOv8.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLOv8.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLOv8.scale
         */
        std::vector<float> scale;

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
        float _keypoint_th = 0.5;
        std::string _type;
        bool _type_pose;

    private:
        std::vector<nn::Object> *_post_process(tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit)
        {
            std::vector<nn::Object> *objects = new std::vector<nn::Object>();
            tensor::Tensor *kp_out = _decode_objs(*objects, outputs, _conf_th, _input_size.width(), _input_size.height());
            if (objects->size() > 0)
            {
                std::vector<nn::Object> *objects_total = objects;
                objects = _nms(*objects);
                delete objects_total;
            }
            // decode keypoints
            if (_type_pose)
            {
                _decode_keypoints(*objects, kp_out);
            }
            if (objects->size() > 0)
                _correct_bbox(*objects, img_w, img_h, fit);
            return objects;
        }

        tensor::Tensor *_decode_objs(std::vector<nn::Object> &objs, tensor::Tensors *outputs, float conf_thresh, int w, int h)
        {
            float stride[3] = {8, 16, 32};
            tensor::Tensor *score_out = NULL; // shape 1, 80, 8400, 1
            tensor::Tensor *box_out = NULL;   // shape 1,  1,    4, 8400
            tensor::Tensor *kp_out = NULL;    // shape 1, 51, 8400, 1
            for (auto i : *outputs)
            {
                if (i.second->shape()[2] == 4)
                {
                    box_out = i.second;
                }
                else if(strstr(i.first.c_str(), "Sigmoid") > 0)
                {
                    score_out = i.second;
                }
                else
                {
                    kp_out = i.second;
                }
            }
            if(!score_out || !kp_out)
            {
                throw err::Exception(err::ERR_ARGS, "model output not valid");
            }
            int total_box_num = box_out->shape()[3];
            // int class_num = this->labels.size();
            int class_num = score_out->shape()[1];
            float *scores_ptr = (float *)score_out->data();
            float *dets_ptr = (float *)box_out->data();
            int idx_start[3] = {
                0,
                (int)(h / stride[0] * w / stride[0]),
                (int)(h / stride[0] * w / stride[0] + h / stride[1] * w / stride[1])};
            for (int i = 0; i < 3; i++)
            {
                int nh = h / stride[i];
                int nw = w / stride[i];
                for (int ay = 0; ay < nh; ++ay)
                {
                    for (int ax = 0; ax < nw; ++ax)
                    {
                        int offset = idx_start[i] + ay * nw + ax;
                        int class_id = _argmax(scores_ptr + offset, class_num, total_box_num);
                        // int max_idx = _argmax2(scores_ptr + offset, class_num * total_box_num - offset, total_box_num);
                        // int class_id = (offset + max_idx) / total_box_num;
                        float obj_score = scores_ptr[offset + class_id * total_box_num];
                        if (obj_score <= conf_thresh)
                        {
                            continue;
                        }
                        float bbox_x = (ax + 0.5 - dets_ptr[offset]) * stride[i];
                        float bbox_y = (ay + 0.5 - dets_ptr[offset + total_box_num]) * stride[i];
                        float bbox_w = (ax + 0.5 + dets_ptr[offset + total_box_num * 2]) * stride[i] - bbox_x;
                        float bbox_h = (ay + 0.5 + dets_ptr[offset + total_box_num * 3]) * stride[i] - bbox_y;
                        Object obj(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score);
                        _KpInfo *kp_info = new _KpInfo(offset, ax, ay, stride[i]);
                        obj.temp = (void *)kp_info;
                        objs.push_back(obj);
                    }
                }
            }
            return kp_out;
        }

        std::vector<nn::Object> *_nms(std::vector<nn::Object> &objs)
        {
            std::vector<nn::Object> *result = new std::vector<nn::Object>();
            std::sort(objs.begin(), objs.end(), [](const nn::Object &a, const nn::Object &b)
                      { return a.score < b.score; });
            for (size_t i = 0; i < objs.size(); ++i)
            {
                nn::Object &a = objs.at(i);
                if (a.score == 0)
                    continue;
                for (size_t j = i + 1; j < objs.size(); ++j)
                {
                    nn::Object &b = objs.at(j);
                    if (b.score != 0 && a.class_id == b.class_id && _calc_iou(a, b) > this->_iou_th)
                    {
                        b.score = 0;
                    }
                }
            }
            for (nn::Object &a : objs)
            {
                if (a.score != 0)
                    result->push_back(a);
                else
                {
                    delete (_KpInfo *)a.temp;
                    a.temp = NULL;
                }
            }
            return result;
        }

        void _decode_keypoints(std::vector<nn::Object> &objs, tensor::Tensor *kp_out)
        {
            float *data = (float *)kp_out->data();
            int keypoint_num = kp_out->shape()[1] / 3; // 1, 51, 8400, 1
            int total_box_num = kp_out->shape()[2]; // 1, 51, 8400, 1
            for (size_t i = 0; i < objs.size(); ++i)
            {
                nn::Object &o = objs.at(i);
                _KpInfo *kp_info = (_KpInfo *)o.temp;
                float *p = data + kp_info->idx;
                for (int k = 0; k < keypoint_num; ++k)
                {
                    float score = _sigmoid(p[(k * 3 + 2) * total_box_num]);
                    int x = -1;
                    int y = -1;
                    if (score > _keypoint_th)
                    {
                        x = (p[(k * 3) * total_box_num] * 2.0 + kp_info->anchor_x) * kp_info->stride;
                        y = (p[(k * 3 + 1) * total_box_num] * 2.0 + kp_info->anchor_y) * kp_info->stride;
                    }
                    o.points.push_back(x);
                    o.points.push_back(y);
                }
                delete (_KpInfo *)o.temp;
                o.temp = NULL;
            }
        }

        void _correct_bbox(std::vector<nn::Object> &objs, int img_w, int img_h, maix::image::Fit fit)
        {
            if (img_w == _input_size.width() && img_h == _input_size.height())
                return;
            if (fit == maix::image::FIT_FILL)
            {
                float scale_x = (float)img_w / _input_size.width();
                float scale_y = (float)img_h / _input_size.height();
                for (nn::Object &obj : objs)
                {
                    obj.x *= scale_x;
                    obj.y *= scale_y;
                    obj.w *= scale_x;
                    obj.h *= scale_y;
                    for (size_t i = 0; i < obj.points.size() / 2; ++i)
                    {
                        obj.points.at(i * 2) *= scale_x;
                        obj.points.at(i * 2 + 1) *= scale_y;
                    }
                }
            }
            else if (fit == maix::image::FIT_CONTAIN)
            {
                float scale_x = ((float)_input_size.width()) / img_w;
                float scale_y = ((float)_input_size.height()) / img_h;
                float scale = std::min(scale_x, scale_y);
                float scale_reverse = 1.0 / scale;
                float pad_w = (_input_size.width() - img_w * scale) / 2.0;
                float pad_h = (_input_size.height() - img_h * scale) / 2.0;
                for (nn::Object &obj : objs)
                {
                    obj.x = (obj.x - pad_w) * scale_reverse;
                    obj.y = (obj.y - pad_h) * scale_reverse;
                    obj.w *= scale_reverse;
                    obj.h *= scale_reverse;
                    for (size_t i = 0; i < obj.points.size() / 2; ++i)
                    {
                        obj.points.at(i * 2) = (obj.points.at(i * 2) - pad_w) * scale_reverse;
                        obj.points.at(i * 2 + 1) = (obj.points.at(i * 2 + 1) - pad_h) * scale_reverse;
                    }
                }
            }
            else if (fit == maix::image::FIT_COVER)
            {
                float scale_x = ((float)_input_size.width()) / img_w;
                float scale_y = ((float)_input_size.height()) / img_h;
                float scale = std::max(scale_x, scale_y);
                float scale_reverse = 1.0 / scale;
                float pad_w = (img_w * scale - _input_size.width()) / 2.0;
                float pad_h = (img_h * scale - _input_size.height()) / 2.0;
                for (nn::Object &obj : objs)
                {
                    obj.x = (obj.x + pad_w) * scale_reverse;
                    obj.y = (obj.y + pad_h) * scale_reverse;
                    obj.w *= scale_reverse;
                    obj.h *= scale_reverse;
                    for (size_t i = 0; i < obj.points.size() / 2; ++i)
                    {
                        obj.points.at(i * 2) = (obj.points.at(i * 2) - pad_w) * scale_reverse;
                        obj.points.at(i * 2 + 1) = (obj.points.at(i * 2 + 1) - pad_h) * scale_reverse;
                    }
                }
            }
            else
            {
                throw err::Exception(err::ERR_ARGS, "fit type not support");
            }
        }

        inline static float _sigmoid(float x) { return 1.0 / (1 + expf(-x)); }

        inline static float _calc_iou(Object &a, Object &b)
        {
            float area1 = a.w * a.h;
            float area2 = b.w * b.h;
            float wi = std::min((a.x + a.w), (b.x + b.w)) -
                       std::max(a.x, b.x);
            float hi = std::min((a.y + a.h), (b.y + b.h)) -
                       std::max(a.y, b.y);
            float area_i = std::max(wi, 0.0f) * std::max(hi, 0.0f);
            return area_i / (area1 + area2 - area_i);
        }

        template <typename T>
        static int _argmax(const T *data, size_t len, size_t stride = 1)
        {
            int maxIndex = 0;
            for (size_t i = 1; i < len; i++)
            {
                int idx = i * stride;
                if (data[maxIndex * stride] < data[idx])
                {
                    maxIndex = i;
                }
            }
            return maxIndex;
        }

        template <typename T>
        static int _argmax2(const T *data, size_t end, size_t stride = 1)
        {
            int maxIndex = 0;
            for (size_t i = stride; i < end; i += stride)
            {
                if (data[maxIndex] < data[i])
                {
                    maxIndex = i;
                }
            }
            return maxIndex;
        }

        static void split0(std::vector<std::string> &items, const std::string &s, const std::string &delimiter)
        {
            items.clear();
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                items.push_back(token);
            }

            items.push_back(s.substr(pos_start));
        }

        static std::vector<std::string> split(const std::string &s, const std::string &delimiter)
        {
            std::vector<std::string> tokens;
            split0(tokens, s, delimiter);
            return tokens;
        }
    };

} // namespace maix::nn
