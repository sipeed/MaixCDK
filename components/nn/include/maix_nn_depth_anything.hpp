 /**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2025.6.9: Add depth anything v2 support
 */

#pragma once
#include <cfloat>
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_image_cmap.hpp"

namespace maix::nn
{
    /**
     * DepthAnything
     * @maixpy maix.nn.DepthAnything
     */
    class DepthAnything
    {
    public:
        /**
         * Construct a new DepthAnything object
         * @param model MUD model path, if empty, will not load model, you can call load() later.
         *                  if not empty, will load model and will raise err::Exception if load failed.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @maixpy maix.nn.DepthAnything.__init__
         * @maixcdk maix.nn.DepthAnything.DepthAnything
         */
         DepthAnything(const string &model = "", bool dual_buff = true)
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

        ~DepthAnything()
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
         * - model_type: depth_anything_v2
         * - input_type: rgb or bgr
         * - mean: 123.675, 116.28, 103.53
         * - scale: 0.017124753831663668, 0.01750700280112045, 0.017429193899782137
         * - labels: imagenet_classes.txt
         * @param model MUD model path
         * @return error code, if load failed, return error code
         * @maixpy maix.nn.DepthAnything.load
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
                if (_extra_info["model_type"] != "depth_anything_v2")
                {
                    log::error("model_type not match, expect 'depth_anything_v2', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            if (_extra_info.find("input_type") != _extra_info.end())
            {
                std::string input_type = _extra_info["input_type"];
                if (input_type == "rgb")
                {
                    _input_img_fmt = maix::image::FMT_RGB888;
                }
                else if (input_type == "bgr")
                {
                    _input_img_fmt = maix::image::FMT_BGR888;
                }
                else if (input_type == "gray")
                {
                    _input_img_fmt = maix::image::FMT_GRAYSCALE;
                }
                else
                {
                    log::error("unknown input type: %s", input_type.c_str());
                    return err::ERR_ARGS;
                }
                _input_is_img = true;
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
                }
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
                }
            }
            else
            {
                log::error("scale key not found");
                return err::ERR_ARGS;
            }
            _inputs = _model->inputs_info();
            if(_inputs[0].shape[3] <= 4) // nhwc
                _input_size = image::Size(_inputs[0].shape[2], _inputs[0].shape[1]);
            else
                _input_size = image::Size(_inputs[0].shape[3], _inputs[0].shape[2]);
            return err::ERR_NONE;
        }

        /**
         * Forward model and get raw image depth estimation data.
         * @param img image, format should match model input_type， or will raise err.Exception
         * @param fit image resize fit mode if input image not equal to model' input size,
         *            will auto resize to model's input size then detect, and recover to image input size.
         *            Default Fit.FIT_CONTAIN, see image.Fit.
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a tensor.Tensor object. If in dual_buff mode, value can be None(in Python) or nullptr(in C++) when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.DepthAnything.get_depth
         */
         tensor::Tensor *get_depth(image::Image &img, image::Fit fit = image::FIT_CONTAIN)
         {
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            if(!(fit == image::FIT_CONTAIN || fit == image::FIT_FILL || fit == image::FIT_COVER))
            {
                throw err::Exception("fit only support FIT_CONTAIN, FIT_FILL, FIT_COVER.");
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false, false);
            if (!outputs)
            {
                return nullptr;
            }
            tensor::Tensor *tensor = outputs->begin()->second;
            if (tensor->dtype() != tensor::DType::FLOAT32)
            {
                throw err::Exception("output tensor dtype only support float32 now");
            }
            tensor::Tensor *result = new tensor::Tensor(tensor->shape(), tensor->dtype(), tensor->data(), true);
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
         *             Default image.CMap.INFERNO.
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a image::Image object. If in dual_buff mode, value can be None(in Python) or nullptr(in C++) when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.DepthAnything.get_depth_image
         */
        image::Image *get_depth_image(image::Image &img, image::Fit fit = image::FIT_CONTAIN, image::CMap cmap = image::CMap::INFERNO)
        {
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            if(!(fit == image::FIT_CONTAIN || fit == image::FIT_FILL || fit == image::FIT_COVER))
            {
                throw err::Exception("fit only support FIT_CONTAIN, FIT_FILL, FIT_COVER.");
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false, false);
            if (!outputs)
            {
                return nullptr;
            }
            tensor::Tensor *tensor = outputs->begin()->second;
            if (tensor->dtype() != tensor::DType::FLOAT32)
            {
                throw err::Exception("output tensor dtype only support float32 now");
            }
            // convert to rgb
            auto convert_image_with_cmap = [](float *data, int w, int h, image::CMap cmap){
                auto &colors = image::cmap_colors_rgb(cmap);
                image::Image *result = new image::Image(w, h, image::Format::FMT_RGB888);
                float max_v = data[0];
                float min_v = data[0];
                for(int i = 1; i < w * h; ++i)
                {
                    if (data[i] > max_v)
                        max_v = data[i];
                    else if (data[i] < min_v)
                        min_v = data[i];
                }
                if(min_v == max_v)
                {
                    memset(result->data(), 127, w * h * 3);
                }
                else
                {
                    float scale = 255.0f / (max_v - min_v);
                    uint8_t *img_data = (uint8_t*)result->data();
                    #pragma omp parallel for
                    for(int i = 0; i < w * h; ++i)
                    {
                        uint8_t gray = static_cast<uint8_t>(std::clamp((data[i] - min_v) * scale, 0.0f, 255.0f));
                        auto &rgb = colors[gray];
                        img_data[3 * i] = rgb[0];
                        img_data[3 * i + 1] = rgb[1];
                        img_data[3 * i + 2] = rgb[2];
                    }
                }
                return result;
            };
            image::Image *result = convert_image_with_cmap((float*)tensor->data(), input_width(), input_height(), cmap);
            // check need resize
            if(img.width() * img.height() != tensor->size_int())
            {
                image::Fit fit_r = (fit == image::FIT_FILL) ? fit : (fit == image::FIT_CONTAIN ? image::FIT_COVER : image::FIT_CONTAIN);
                image::Image *result2 = result->resize(img.width(), img.height(), fit_r);
                delete result;
                result = result2;
            }
            delete outputs;
            return result;
        }

        /**
         * Get model input size, only for image input
         * @return model input size
         * @maixpy maix.nn.DepthAnything.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width, only for image input
         * @return model input size of width
         * @maixpy maix.nn.DepthAnything.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height, only for image input
         * @return model input size of height
         * @maixpy maix.nn.DepthAnything.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }


        /**
         * Get input image format, only for image input
         * @return input image format, image::Format type.
         * @maixpy maix.nn.DepthAnything.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Get input shape, if have multiple input, only return first input shape
         * @return input shape, list type
         * @maixpy maix.nn.DepthAnything.input_shape
         */
        std::vector<int> input_shape()
        {
            return _inputs[0].shape;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.DepthAnything.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.DepthAnything.scale
         */
        std::vector<float> scale;

    private:
        image::Format _input_img_fmt;
        bool _input_is_img;
        bool _dual_buff;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        image::Size _input_size;
        std::vector<nn::LayerInfo> _inputs;

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
