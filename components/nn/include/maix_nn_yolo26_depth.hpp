/**
 * @author maixpy_skill
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2026.8.2: Add yolo26-depth support
 */

#pragma once
#include <cfloat>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_image_cmap.hpp"
#include "maix_time.hpp"

namespace maix::nn
{
    /**
     * YOLO26-depth
     * Monocular depth estimation model (yolo26n-depth etc.) inference wrapper.
     *
     * Usage:
     *     nn::Yolo26Depth model("/tmp/yolo26n-depth.mud");
     *     image::Image *img = cam.read();
     *     image::Image *heatmap = model.get_depth_image(*img, image::FIT_CONTAIN, image::CMap::JET);
     *     disp.show(*heatmap);
     *     delete heatmap;
     *
     * Note: the model MUD file's [extra] section should have model_type=yolo26_depth, input_type=rgb.
     * @maixpy maix.nn.Yolo26Depth
     */
    class Yolo26Depth
    {
    public:
        /**
         * Construct a new Yolo26Depth object
         * @param model MUD model path, if empty, will not load model, you can call load() later.
         *                  if not empty, will load model and will raise err::Exception if load failed.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @maixpy maix.nn.Yolo26Depth.__init__
         */
        Yolo26Depth(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            _input_img_fmt = image::Format::FMT_RGB888;
            _input_w = _input_h = 0;
            _output_w = _output_h = 0;
            _cmap = image::CMap::JET;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~Yolo26Depth()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
        }

        /**
         * Load model from file, model format is .mud,
         * MUD file should contain [extra] section, have key-values:
         * - model_type: yolo26_depth
         * - input_type: rgb or bgr
         * @param model MUD model path
         * @return error code, if load failed, return error code
         * @maixpy maix.nn.Yolo26Depth.load
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
            auto inputs = _model->inputs_info();
            if (inputs.empty())
            {
                return err::ERR_ARGS;
            }
            // NHWC: [1, H, W, 3];  NCHW: [1, 3, H, W]
            if (inputs[0].shape[3] <= 4)   // NHWC
            {
                _input_h = inputs[0].shape[1];
                _input_w = inputs[0].shape[2];
            }
            else
            {
                _input_h = inputs[0].shape[2];
                _input_w = inputs[0].shape[3];
            }
            // input_type decides input image format
            auto extra = _model->extra_info();
            auto it = extra.find("input_type");
            if (it != extra.end())
            {
                if (it->second == "bgr")
                    _input_img_fmt = image::Format::FMT_BGR888;
                else if (it->second == "gray")
                    _input_img_fmt = image::Format::FMT_GRAYSCALE;
                else
                    _input_img_fmt = image::Format::FMT_RGB888;
            }
            return err::ERR_NONE;
        }

        /**
         * Get model input size, only for image input
         * @return model input size
         * @maixpy maix.nn.Yolo26Depth.input_size
         */
        image::Size input_size()
        {
            return image::Size(_input_w, _input_h);
        }

        /**
         * Get model input width, only for image input
         * @return model input size of width
         * @maixpy maix.nn.Yolo26Depth.input_width
         */
        int input_width()
        {
            return _input_w;
        }

        /**
         * Get model input height, only for image input
         * @return model input size of height
         * @maixpy maix.nn.Yolo26Depth.input_height
         */
        int input_height()
        {
            return _input_h;
        }

        /**
         * Get input image format, only for image input
         * @return input image format, image::Format type.
         * @maixpy maix.nn.Yolo26Depth.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Get model output size (depth map size), only for image input
         * @return model output size
         * @maixpy maix.nn.Yolo26Depth.output_size
         */
        image::Size output_size()
        {
            return image::Size(_output_w, _output_h);
        }

        /**
         * Forward model and get raw image depth estimation data.
         * @param img image, format should match model input_type， or will raise err.Exception
         * @param fit image resize fit mode if input image not equal to model' input size,
         *            will auto resize to model's input size then detect, and recover to image input size.
         *            Default Fit.FIT_CONTAIN, see image.Fit.
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a tensor.Tensor object. If in dual_buff mode, value can be None(in Python) or nullptr(in C++) when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.Yolo26Depth.get_depth
         */
        tensor::Tensor *get_depth(image::Image &img, image::Fit fit = image::FIT_CONTAIN)
        {
            if (_model == nullptr) return nullptr;
            tensor::Tensors *outputs = _model->forward_image(img, {}, {}, fit, false, false);
            if (!outputs) return nullptr;
            tensor::Tensor *t = outputs->begin()->second;
            if (t->dtype() != tensor::DType::FLOAT32)
            {
                delete outputs;
                return nullptr;
            }
            _output_h = t->shape()[2];
            _output_w = t->shape()[3];
            tensor::Tensor *result = new tensor::Tensor(t->shape(), t->dtype(), t->data(), true);
            delete outputs;
            return result;
        }

