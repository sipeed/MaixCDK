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
#include <limits>
#include <memory>
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
     *     nn::YOLO26Depth model("/tmp/yolo26n-depth.mud");
     *     image::Image *img = cam.read();
     *     tensor::Tensor *depth = model.get_depth(*img, image::FIT_CONTAIN);
     *     image::Image *heatmap = model.depth_to_image(*depth, img->width(), img->height());
     *     float meters = model.get_distance(*depth, img->width() / 2, img->height() / 2,
     *                                       img->width(), img->height());
     *     disp.show(*heatmap);
     *     delete heatmap;
     *     delete depth;
     *     delete img;
     *
     * Note: the model MUD file's [extra] section should have model_type=yolo26_depth, input_type=rgb.
     * @maixpy maix.nn.YOLO26Depth
     */
    class YOLO26Depth
    {
    public:
        /**
         * Construct a new YOLO26Depth object
         * @param model MUD model path, if empty, will not load model, you can call load() later.
         *                  if not empty, will load model and will raise err::Exception if load failed.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @maixpy maix.nn.YOLO26Depth.__init__
         */
        YOLO26Depth(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            _input_img_fmt = image::Format::FMT_RGB888;
            _input_w = _input_h = 0;
            _output_w = _output_h = 0;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~YOLO26Depth()
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
         * @maixpy maix.nn.YOLO26Depth.load
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
         * @maixpy maix.nn.YOLO26Depth.input_size
         */
        image::Size input_size()
        {
            return image::Size(_input_w, _input_h);
        }

        /**
         * Get model input width, only for image input
         * @return model input size of width
         * @maixpy maix.nn.YOLO26Depth.input_width
         */
        int input_width()
        {
            return _input_w;
        }

        /**
         * Get model input height, only for image input
         * @return model input size of height
         * @maixpy maix.nn.YOLO26Depth.input_height
         */
        int input_height()
        {
            return _input_h;
        }

        /**
         * Get input image format, only for image input
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLO26Depth.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Get model output size (depth map size), only for image input
         * @return model output size
         * @maixpy maix.nn.YOLO26Depth.output_size
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
         * @param cal_a calibration scale factor a, the depth is converted by d_real = exp(a * log(d) + b).
         *            Default 1.0 (no calibration). Set both cal_a/cal_b to fit your camera/scene.
         * @param cal_b calibration offset b, see cal_a. Default 0.0 (no calibration).
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a tensor.Tensor object with calibrated depth in meters. If in dual_buff mode, value can be None(in Python) or nullptr(in C++) when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.YOLO26Depth.get_depth
         * @maixcdk maix.nn.YOLO26Depth.get_depth
         */
        tensor::Tensor *get_depth(image::Image &img, image::Fit fit = image::FIT_CONTAIN,
                                  float cal_a = 1.0f, float cal_b = 0.0f)
        {
            if (_model == nullptr) return nullptr;
            std::unique_ptr<tensor::Tensors> outputs(_model->forward_image(img, {}, {}, fit, false, false));
            if (!outputs) return nullptr;
            if (outputs->size() == 0)
                throw err::Exception(err::ERR_RUNTIME, "depth model returned no output tensor");
            tensor::Tensor *t = outputs->begin()->second;
            if (!t)
                throw err::Exception(err::ERR_RUNTIME, "depth model returned a null output tensor");
            DepthInfo info = _get_depth_info(*t);
            _output_h = info.height;
            _output_w = info.width;
            tensor::Tensor *result = new tensor::Tensor(t->shape(), t->dtype(), t->data(), true);
            _apply_calibration((float *)result->data(), info.width * info.height, cal_a, cal_b);
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
         * @param cal_a calibration scale factor a, the depth is converted by d_real = exp(a * log(d) + b).
         *            Default 1.0 (no calibration). Set both cal_a/cal_b to fit your camera/scene.
         * @param cal_b calibration offset b, see cal_a. Default 0.0 (no calibration).
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a image::Image object with calibrated depth heatmap. If in dual_buff mode, value can be None(in Python) or nullptr(in C++) when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.YOLO26Depth.get_depth_image
         * @maixcdk maix.nn.YOLO26Depth.get_depth_image
         */
        image::Image *get_depth_image(image::Image &img, image::Fit fit = image::FIT_CONTAIN,
                                      image::CMap cmap = image::CMap::JET,
                                      float cal_a = 1.0f, float cal_b = 0.0f)
        {
            std::unique_ptr<tensor::Tensor> depth(get_depth(img, fit, cal_a, cal_b));
            if (!depth) return nullptr;
            return depth_to_image(*depth, img.width(), img.height(), fit, cmap);
        }

        /**
         * Convert an existing raw depth tensor to an RGB heatmap without running inference.
         * @param depth raw float32 depth tensor returned by get_depth().
         * @param image_width width of the source image passed to get_depth().
         * @param image_height height of the source image passed to get_depth().
         * @param fit resize fit mode used by get_depth().
         * @param cmap color map used to visualize the depth values.
         * @return a newly allocated image.Image at source-image size. In C++, delete it after use.
         * @throw If the tensor, image size, or fit mode is invalid, raises err.Exception.
         * @maixpy maix.nn.YOLO26Depth.depth_to_image
         * @maixcdk maix.nn.YOLO26Depth.depth_to_image
         */
        image::Image *depth_to_image(tensor::Tensor &depth, int image_width, int image_height,
                                     image::Fit fit = image::FIT_CONTAIN,
                                     image::CMap cmap = image::CMap::JET)
        {
            DepthInfo info = _get_depth_info(depth);
            DepthTransform transform = _get_depth_transform(image_width, image_height,
                                                            info.width, info.height, fit);

            const float *color_data = info.data;
            int color_width = info.width;
            int color_height = info.height;
            std::vector<float> content;

            if (fit == image::FIT_CONTAIN &&
                (transform.content_width != info.width || transform.content_height != info.height))
            {
                color_width = transform.content_width;
                color_height = transform.content_height;
                content.resize(color_width * color_height);
                for (int y = 0; y < color_height; ++y)
                {
                    const float *src = info.data + (y + transform.offset_y) * info.width + transform.offset_x;
                    std::copy(src, src + color_width, content.begin() + y * color_width);
                }
                color_data = content.data();
            }

            image::Image *heatmap = _colorize(color_data, color_width, color_height, cmap);
            if (color_width == image_width && color_height == image_height)
                return heatmap;

            image::Fit restore_fit = (fit == image::FIT_COVER) ? image::FIT_CONTAIN : image::FIT_FILL;
            image::Image *result = heatmap->resize(image_width, image_height, restore_fit);
            delete heatmap;
            return result;
        }

        /**
         * Read the estimated distance at a source-image position from an existing depth tensor.
         * This method only performs CPU-side coordinate mapping and does not run inference.
         * @param depth raw float32 depth tensor returned by get_depth().
         * @param x x coordinate in the source image.
         * @param y y coordinate in the source image.
         * @param image_width width of the source image passed to get_depth().
         * @param image_height height of the source image passed to get_depth().
         * @param fit resize fit mode used by get_depth().
         * @return estimated distance in meters, or NaN if the coordinate is outside the
         *         represented image region or the model value is invalid.
         * @throw If the tensor, image size, or fit mode is invalid, raises err.Exception.
         * @maixpy maix.nn.YOLO26Depth.get_distance
         * @maixcdk maix.nn.YOLO26Depth.get_distance
         */
        float get_distance(tensor::Tensor &depth, int x, int y, int image_width, int image_height,
                           image::Fit fit = image::FIT_CONTAIN)
        {
            DepthInfo info = _get_depth_info(depth);
            DepthTransform transform = _get_depth_transform(image_width, image_height,
                                                            info.width, info.height, fit);
            if (x < 0 || y < 0 || x >= image_width || y >= image_height)
                return std::numeric_limits<float>::quiet_NaN();

            int depth_x = transform.offset_x + (int)(x * transform.scale_x);
            int depth_y = transform.offset_y + (int)(y * transform.scale_y);
            if (depth_x < 0 || depth_y < 0 || depth_x >= info.width || depth_y >= info.height)
                return std::numeric_limits<float>::quiet_NaN();

            float value = info.data[depth_y * info.width + depth_x];
            if (!std::isfinite(value) || value <= 0)
                return std::numeric_limits<float>::quiet_NaN();
            return value;
        }

    private:
        struct DepthInfo
        {
            const float *data;
            int width;
            int height;
        };

        struct DepthTransform
        {
            float scale_x;
            float scale_y;
            int offset_x;
            int offset_y;
            int content_width;
            int content_height;
        };

        nn::NN *_model;
        bool _dual_buff;
        image::Format _input_img_fmt;
        int _input_w, _input_h;
        int _output_w, _output_h;

        DepthInfo _get_depth_info(tensor::Tensor &depth)
        {
            if (depth.dtype() != tensor::DType::FLOAT32)
                throw err::Exception(err::ERR_ARGS, "depth tensor dtype must be float32");
            std::vector<int> shape = depth.shape();
            if (shape.size() != 4 || shape[0] != 1 || shape[1] != 1 ||
                shape[2] <= 0 || shape[3] <= 0 || depth.data() == nullptr)
                throw err::Exception(err::ERR_ARGS, "depth tensor shape must be [1, 1, H, W]");
            return {(const float *)depth.data(), shape[3], shape[2]};
        }

        DepthTransform _get_depth_transform(int image_width, int image_height,
                                            int depth_width, int depth_height, image::Fit fit)
        {
            if (image_width <= 0 || image_height <= 0)
                throw err::Exception(err::ERR_ARGS, "source image size must be positive");

            DepthTransform transform = {};
            if (fit == image::FIT_FILL)
            {
                transform.content_width = depth_width;
                transform.content_height = depth_height;
                transform.offset_x = 0;
                transform.offset_y = 0;
            }
            else if (fit == image::FIT_CONTAIN)
            {
                float scale = std::min((float)depth_width / image_width,
                                       (float)depth_height / image_height);
                transform.content_width = (int)std::round(image_width * scale);
                transform.content_height = (int)std::round(image_height * scale);
                transform.offset_x = (depth_width - transform.content_width) / 2;
                transform.offset_y = (depth_height - transform.content_height) / 2;
            }
            else if (fit == image::FIT_COVER)
            {
                float scale = std::max((float)depth_width / image_width,
                                       (float)depth_height / image_height);
                transform.content_width = (int)std::round(image_width * scale);
                transform.content_height = (int)std::round(image_height * scale);
                transform.offset_x = -(transform.content_width - depth_width) / 2;
                transform.offset_y = -(transform.content_height - depth_height) / 2;
            }
            else
            {
                throw err::Exception(err::ERR_ARGS, "unsupported depth image fit mode");
            }

            if (transform.content_width <= 0 || transform.content_height <= 0)
                throw err::Exception(err::ERR_ARGS, "invalid mapped depth content size");
            transform.scale_x = (float)transform.content_width / image_width;
            transform.scale_y = (float)transform.content_height / image_height;
            return transform;
        }

        /**
         * Apply log-affine depth calibration in place: d_real = exp(a * log(d) + b).
         * With default a=1, b=0 the data is unchanged.
         */
        void _apply_calibration(float *data, int count, float cal_a, float cal_b)
        {
            if (cal_a == 1.0f && cal_b == 0.0f)
                return;
            for (int i = 0; i < count; i++)
            {
                float d = data[i];
                if (std::isfinite(d) && d > 0)
                    data[i] = expf(cal_a * logf(d) + cal_b);
            }
        }

        /**
         * Convert depth map to RGB888 heatmap: disparity(1/depth) + min-max normalize + cmap lookup.
         * Return a new image::Image, caller should delete it.
         */
        image::Image *_colorize(const float *depth, int w, int h, image::CMap cmap)
        {
            auto &colors = image::cmap_colors_rgb(cmap);
            image::Image *result = new image::Image(w, h, image::Format::FMT_RGB888);
            uint8_t *img_data = (uint8_t *)result->data();
            int n = w * h;

            float min_v = FLT_MAX, max_v = -FLT_MAX;
            int valid_count = 0;
            for (int i = 0; i < n; i++)
            {
                float d = depth[i];
                if (std::isfinite(d) && d > 0)
                {
                    float v = 1.0f / d;   // disparity
                    if (v < min_v) min_v = v;
                    if (v > max_v) max_v = v;
                    ++valid_count;
                }
            }
            if (valid_count == 0)
            {
                memset(img_data, 0, n * 3);
                return result;
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
