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

namespace maix::nn
{
    /**
     * Classifier
     * @maixpy maix.nn.Classifier
     */
    class Classifier
    {
    public:
        /**
         * Construct a new Classifier object
         * @param model MUD model path, if empty, will not load model, you can call load() later.
         *                  if not empty, will load model and will raise err::Exception if load failed.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @maixpy maix.nn.Classifier.__init__
         * @maixcdk maix.nn.Classifier.Classifier
         */
        Classifier(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            _chw = true;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~Classifier()
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
         * - model_type: classifier
         * - input_type: rgb or bgr
         * - mean: 123.675, 116.28, 103.53
         * - scale: 0.017124753831663668, 0.01750700280112045, 0.017429193899782137
         * - labels: imagenet_classes.txt
         * @param model MUD model path
         * @return error code, if load failed, return error code
         * @maixpy maix.nn.Classifier.load
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
                if (_extra_info["model_type"] != "classifier")
                {
                    log::error("model_type not match, expect 'classifier', but got '%s'", _extra_info["model_type"].c_str());
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
            if (_extra_info.find("input_channel") != _extra_info.end())
            {
                std::string channel_str = _extra_info["input_channel"];
                if(channel_str == "hwc")
                    _chw = false;
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
            _inputs = _model->inputs_info();
            if(_chw)
                _input_size = image::Size(_inputs[0].shape[3], _inputs[0].shape[2]);
            else
                _input_size = image::Size(_inputs[0].shape[2], _inputs[0].shape[1]);
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
         * Forward image to model, get result. Only for image input, use classify_raw for tensor input.
         * @param img image, format should match model input_type， or will raise err.Exception
         * @param softmax if true, will do softmax to result, or will return raw value
         * @param fit image resize fit mode, default Fit.FIT_COVER, see image.Fit.
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a list of (label, score). If in dual_buff mode, value can be one element list and score is zero when not ready. In C++, you need to delete it after use.
         * @maixpy maix.nn.Classifier.classify
         */
        std::vector<std::pair<int, float>> *classify(image::Image &img, bool softmax = true, image::Fit fit = image::FIT_COVER)
        {
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false, false, _chw);
            if (!outputs)
            {
                std::vector<std::pair<int, float>> *res = new std::vector<std::pair<int, float>>(1);
                res->at(0).first = 0;
                res->at(0).second = 0;
                return res;
            }
            tensor::Tensor *tensor = outputs->begin()->second;
            if (tensor->dtype() != tensor::DType::FLOAT32)
            {
                throw err::Exception("output tensor dtype only support float32 now");
            }
            if (softmax)
                maix::nn::F::softmax(tensor, true);
            std::vector<std::pair<int, float>> *result = new std::vector<std::pair<int, float>>(tensor->size_int());
            float *data = (float *)tensor->data();
            for (int i = 0; i < tensor->size_int(); i++)
            {
                result->at(i).first = i;
                result->at(i).second = data[i];
            }
            std::sort(result->begin(), result->end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b)
                      { return a.second > b.second; });
            delete outputs;
            return result;
        }

        /**
         * Forward tensor data to model, get result
         * @param data tensor data, format should match model input_type， or will raise err.Excetion
         * @param softmax if true, will do softmax to result, or will return raw value
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a list of (label, score). In C++, you need to delete it after use.
         * @maixpy maix.nn.Classifier.classify_raw
         */
        std::vector<std::pair<int, float>> *classify_raw(tensor::Tensor &data, bool softmax = true)
        {
            // TODO: add classify for tensor data
            throw err::Exception(err::ERR_NOT_IMPL);
        }

        /**
         * Get model input size, only for image input
         * @return model input size
         * @maixpy maix.nn.Classifier.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width, only for image input
         * @return model input size of width
         * @maixpy maix.nn.Classifier.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height, only for image input
         * @return model input size of height
         * @maixpy maix.nn.Classifier.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }


        /**
         * Get input image format, only for image input
         * @return input image format, image::Format type.
         * @maixpy maix.nn.Classifier.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Get input shape, if have multiple input, only return first input shape
         * @return input shape, list type
         * @maixpy maix.nn.Classifier.input_shape
         */
        std::vector<int> input_shape()
        {
            return _inputs[0].shape;
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.Classifier.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.Classifier.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.Classifier.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.Classifier.scale
         */
        std::vector<float> scale;

    private:
        image::Format _input_img_fmt;
        bool _input_is_img;
        bool _dual_buff;
        bool _chw;
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
