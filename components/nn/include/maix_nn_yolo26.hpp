/**
 * @author Tao@sipeed, modified for YOLO26
 * @copyright Sipeed Ltd 2026-
 * @license Apache 2.0
 * @update 2026: Add YOLO26 support with NEON optimization.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"

#include <arm_neon.h>

namespace maix::nn
{
    /**
     * YOLO26 class for object detection
     * @maixpy maix.nn.YOLO26
     */
    class YOLO26
    {
    public:
        /**
         * Constructor of YOLO26 class
         * @param model model path, default empty, you can load model later by load function.
         * @param[in] dual_buff prepare dual input output buffer to accelerate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.YOLO26.__init__
         * @maixcdk maix.nn.YOLO26.YOLO26
         */
        YOLO26(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    char tmp[128] = {0};
                    snprintf(tmp, sizeof(tmp), "load model %s failed", model.c_str());
                    throw err::Exception(e, tmp);
                }
            }
        }

        /**
         * Destructor of YOLO26 class
         * @maixcdk maix.nn.YOLO26.~YOLO26
         */
        ~YOLO26()
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
         * @maixpy maix.nn.YOLO26.load
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
            
            // Check model type
            if (_extra_info.find("model_type") != _extra_info.end())
            {
                if (_extra_info["model_type"] != "yolo26")
                {
                    log::error("model_type not match, expect 'yolo26', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: yolo26");
            
            // Parse input type
            if (_extra_info.find("input_type") != _extra_info.end())
            {
                std::string input_type = _extra_info["input_type"];
                if (input_type == "rgb")
                {
                    _input_img_fmt = maix::image::FMT_RGB888;
                    log::print(log::LogLevel::LEVEL_INFO, "\tinput type: rgb\n");
                }
                else if (input_type == "bgr")
                {
                    _input_img_fmt = maix::image::FMT_BGR888;
                    log::print(log::LogLevel::LEVEL_INFO, "\tinput type: bgr\n");
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
            
            // Parse mean
            if (_extra_info.find("mean") != _extra_info.end())
            {
                std::string mean_str = _extra_info["mean"];
                std::vector<std::string> mean_strs = split(mean_str, ",");
                log::print(log::LogLevel::LEVEL_INFO, "\tmean:");
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
                    log::print(log::LogLevel::LEVEL_INFO, "%f ", this->mean.back());
                }
                log::print(log::LogLevel::LEVEL_INFO, "\n");
            }
            else
            {
                log::error("mean key not found");
                return err::ERR_ARGS;
            }
            
            // Parse scale
            if (_extra_info.find("scale") != _extra_info.end())
            {
                std::string scale_str = _extra_info["scale"];
                std::vector<std::string> scale_strs = split(scale_str, ",");
                log::print(log::LogLevel::LEVEL_INFO, "\tscale:");
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
                    log::print(log::LogLevel::LEVEL_INFO, "%f ", this->scale.back());
                }
                log::print(log::LogLevel::LEVEL_INFO, "\n");
            }
            else
            {
                log::error("scale key not found");
                return err::ERR_ARGS;
            }
            
            // Parse labels
            err::Err e = _model->extra_info_labels(labels);
            if (e == err::Err::ERR_NONE)
            {
                log::print(log::LogLevel::LEVEL_INFO, "\tlabels num: %ld\n", labels.size());
            }
            else
            {
                log::error("labels key not found: %s", err::to_str(e).c_str());
                return err::ERR_ARGS;
            }
            
            // Get input size from model
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            if (inputs[0].shape[3] <= 4) // nhwc
                _input_size = image::Size(inputs[0].shape[2], inputs[0].shape[1]);
            else
                _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print(log::LogLevel::LEVEL_INFO, "\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());
            
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Confidence threshold, default 0.5.
         * @param iou_th IoU threshold, default 0.45.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @param sort Sort result according to object size, default 0 means not sort, 1 means bigger in front, -1 means smaller in front.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         * @maixpy maix.nn.YOLO26.detect
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
            if (!outputs) // not ready, return empty result
            {
                return new std::vector<nn::Object>();
            }
            
            std::vector<nn::Object> *res = _post_process(outputs, img.width(), img.height(), fit, sort);
            delete outputs;
            
            if (res == NULL)
            {
                throw err::Exception("post process failed, please see log before");
            }
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.YOLO26.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.YOLO26.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.YOLO26.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLO26.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLO26.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.YOLO26.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLO26.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLO26.scale
         */
        std::vector<float> scale;

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
        bool _dual_buff;
        
        // YOLO26 specific constants
        static constexpr float LOGIT_THRESHOLD = -0.2f;

    private:
        /**
         * Post process for YOLO26 output
         */
        std::vector<nn::Object> *_post_process(tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit, int sort)
        {
            std::vector<nn::Object> *objects = new std::vector<nn::Object>();
            int num_class = labels.size();
            
            // Get output tensors - YOLO26 has 6 outputs: 3 bbox + 3 cls
            float *bbox[3] = {
                (float *)(*outputs)["output0"].data(),  // 1*80*80*4
                (float *)(*outputs)["588"].data(),      // 1*40*40*4
                (float *)(*outputs)["610"].data()       // 1*20*20*4
            };
            float *cls[3] = {
                (float *)(*outputs)["580"].data(),      // 1*80*80*num_class
                (float *)(*outputs)["602"].data(),      // 1*40*40*num_class
                (float *)(*outputs)["624"].data()       // 1*20*20*num_class
            };
            
            // Generate proposals for each scale
            _generate_proposals(8, 80, 80, bbox[0], cls[0], num_class, *objects);
            _generate_proposals(16, 40, 40, bbox[1], cls[1], num_class, *objects);
            _generate_proposals(32, 20, 20, bbox[2], cls[2], num_class, *objects);
            
            // NMS
            if (objects->size() > 0)
            {
                std::vector<nn::Object> *objects_total = objects;
                objects = _nms(*objects);
                delete objects_total;
                
                if (sort != 0)
                {
                    _sort_objects(*objects, sort);
                }
            }
            
            // Correct bbox to original image size
            if (objects->size() > 0)
            {
                _correct_bbox(*objects, img_w, img_h, fit);
            }
            
            return objects;
        }

        /**
         * Generate proposals using NEON SIMD optimization
         */
        void _generate_proposals(int stride, int fw, int fh,
                                  const float *__restrict__ bbox,
                                  const float *__restrict__ cls,
                                  int num_class,
                                  std::vector<nn::Object> &objs)
        {
            const int total = fw * fh;
            const float stride_f = (float)stride;
            
            for (int i = 0; i < total; i++)
            {
                const float *c = cls + i * num_class;
                
                // Prefetch next data
                if (i + 4 < total)
                {
                    __builtin_prefetch(cls + (i + 4) * num_class, 0, 1);
                }
                
                // Find max logit using NEON
                float max_logit = _find_max_neon(c, num_class);
                
                // Early exit
                if (max_logit < LOGIT_THRESHOLD)
                    continue;
                
                // Find class id
                int class_id = 0;
                float max_val = c[0];
                for (int j = 1; j < num_class; j++)
                {
                    if (c[j] > max_val)
                    {
                        max_val = c[j];
                        class_id = j;
                    }
                }
                
                // Sigmoid and threshold
                float score = _sigmoid(max_logit);
                if (score <= _conf_th)
                    continue;
                
                // Calculate bbox
                int ax = i % fw;
                int ay = i / fw;
                const float *b = bbox + i * 4;
                
                float cx = (ax + 0.5f) * stride_f;
                float cy = (ay + 0.5f) * stride_f;
                
                float x = cx - b[0] * stride_f;
                float y = cy - b[1] * stride_f;
                float w = (b[0] + b[2]) * stride_f;
                float h = (b[1] + b[3]) * stride_f;
                
                // Clamp to input size
                if (x < 0)
                {
                    w += x;
                    x = 0;
                }
                if (y < 0)
                {
                    h += y;
                    y = 0;
                }
                if (x + w > _input_size.width())
                {
                    w = _input_size.width() - x;
                }
                if (y + h > _input_size.height())
                {
                    h = _input_size.height() - y;
                }
                
                if (w > 0 && h > 0)
                {
                    Object obj(x, y, w, h, class_id, score);
                    objs.push_back(obj);
                }
            }
        }

        /**
         * Find max value using NEON SIMD
         */
        inline float _find_max_neon(const float *data, int count)
        {
            if (count <= 0)
                return -1e9f;
            
            int vec_count = count / 16 * 16;
            float32x4_t vmax0 = vdupq_n_f32(-1e9f);
            float32x4_t vmax1 = vdupq_n_f32(-1e9f);
            float32x4_t vmax2 = vdupq_n_f32(-1e9f);
            float32x4_t vmax3 = vdupq_n_f32(-1e9f);
            
            for (int j = 0; j < vec_count; j += 16)
            {
                vmax0 = vmaxq_f32(vmax0, vld1q_f32(data + j));
                vmax1 = vmaxq_f32(vmax1, vld1q_f32(data + j + 4));
                vmax2 = vmaxq_f32(vmax2, vld1q_f32(data + j + 8));
                vmax3 = vmaxq_f32(vmax3, vld1q_f32(data + j + 12));
            }
            
            // Merge
            vmax0 = vmaxq_f32(vmax0, vmax1);
            vmax2 = vmaxq_f32(vmax2, vmax3);
            vmax0 = vmaxq_f32(vmax0, vmax2);
            
            // Horizontal max
            float32x2_t vmax_pair = vpmax_f32(vget_low_f32(vmax0), vget_high_f32(vmax0));
            vmax_pair = vpmax_f32(vmax_pair, vmax_pair);
            float max_val = vget_lane_f32(vmax_pair, 0);
            
            // Handle remaining elements
            for (int j = vec_count; j < count; j++)
            {
                if (data[j] > max_val)
                    max_val = data[j];
            }
            
            return max_val;
        }

        /**
         * NMS (Non-Maximum Suppression)
         */
        std::vector<nn::Object> *_nms(std::vector<nn::Object> &objs)
        {
            std::vector<nn::Object> *result = new std::vector<nn::Object>();
            
            // Sort by score
            std::sort(objs.begin(), objs.end(), [](const nn::Object &a, const nn::Object &b)
                      { return a.score > b.score; });
            
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
                {
                    result->push_back(a);
                }
            }
            
            return result;
        }

        /**
         * Sort objects by size
         */
        void _sort_objects(std::vector<nn::Object> &objects, int sort)
        {
            if (sort > 0)
                std::sort(objects.begin(), objects.end(), [](const nn::Object a, const nn::Object b)
                          { return (a.w * a.h) > (b.w * b.h); });
            else
                std::sort(objects.begin(), objects.end(), [](const nn::Object a, const nn::Object b)
                          { return (a.w * a.h) < (b.w * b.h); });
        }

        /**
         * Correct bbox to original image size
         */
        void _correct_bbox(std::vector<nn::Object> &objs, int img_w, int img_h, maix::image::Fit fit)
        {
#define CORRECT_BBOX_RANGE(obj)        \
    do                                 \
    {                                  \
        if (obj.x < 0)                 \
        {                              \
            obj.w += obj.x;            \
            obj.x = 0;                 \
        }                              \
        if (obj.y < 0)                 \
        {                              \
            obj.h += obj.y;            \
            obj.y = 0;                 \
        }                              \
        if (obj.x + obj.w > img_w)     \
        {                              \
            obj.w = img_w - obj.x;     \
        }                              \
        if (obj.y + obj.h > img_h)     \
        {                              \
            obj.h = img_h - obj.y;     \
        }                              \
    } while (0)

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
                    CORRECT_BBOX_RANGE(obj);
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
                    CORRECT_BBOX_RANGE(obj);
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
                    CORRECT_BBOX_RANGE(obj);
                }
            }
            else
            {
                throw err::Exception(err::ERR_ARGS, "fit type not support");
            }
#undef CORRECT_BBOX_RANGE
        }

        /**
         * Sigmoid function
         */
        inline static float _sigmoid(float x)
        {
            return 1.0f / (1.0f + expf(-x));
        }

        /**
         * Calculate IoU (Intersection over Union)
         */
        inline static float _calc_iou(Object &a, Object &b)
        {
            float area1 = a.w * a.h;
            float area2 = b.w * b.h;
            float wi = std::min((a.x + a.w), (b.x + b.w)) - std::max(a.x, b.x);
            float hi = std::min((a.y + a.h), (b.y + b.h)) - std::max(a.y, b.y);
            float area_i = std::max(wi, 0.0f) * std::max(hi, 0.0f);
            return area_i / (area1 + area2 - area_i);
        }

        /**
         * Split string by delimiter
         */
        static std::vector<std::string> split(const std::string &s, const std::string &delimiter)
        {
            std::vector<std::string> tokens;
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                token.erase(0, token.find_first_not_of(" \t\r\n"));
                token.erase(token.find_last_not_of(" \t\r\n") + 1);
                tokens.push_back(token);
            }
            token = s.substr(pos_start);
            token.erase(0, token.find_first_not_of(" \t\r\n"));
            token.erase(token.find_last_not_of(" \t\r\n") + 1);
            tokens.push_back(token);
            
            return tokens;
        }
    };

} // namespace maix::nn