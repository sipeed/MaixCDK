/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"

namespace maix::nn
{
    /**
     * YOLOv5 class
     * @maixpy maix.nn.YOLOv5
    */
    class YOLOv5
    {
    public:
        /**
         * Constructor of YOLOv5 class
         * @param model model path, default empty, you can load model later by load function.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.YOLOv5.__init__
         * @maixcdk maix.nn.YOLOv5.YOLOv5
        */
        YOLOv5(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~YOLOv5()
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
         * @maixpy maix.nn.YOLOv5.load
        */
        err::Err load(const string &model)
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            _model = new nn::NN(model, _dual_buff);
            if (!_model)
            {
                return err::ERR_NO_MEM;
            }
            _extra_info = _model->extra_info();
            if (_extra_info.find("model_type") != _extra_info.end())
            {
                if (_extra_info["model_type"] != "yolov5")
                {
                    log::error("model_type not match, expect 'yolov5', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: yolov5");
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
            err::Err e = _model->extra_info_labels(labels);
            if(e == err::Err::ERR_NONE)
            {
                log::print("\tlabels num: %ld\n", labels.size());
            }
            else
            {
                log::error("labels key not found: %s", err::to_str(e).c_str());
                return err::ERR_ARGS;
            }
            if (_extra_info.find("anchors") != _extra_info.end())
            {
                std::string &anchors_str = _extra_info["anchors"];
                std::vector<std::string> anchors_strs;
                split0(anchors_strs, anchors_str, ",");
                log::print("\tanchors:");
                for (auto &it : anchors_strs)
                {
                    try
                    {
                        this->anchors.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("anchors value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%.2f ", this->anchors.back());
                }
                log::print("\n");
                if (this->anchors.size() % 2 != 0)
                {
                    log::error("anchors value error, should even");
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("anchors key not found");
                return err::ERR_ARGS;
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print("\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Confidence threshold, default 0.5.
         * @param iou_th IoU threshold, default 0.45.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @param sort sort result according to object size, default 0 means not sort, 1 means bigger in front, -1 means smaller in front.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         * @maixpy maix.nn.YOLOv5.detect
        */
        std::vector<nn::Object> *detect(image::Image &img, float conf_th = 0.5, float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN, int sort = 0)
        {
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false);
            if (!outputs) // not ready, return empty result.
            {
                return new std::vector<nn::Object>();
            }
            std::vector<nn::Object> * res = _post_process(outputs, img.width(), img.height(), fit, sort);
            delete outputs;
            if(res == NULL)
            {
                throw err::Exception("post process failed, please see log before");
            }
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.YOLOv5.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.YOLOv5.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.YOLOv5.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLOv5.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLOv5.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.YOLOv5.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLOv5.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLOv5.scale
         */
        std::vector<float> scale;

        /**
         * Get anchors
         * @maixpy maix.nn.YOLOv5.anchors
         */
        std::vector<float> anchors;

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
        bool _dual_buff;

    private:
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

        std::vector<nn::Object> *_post_process(tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit, int sort)
        {
            std::vector<nn::Object> *objects = new std::vector<nn::Object>();
            int layer_num = outputs->size();
            int i = 0;
            for (auto it = outputs->begin(); it != outputs->end(); it++)
            {
                if(i == 0)
                {
                    std::vector<int> shape = it->second->shape();
                    if((size_t)shape[1] != (labels.size() + 5) * anchors.size() / 2 / layer_num)
                    {
                        log::error("mud labels or anchors not match model's");
                        delete objects;
                        return NULL;
                    }
                }
                // log::info("output: %s, tensor: %s", it->first.c_str(), it->second->to_str().c_str());
                _get_layer_objs(*objects, *it->second, i++, layer_num);
            }
            if(objects->size() > 0)
            {
                std::vector<nn::Object> *objects_total = objects;
                objects = _nms(*objects);
                delete objects_total;
                if(sort != 0)
                {
                    _sort_objects(*objects, sort);
                }
            }
            if(objects->size() > 0)
                _correct_bbox(*objects, img_w, img_h, fit);
            return objects;
        }

        void _get_layer_objs(std::vector<nn::Object> &objs, tensor::Tensor &output, int layer_i, int layer_num)
        {
            int h = output.shape()[2];
            int w = output.shape()[3];
            int class_num = this->labels.size();
            int box_len = class_num + 5;
            int s = w * h;
            int anchor_stride = box_len * s;
            int s4 = 4 * s;
            int s3 = 3 * s;
            int s2 = 2 * s;
            float *data = (float *)output.data();
            int anchor_num = this->anchors.size() / 2 / layer_num;
            int anchor_start = anchor_num * layer_i * 2;
            float scale_x = _input_size.width() / w;
            float scale_y = _input_size.height() / h;
            for (int a = 0; a < anchor_num; ++a)
            {
                for (int y = 0; y < h; ++y)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        float *p = data + a * anchor_stride + y * w + x + s4;
                        float obj_score = _sigmoid(*p);
                        if (obj_score <= _conf_th)
                            continue;
                        float *cls_scores = p + s;
                        int class_id = _argmax(cls_scores, class_num, s);
                        obj_score *= _sigmoid(cls_scores[class_id * s]);
                        if (obj_score <= _conf_th)
                            continue;
                        float bbox_x = (_sigmoid(*(p - s4)) * 2 + x - 0.5) * scale_x;
                        float bbox_y = (_sigmoid(*(p - s3)) * 2 + y - 0.5) * scale_y;
                        float bbox_w = pow(_sigmoid(*(p - s2)) * 2, 2) * this->anchors[anchor_start + a * 2];
                        float bbox_h = pow(_sigmoid(*(p - s)) * 2, 2) * this->anchors[anchor_start + a * 2 + 1];
                        bbox_x -= bbox_w * 0.5; // center x to left top x
                        bbox_y -= bbox_h * 0.5; // center y to left top y
                        Object obj(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score);
                        objs.push_back(obj);
                    }
                }
            }
        }

        std::vector<nn::Object> *_nms(std::vector<nn::Object> &objs)
        {
            std::vector<nn::Object> *result = new std::vector<nn::Object>();
            std::sort(objs.begin(), objs.end(), [](const nn::Object &a, const nn::Object &b) {
                return a.score > b.score;
            });
            for(size_t i=0; i < objs.size(); ++i)
            {
                nn::Object &a = objs.at(i);
                if(a.score == 0)
                    continue;
                for(size_t j = i + 1; j < objs.size(); ++j)
                {
                    nn::Object &b = objs.at(j);
                    if(b.score != 0 && a.class_id == b.class_id && _calc_iou(a, b) > this->_iou_th)
                    {
                        b.score = 0;
                    }
                }
            }
            for(nn::Object &a :objs)
            {
                if(a.score != 0)
                {
                    if (a.x < 0)
                    {
                        a.w += a.x;
                        a.x = 0;
                    }
                    if (a.y < 0)
                    {
                        a.h += a.y;
                        a.y = 0;
                    }
                    if (a.x + a.w > _input_size.width())
                    {
                        a.w = _input_size.width() - a.x;
                    }
                    if (a.y + a.h > _input_size.height())
                    {
                        a.h = _input_size.height() - a.y;
                    }
                    result->push_back(a);
                }
            }
            return result;
        }

        void _sort_objects(std::vector<nn::Object> &objects, int sort)
        {
            if (sort > 0)
                std::sort(objects.begin(), objects.end(), [](const nn::Object a, const nn::Object b)
                      { return (a.w * a.h) > (b.w * b.h); });
            else
                std::sort(objects.begin(), objects.end(), [](const nn::Object a, const nn::Object b)
                      { return (a.w * a.h) < (b.w * b.h); });
        }

        void _correct_bbox(std::vector<nn::Object> &objs, int img_w, int img_h, maix::image::Fit fit)
        {
#define CORRECT_BBOX_RANGE(obj)      \
    do                               \
    {                                \
        if (obj.x < 0)              \
        {                            \
            obj.w += obj.x;        \
            obj.x = 0;              \
        }                            \
        if (obj.y < 0)              \
        {                            \
            obj.h += obj.y;        \
            obj.y = 0;              \
        }                            \
        if (obj.x + obj.w > img_w) \
        {                            \
            obj.w = img_w - obj.x; \
        }                            \
        if (obj.y + obj.h > img_h) \
        {                            \
            obj.h = img_h - obj.y; \
        }                            \
    } while (0)

            if(img_w == _input_size.width() && img_h == _input_size.height())
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
                    CORRECT_BBOX_RANGE(obj);
                }
            }
            else if(fit == maix::image::FIT_CONTAIN)
            {
                float scale_x = ((float)_input_size.width()) / img_w ;
                float scale_y = ((float)_input_size.height()) / img_h ;
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
                    CORRECT_BBOX_RANGE(obj);
                }
            }
            else if(fit == maix::image::FIT_COVER)
            {
                float scale_x = ((float)_input_size.width()) / img_w ;
                float scale_y = ((float)_input_size.height()) / img_h ;
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
                    CORRECT_BBOX_RANGE(obj);
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

        static void split0(std::vector<std::string> &items, const std::string &s, const std::string &delimiter)
        {
            items.clear();
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
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