        /**
         * Forward model and get image depth estimation data normlized to [0, 255] and as a image.Image object.
         * @param img image, format should match model input_type， or will raise err.Exception
         * @param fit image resize fit mode if input image not equal to model' input size,
         *            will auto resize to model's input size then detect, and recover to image input size.
         *            Default Fit.FIT_CONTAIN, see image.Fit.
         * @param cmap Color map used convert grayscale distance estimation image to RGB image.
         *             Diiferent cmap will influence finally image.
         *             Default image.CMap.JET (near red/yellow, far blue).
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a image::Image object. If in dual_buff mode, value can be None(in Python) or nullptr(in C++) when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.Yolo26Depth.get_depth_image
         */
        image::Image *get_depth_image(image::Image &img, image::Fit fit = image::FIT_CONTAIN,
                                      image::CMap cmap = image::CMap::JET)
        {
            if (_model == nullptr) return nullptr;
            _cmap = cmap;

            tensor::Tensors *outputs = _model->forward_image(img, {}, {}, fit, false, false);
            if (!outputs) return nullptr;
            tensor::Tensor *t = outputs->begin()->second;
            if (t->dtype() != tensor::DType::FLOAT32)
            {
                delete outputs;
                return nullptr;
            }
            int out_w = t->shape()[3];
            int out_h = t->shape()[2];
            _output_w = out_w;
            _output_h = out_h;
            const float *depth = (const float *)t->data();

            // compute letterbox content region (remove padding), keep depth pixel-aligned with image
            int top = 0, bottom = 0, left = 0, right = 0;
            if (fit == image::Fit::FIT_CONTAIN && (img.width() != out_w || img.height() != out_h))
            {
                float gain = std::min((float)out_h / img.height(), (float)out_w / img.width());
                int rw = (int)std::round(img.width() * gain);
                int rh = (int)std::round(img.height() * gain);
                int pw = out_w - rw;
                int ph = out_h - rh;
                left = (int)std::round(pw / 2.0f - 0.1f);
                right = (int)std::round(pw / 2.0f + 0.1f);
                top = (int)std::round(ph / 2.0f - 0.1f);
                bottom = (int)std::round(ph / 2.0f + 0.1f);
            }
            int crop_w = out_w - left - right;
            int crop_h = out_h - top - bottom;

            // extract content region depth (row-major)
            std::vector<float> crop(crop_w * crop_h);
            for (int y = 0; y < crop_h; y++)
                for (int x = 0; x < crop_w; x++)
                    crop[y * crop_w + x] = depth[(y + top) * out_w + (x + left)];

            // generate heatmap on crop size
            image::Image *heatmap = _colorize(crop.data(), crop_w, crop_h);

            // resize back to input image size
            if (crop_w != img.width() || crop_h != img.height())
            {
                image::Image *result = heatmap->resize(img.width(), img.height(), image::FIT_FILL);
                delete heatmap;
                delete outputs;
                return result;
            }
            delete outputs;
            return heatmap;
        }

    private:
        nn::NN *_model;
        bool _dual_buff;
        image::Format _input_img_fmt;
        int _input_w, _input_h;
        int _output_w, _output_h;
        image::CMap _cmap;

        /**
         * Convert depth map to RGB888 heatmap: disparity(1/depth) + min-max normalize + cmap lookup.
         * Return a new image::Image, caller should delete it.
         */
        image::Image *_colorize(const float *depth, int w, int h)
        {
            auto &colors = image::cmap_colors_rgb(_cmap);
            image::Image *result = new image::Image(w, h, image::Format::FMT_RGB888);
            uint8_t *img_data = (uint8_t *)result->data();
            int n = w * h;

            float min_v = FLT_MAX, max_v = -FLT_MAX;
            for (int i = 0; i < n; i++)
            {
                float d = depth[i];
                if (std::isfinite(d) && d > 0)
                {
                    float v = 1.0f / d;   // disparity
                    if (v < min_v) min_v = v;
                    if (v > max_v) max_v = v;
                }
            }
            if (min_v == max_v)
            {
                memset(img_data, 127, n * 3);
                return result;
            }
            float scale = 255.0f / (max_v - min_v);

#pragma omp parallel for
            for (int i = 0; i < n; i++)
            {
                float d = depth[i];
                if (std::isfinite(d) && d > 0)
                {
                    uint8_t gray = (uint8_t)std::clamp((1.0f / d - min_v) * scale, 0.0f, 255.0f);
                    const auto &rgb = colors[gray];
                    img_data[3 * i + 0] = rgb[0];
                    img_data[3 * i + 1] = rgb[1];
                    img_data[3 * i + 2] = rgb[2];
                }
                else
                {
                    img_data[3 * i + 0] = 0;
                    img_data[3 * i + 1] = 0;
                    img_data[3 * i + 2] = 0;
                }
            }
            return result;
        }
    };
}
