/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.5.15: Create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <tuple>
#include "libmaix_nn_decoder_retinaface.hpp"

namespace maix::nn
{
    /**
     * Retinaface class
     * @maixpy maix.nn.Retinaface
     */
    class Retinaface
    {
    public:
    public:
        /**
         * Constructor of Retinaface class
         * @param model model path, default empty, you can load model later by load function.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.Retinaface.__init__
         * @maixcdk maix.nn.Retinaface.Retinaface
         */
        Retinaface(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _priorboxes = nullptr;
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

        ~Retinaface()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            if(_priorboxes)
            {
                free(_priorboxes);
            }
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.Retinaface.load
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
                if (_extra_info["model_type"] != "retinaface")
                {
                    log::error("model_type not match, expect 'retinaface', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: retinaface");
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
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print("\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());

            // decoder params
            _config.variance[0] = 0.1;
            _config.variance[1] = 0.2;
            _config.nms = 0.2;
            _config.score_thresh = 0.5;
            _config.input_w = _input_size.width();
            _config.input_h = _input_size.height();
            _config.steps[0] = 8;
            _config.steps[1] = 16;
            _config.steps[2] = 32;
            _config.min_sizes[0] = 16;
            _config.min_sizes[1] = 32;
            _config.min_sizes[2] = 64;
            _config.min_sizes[3] = 128;
            _config.min_sizes[4] = 256;
            _config.min_sizes[5] = 512;

            _channel_num = retinaface_get_channel_num(&_config);
            _priorboxes = retinaface_get_priorboxes(&_config, &_channel_num);

            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Confidence threshold, default 0.4.
         * @param iou_th IoU threshold, default 0.45.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         * @maixpy maix.nn.Retinaface.detect
         */
        std::vector<nn::Object> *detect(image::Image &img, float conf_th = 0.4, float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN)
        {
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false);
            if (!outputs) // not ready for dual_buff mode.
            {
                return new std::vector<nn::Object>();
            }
            std::vector<nn::Object> *res = _post_process(outputs, img.width(), img.height(), fit);
            delete outputs;
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.Retinaface.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.Retinaface.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.Retinaface.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.Retinaface.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.Retinaface.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.Retinaface.scale
         */
        std::vector<float> scale;

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
        libmaix_nn_decoder_retinaface_config_t _config;
        nn::ObjectFloat *_priorboxes;
        int _channel_num;
        bool _dual_buff;

    private:
        std::vector<nn::Object> *_post_process(tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit)
        {
            std::vector<nn::Object> *objects = new std::vector<nn::Object>(_channel_num);
            tensor::Tensor *conf = nullptr;
            tensor::Tensor *loc = nullptr;
            tensor::Tensor *landms = nullptr;
            for(auto i : outputs->tensors)
            {
                if(i.second->shape()[2] == 2)
                    conf = i.second;
                else if(i.second->shape()[2] == 4)
                    loc = i.second;
                else if(i.second->shape()[2] == 10)
                    landms = i.second;
            }
            if(!conf || !loc || !landms)
                return nullptr;
            float *conf_data = (float *)conf->data();
            float *loc_data = (float *)loc->data();
            float *landms_data = (float *)landms->data();
            int valid_num = _channel_num;
            _config.nms = _iou_th;
            _config.score_thresh = _conf_th;
            retinaface_decode(loc_data, conf_data, landms_data, _priorboxes, objects, &valid_num, true, &_config);
            if (valid_num > 0)
            {
                std::vector<nn::Object> *objects_total = objects;
                objects = _nms(*objects, valid_num);
                delete objects_total;
            }
            else
            {
                delete objects;
                return new std::vector<nn::Object>();
            }
            _correct_bbox(*objects, img_w, img_h, fit);
            return objects;
        }

        std::vector<nn::Object> *_nms(std::vector<nn::Object> &objs, int num)
        {
            std::vector<nn::Object> *result = new std::vector<nn::Object>();
            std::sort(objs.begin(), objs.begin() + num, [](const nn::Object &a, const nn::Object &b)
                      { return a.score < b.score; });
            for (int i = 0; i < num; ++i)
            {
                nn::Object &a = objs.at(i);
                if (a.score == 0)
                    continue;
                for (int j = i + 1; j < num; ++j)
                {
                    nn::Object &b = objs.at(j);
                    if (b.score != 0 && a.class_id == b.class_id && _calc_iou(a, b) > this->_iou_th)
                    {
                        b.score = 0;
                    }
                }
            }
            for (int i=0; i<num; ++i)
            {
                nn::Object &a = objs.at(i);
                if (a.score != 0)
                    result->push_back(a);
            }
            return result;
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
                    for(size_t i=0; i<obj.points.size() / 2; ++i)
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
                    for(size_t i=0; i<obj.points.size() / 2; ++i)
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
                    for(size_t i=0; i<obj.points.size() / 2; ++i)
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
