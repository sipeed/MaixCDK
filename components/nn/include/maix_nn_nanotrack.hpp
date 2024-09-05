/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.9.2: Create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <cmath>

namespace maix::nn
{
    /**
     * NanoTrack class
     * @maixpy maix.nn.NanoTrack
     */
    class NanoTrack
    {
    public:
        /**
         * Constructor of NanoTrack class
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.NanoTrack.__init__
         * @maixcdk maix.nn.NanoTrack.NanoTrack
         */
        NanoTrack(const string &model = "")
        {
            _model = nullptr;
            _model_target = nullptr;
            _model_head = nullptr;
            _dual_buff = false;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }

            // nanotrack params
            _context_amount = 0.5;
            _stride = 16;
            _head_out_size = 15;
            _penalty_k = 0.138;
            _window_influence = 0.455;
            _lr = 0.348;
            _generate_window(_head_out_size, _window);
            _points = _generate_points(_stride, _head_out_size);
            _bbox.resize(_head_out_size * _head_out_size);
            _penalty.resize(_head_out_size * _head_out_size);
        }

        ~NanoTrack()
        {
            _free_models();
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.NanoTrack.load
         */
        err::Err load(const string &model)
        {
            _free_models();
            _model = new nn::NN(model, _dual_buff);
            if (!_model)
            {
                return err::ERR_NO_MEM;
            }
            _extra_info = _model->extra_info();
            if (_extra_info.find("model_type") != _extra_info.end())
            {
                if (_extra_info["model_type"] != "nanotrack")
                {
                    log::error("model_type not match, expect 'nanotrack', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: nanotrack");
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
                log::error("mean use 0");
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
                log::error("scale use 1");
            }
            std::string model_dir = fs::abspath(fs::dirname(model));
            if (_extra_info.find("target_model") != _extra_info.end())
            {
                std::string model_target = _extra_info["target_model"];
                _model_target = new nn::NN(model_dir + "/" + model_target, _dual_buff);
                if (!_model_target)
                {
                    delete _model;
                    _model = nullptr;
                    return err::ERR_NO_MEM;
                }
            }
            else
            {
                log::error("target_model key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info.find("head_model") != _extra_info.end())
            {
                std::string model_head = _extra_info["head_model"];
                _model_head = new nn::NN(model_dir + "/" + model_head, _dual_buff);
                if (!_model_head)
                {
                    delete _model_target;
                    _model_target = nullptr;
                    delete _model;
                    _model = nullptr;
                    return err::ERR_NO_MEM;
                }
            }
            else
            {
                log::error("target_model key not found");
                return err::ERR_ARGS;
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            inputs = _model_target->inputs_info();
            _input_size_target = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print("\tinput size: %dx%d, target size: %dx%d\n\n", _input_size.width(), _input_size.height(), _input_size_target.width(), _input_size_target.height());
            return err::ERR_NONE;
        }

        /**
         * Init tracker, give tacker first target image and target position.
         * @param img Image want to detect, target should be in this image.
         * @param x the target position left top coordinate x.
         * @param y the target position left top coordinate y.
         * @param w the target width.
         * @param h the target height.
         * @throw If image format not match model input format, will throw err::Exception.
         * @maixpy maix.nn.NanoTrack.init
         */
        void init(image::Image &img, int x, int y, int w, int h)
        {
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            // crop image
            _get_avg_color(img, _input_img_fmt);
            int c_x = x + w / 2;
            int c_y = y + h / 2;
            float size_target = std::sqrt((w + _context_amount * (w + h)) * (h + _context_amount * (w + h)));
            int size_target_int = std::round(size_target);
            int crop_x_x1, crop_x_y1, crop_x_x2, crop_x_y2;
            image::Image *img_target = _padding_crop(img, c_x, c_y, size_target_int, crop_x_x1, crop_x_y1, crop_x_x2, crop_x_y2);

            // forward target feature
            tensor::Tensors *outputs;
            outputs = _model_target->forward_image(*img_target, this->mean, this->scale, image::FIT_FILL, false, true);
            delete img_target;
            _head_inputs.clear();
            for (auto it = outputs->begin(); it != outputs->end(); it++)
            {
                _head_inputs.add_tensor("input1", it->second, true, true);
                break;
            }
            _last_target.x = c_x; // center, not left-top
            _last_target.y = c_y; // center, not left-top
            _last_target.w = w;
            _last_target.h = h;
            delete outputs;
        }

        /**
         * Track object acoording to last object position and the init function learned target feature.
         * @param img image to detect object and track, can be any resolution, before detect it will crop a area according to last time target's position.
         * @param threshold If score < threshold, will see this new detection is invalid, but remain return this new detecion,  default 0.9.
         * @return object, position and score, and detect area in points's first 4 element(x, y, w, h, center_x, center_y, input_size, target_size)
         * @maixpy maix.nn.NanoTrack.track
        */
        nn::Object track(image::Image &img, float threshold = 0.9)
        {
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            // crop image
            float size_target = std::sqrt((_last_target.w + _context_amount * (_last_target.w + _last_target.h)) * (_last_target.h + _context_amount * (_last_target.w + _last_target.h)));
            int size_x = std::round(size_target * _input_size.width() / _input_size_target.width());
            float scale_z = _input_size_target.width() / size_target;
            int crop_x_x1, crop_x_y1, crop_x_x2, crop_x_y2;
            image::Image *img_target = _padding_crop(img, _last_target.x, _last_target.y, size_x, crop_x_x1, crop_x_y1, crop_x_x2, crop_x_y2);

            // forward target feature
            tensor::Tensors *outputs;
            outputs = _model->forward_image(*img_target, this->mean, this->scale, image::FIT_FILL, false, true);
            delete img_target;
            for (auto it = outputs->begin(); it != outputs->end(); it++)
            {
                _head_inputs.add_tensor("input2", it->second, false, false);
                break;
            }

            // forward head
            tensor::Tensors *outputs2  = NULL;
            tensor::Tensor *score_out  = NULL; // 1x2x15x15
            tensor::Tensor *bbox_out = NULL;  // 1x4x15x15
            outputs2 = _model_head->forward(_head_inputs, false, true);
            delete outputs;
            for (auto it = outputs2->begin(); it != outputs2->end(); it++)
            {
                if (it->second->shape()[1] == 2)
                    score_out = it->second;
                else if (it->second->shape()[1] == 4)
                    bbox_out = it->second;
            }
            if(!bbox_out || ! score_out)
            {
                delete outputs2;
                throw err::Exception(err::ERR_ARGS, "wrong model");
            }

            // decode score and bbox
            float *score = (float *)score_out->data();
            _softmax_2xn(score, _points.size());
            score += _points.size(); // we only use the target prob
            float *bbox = (float *)bbox_out->data();
            int stride1 = _points.size();
            int stride2 = stride1 * 2;
            int stride3 = stride1 * 3;
            float max_score = 0;
            int best_idx = 0;
            for (size_t i = 0; i < _points.size(); ++i)
            {
                nn::ObjectFloat &box = _bbox.at(i);
                box.x = _points[i][0] - bbox[i];
                box.y = _points[i][1] - bbox[i + stride1];
                box.w = _points[i][0] + bbox[i + stride2];
                box.h = _points[i][1] + bbox[i + stride3];
                // center bbox
                box.w = box.w - box.x;
                box.h = box.h - box.y;
                box.x += box.w / 2.0;
                box.y += box.h / 2.0;
                // penalty
                float s_c = _change(_sz(box.w, box.h) / _sz(_last_target.w * scale_z, _last_target.h * scale_z));
                float r_c = _change(((float)_last_target.w / _last_target.h) / (box.w / box.h));
                _penalty[i] = std::exp(-(r_c * s_c - 1) * _penalty_k);
                score[i] *= _penalty[i];
                score[i] = score[i] * (1 - _window_influence) + _window[i] * _window_influence;
                if (score[i] > max_score)
                {
                    max_score = score[i];
                    best_idx = i;
                }
            }
            nn::ObjectFloat &box = _bbox.at(best_idx);
            float temp = 1.0 / scale_z;
            box.x *= temp;
            box.y *= temp;
            box.w *= temp;
            box.h *= temp;
            int cx = box.x + _last_target.x;
            int cy = box.y + _last_target.y;
            // smooth
            float lr = _penalty[best_idx] * max_score * _lr;
            int w = _last_target.w * (1 - lr) + box.w * lr;
            int h = _last_target.h * (1 - lr) + box.h * lr;
            _bbox_clip(cx, cy, w, h, img.width(), img.height());

            // update status
            if(max_score >= threshold)
            {
                _last_target.x = cx;
                _last_target.y = cy;
                _last_target.w = w;
                _last_target.h = h;
            }

            // clear data
            _head_inputs.rm_tensor("input2");
            delete outputs2;
            nn::Object res(cx - w / 2, cy - h / 2, w, h, 0, max_score);
            res.points.resize(8);
            res.points.at(0) = crop_x_x1;
            res.points.at(1) = crop_x_y1;
            res.points.at(2) = crop_x_x2 - crop_x_x1;
            res.points.at(3) = crop_x_y2 - crop_x_y1;
            res.points.at(4) = cx;
            res.points.at(5) = cy;
            res.points.at(6) = size_x;
            res.points.at(7) = size_target;
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.NanoTrack.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.NanoTrack.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.NanoTrack.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.NanoTrack.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.NanoTrack.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.NanoTrack.scale
         */
        std::vector<float> scale;

    private:
        image::Size _input_size;
        image::Size _input_size_target;
        image::Format _input_img_fmt;
        nn::NN *_model;
        nn::NN *_model_target;
        nn::NN *_model_head;
        std::map<string, string> _extra_info;
        bool _dual_buff;
        float _context_amount;
        int _stride;
        int _head_out_size;
        float _penalty_k;
        float _window_influence;
        float _lr;
        uint8_t _avg_color[3];
        std::vector<std::vector<float>> _points;
        std::vector<nn::ObjectFloat> _bbox;
        tensor::Tensors _head_inputs;
        nn::Object _last_target;
        std::vector<float> _penalty;
        std::vector<float> _window;

    private:
        inline float _sz(float w, float h)
        {
            float pad = (w + h) * 0.5;
            return std::sqrt((w + pad) * (h + pad));
        }

        inline float _change(float r)
        {
            float t = 1.0 / r;
            return r > t ? r : t;
        }

        inline void _bbox_clip(int &cx, int &cy, int &w, int &h, int max_w, int max_h)
        {
            cx = max(0, min(cx, max_w - 1));
            cy = max(0, min(cy, max_h - 1));
            w = max(10, min(w, max_w));
            h = max(10, min(h, max_h));
        }

        void _free_models()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            if (_model_target)
            {
                delete _model_target;
                _model_target = nullptr;
            }
            if (_model_head)
            {
                delete _model_head;
                _model_head = nullptr;
            }
        }

        void _get_avg_color(image::Image &img, const image::Format &fmt)
        {
            uint8_t max_v_ch1 = 0, max_v_ch2 = 0, max_v_ch3 = 0;
            uint8_t min_v_ch1 = 255, min_v_ch2 = 255, min_v_ch3 = 255;
            uint8_t *p = (uint8_t *)img.data();
            for (int i = 0; i < img.data_size() / 3; ++i)
            {
                if (p[i * 3] > max_v_ch1)
                    max_v_ch1 = p[i * 3];
                else if (p[i * 3] < min_v_ch1)
                    min_v_ch1 = p[i * 3];
                if (p[i * 3 + 1] > max_v_ch2)
                    max_v_ch2 = p[i * 3 + 1];
                else if (p[i * 3 + 1] < min_v_ch2)
                    min_v_ch2 = p[i * 3 + 1];
                if (p[i * 3 + 2] > max_v_ch3)
                    max_v_ch3 = p[i * 3 + 2];
                else if (p[i * 3 + 2] < min_v_ch3)
                    min_v_ch3 = p[i * 3 + 2];
            }
            _avg_color[0] = (max_v_ch1 - min_v_ch1) / 2;
            _avg_color[1] = (max_v_ch2 - min_v_ch2) / 2;
            _avg_color[2] = (max_v_ch3 - min_v_ch3) / 2;
        }

        image::Image *_padding_crop(image::Image &img, int c_x, int c_y, int size_target, int &copy_x1, int &copy_y1, int &copy_x2, int &copy_y2)
        {
            int x1 = c_x - size_target / 2;
            int y1 = c_y - size_target / 2;
            int x2 = c_x + size_target / 2;
            int y2 = c_y + size_target / 2;
            // not pad
            if (x1 >= 0 && y1 >= 0 && x2 < img.width() && y2 < img.height())
            {
                copy_x1 = x1;
                copy_y1 = y1;
                copy_x2 = x2;
                copy_y2 = y2;
                return img.crop(x1, y1, size_target, size_target);
            }
            image::Image *res = new image::Image(size_target, size_target, img.format());
            if (!res)
                throw err::Exception(err::ERR_NO_MEM);
            copy_x1 = max(0, x1);
            copy_y1 = max(0, y1);
            copy_x2 = min(img.width(), x2);
            copy_y2 = min(img.height(), y2);
            uint8_t *data = (uint8_t *)res->data();
            uint8_t *src = (uint8_t *)img.data();
            // pad
            for (int y = 0; y < copy_y1 - y1; ++y)
            {
                for (int x = 0; x < x2 - x1; ++x)
                {
                    uint8_t *p = data + (y * res->width() + x) * 3;
                    *p = _avg_color[0];
                    *(p + 1) = _avg_color[1];
                    *(p + 2) = _avg_color[2];
                }
            }
            for (int y = copy_y2; y < y2; ++y)
            {
                for (int x = 0; x < x2 - x1; ++x)
                {
                    uint8_t *p = data + ((y +  - y1) * res->width() + x) * 3;
                    *p = _avg_color[0];
                    *(p + 1) = _avg_color[1];
                    *(p + 2) = _avg_color[2];
                }
            }
            for (int y = copy_y1; y < copy_y2; ++y)
            {
                for (int x = 0; x < copy_x1 - x1; ++x)
                {
                    uint8_t *p = data + ((y - y1) * res->width() + x) * 3;
                    *p = _avg_color[0];
                    *(p + 1) = _avg_color[1];
                    *(p + 2) = _avg_color[2];
                }
            }
            for (int y = copy_y1; y < copy_y2; ++y)
            {
                for (int x = copy_x2; x < x2; ++x)
                {
                    uint8_t *p = data + ((y - y1) * res->width() + x - x1) * 3;
                    *p = _avg_color[0];
                    *(p + 1) = _avg_color[1];
                    *(p + 2) = _avg_color[2];
                }
            }
            for (int y = copy_y1; y < copy_y2; ++y)
            {
                uint8_t *p = data + ((y - y1) * res->width() + (copy_x1 - x1)) * 3;
                uint8_t *s = src + (y * img.width() + copy_x1) * 3;
                int length = (copy_x2 - copy_x1) * 3;
                memcpy(p, s, length);
            }
            // res->save("/root/test.png");
            return res;
        }

        std::vector<std::vector<float>> _generate_points(int stride = 16, int size = 15)
        {
            // 计算原点位置
            float ori = -(size / 2) * stride;

            // 创建points向量
            std::vector<std::vector<float>> points(size * size, std::vector<float>(2, 0.0f));

            int index = 0;
            for (int dy = 0; dy < size; ++dy)
            {
                for (int dx = 0; dx < size; ++dx)
                {
                    float x = ori + stride * dx;
                    float y = ori + stride * dy;
                    points[index][0] = x;
                    points[index][1] = y;
                    ++index;
                }
            }

            return points;
        }

        std::vector<float> _generate_hanning_window(int size)
        {
            std::vector<float> hanning(size);
            for (int i = 0; i < size; ++i)
            {
                hanning[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
            }
            return hanning;
        }

        void _generate_window(int size, std::vector<float> &window)
        {
            std::vector<float> hanning = _generate_hanning_window(size);

            window.resize(size * size);
            for (int i = 0; i < size; ++i)
            {
                for (int j = 0; j < size; ++j)
                {
                    window[i * size + j] = hanning[i] * hanning[j];
                }
            }
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

        static void _softmax_2xn(float *data, int size)
        {
            float *p1 = data;
            float *p2 = data + size;
            float large;
            float sum;
            for(int i=0; i< size; ++i)
            {
                large = *p1 > *p2 ? *p1 : *p2;
                *p1 = expf(*p1 - large);
                *p2 = expf(*p2 - large);
                sum = *p1 + *p2;
                *p1 /= sum;
                *p2 /= sum;
                ++p1;
                ++p2;
            }
        }
    };

} // namespace maix::nn
