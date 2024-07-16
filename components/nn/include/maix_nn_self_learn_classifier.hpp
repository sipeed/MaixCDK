/**
 * Self learn classifier, learn anything and recognize.
 * @author neucrack@sipeed
 * @license Apache 2.0
 * @date 2024.6.14 Add support.
 */
#pragma once

#include "maix_basic.hpp"
#include "maix_nn.hpp"

namespace maix::nn
{

    /**
     * SelfLearnClassifier
     * @maixpy maix.nn.SelfLearnClassifier
     */
    class SelfLearnClassifier
    {
    public:
        /**
         * Construct a new SelfLearnClassifier object
         * @param model MUD model path, if empty, will not load model, you can call load_model() later.
         *                  if not empty, will load model and will raise err::Exception if load failed.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @maixpy maix.nn.SelfLearnClassifier.__init__
         * @maixcdk maix.nn.SelfLearnClassifier.SelfLearnClassifier
         */
        SelfLearnClassifier(const std::string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _feature_num = 0;
            _dual_buff = dual_buff;
            if (!model.empty())
            {
                err::Err e = load_model(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~SelfLearnClassifier()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            for (auto i : _features)
            {
                delete[] i;
            }
            for (auto i : _features_sample)
            {
                delete[] i;
            }
        }

        /**
         * Load model from file, model format is .mud,
         * MUD file should contain [extra] section, have key-values:
         * - model_type: classifier_no_top
         * - input_type: rgb or bgr
         * - mean: 123.675, 116.28, 103.53
         * - scale: 0.017124753831663668, 0.01750700280112045, 0.017429193899782137
         * @param model MUD model path
         * @return error code, if load failed, return error code
         * @maixpy maix.nn.SelfLearnClassifier.load_model
         */
        err::Err load_model(const string &model)
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
                if (_extra_info["model_type"] != "classifier_no_top")
                {
                    log::error("model_type not match, expect 'classifier_no_top', but got '%s'", _extra_info["model_type"].c_str());
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
            _input_size = image::Size(_inputs[0].shape[3], _inputs[0].shape[2]);
            _feature_num = _model->outputs_info()[0].shape_int();
            return err::ERR_NONE;
        }

        /**
         * Classify image
         * @param img image, format should match model input_type， or will raise err.Exception
         * @param fit image resize fit mode, default Fit.FIT_COVER, see image.Fit.
         * @throw If error occurred, will raise err::Exception, you can find reason in log, mostly caused by args error or hardware error.
         * @return result, a list of (idx, distance), smaller distance means more similar. In C++, you need to delete it after use.
         * @maixpy maix.nn.SelfLearnClassifier.classify
         */
        std::vector<std::pair<int, float>> *classify(image::Image &img, image::Fit fit = image::FIT_COVER)
        {
            float *feature = NULL;
            tensor::Tensors *outs = _get_feature(img, &feature, fit);
            std::vector<std::pair<int, float>> *distances = new std::vector<std::pair<int, float>>();
            for (size_t i = 0; i < _features.size(); ++i)
            {
                float distance = _get_distance(feature, _features[i]);
                distances->push_back(std::make_pair(i, distance));
            }
            delete outs;
            // sort
            std::sort(distances->begin(), distances->end(), [](const std::pair<int, float> &a, const std::pair<int, float> &b)
                      { return a.second < b.second; });
            return distances;
        }

        /**
         * Add a class to recognize
         * @param img Add a image as a new class
         * @param fit image resize fit mode, default Fit.FIT_COVER, see image.Fit.
         * @maixpy maix.nn.SelfLearnClassifier.add_class
         */
        void add_class(image::Image &img, image::Fit fit = image::FIT_COVER)
        {
            float *feature = NULL;
            tensor::Tensors *outs = _get_feature(img, &feature, fit);
            _add_feature(feature);
            delete outs;
        }

        /**
         * Get class number
         * @maixpy maix.nn.SelfLearnClassifier.class_num
         */
        int class_num()
        {
            return (int)_features.size();
        }

        /**
         * Remove a class
         * @param idx index, value from 0 to class_num();
         * @maixpy maix.nn.SelfLearnClassifier.rm_class
         */
        err::Err rm_class(int idx)
        {
            if ((size_t)idx >= _features.size())
                return err::ERR_ARGS;
            delete[] _features[idx];
            _features.erase(_features.begin() + idx);
            return err::ERR_NONE;
        }

        /**
         * Add sample, you should call learn method after add some samples to learn classes.
         * Sample image can be any of classes we already added.
         * @param img Add a image as a new sample.
         * @maixpy maix.nn.SelfLearnClassifier.add_sample
         */
        void add_sample(image::Image &img, image::Fit fit = image::FIT_COVER)
        {
            float *feature = NULL;
            tensor::Tensors *outs = _get_feature(img, &feature, fit);
            _add_feature_sample(feature);
            delete outs;
        }

        /**
         * Remove a sample
         * @param idx index, value from 0 to sample_num();
         * @maixpy maix.nn.SelfLearnClassifier.rm_sample
         */
        err::Err rm_sample(int idx)
        {
            if ((size_t)idx >= _features_sample.size())
                return err::ERR_ARGS;
            delete[] _features_sample[idx];
            _features_sample.erase(_features_sample.begin() + idx);
            return err::ERR_NONE;
        }

        /**
         * Get sample number
         * @maixpy maix.nn.SelfLearnClassifier.sample_num
         */
        int sample_num()
        {
            return (int)_features_sample.size();
        }

        /**
         * Start auto learn class features from classes image and samples.
         * You should call this method after you add some samples.
         * @return learn epoch(times), 0 means learn nothing.
         * @maixpy maix.nn.SelfLearnClassifier.learn
         */
        int learn();

        /**
         * Clear all class and samples
         * @maixpy maix.nn.SelfLearnClassifier.clear
         */
        void clear()
        {
            for (auto i : _features)
            {
                delete[] i;
            }
            _features.clear();
            for (auto i : _features_sample)
            {
                delete[] i;
            }
            _features_sample.clear();
        }

        /**
         * Get model input size, only for image input
         * @return model input size
         * @maixpy maix.nn.SelfLearnClassifier.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width, only for image input
         * @return model input size of width
         * @maixpy maix.nn.SelfLearnClassifier.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height, only for image input
         * @return model input size of height
         * @maixpy maix.nn.SelfLearnClassifier.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format, only for image input
         * @return input image format, image::Format type.
         * @maixpy maix.nn.SelfLearnClassifier.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Get input shape, if have multiple input, only return first input shape
         * @return input shape, list type
         * @maixpy maix.nn.SelfLearnClassifier.input_shape
         */
        std::vector<int> input_shape()
        {
            return _inputs[0].shape;
        }

        /**
         * Save features and labels to a binary file
         * @param path file path to save, e.g. /root/my_classes.bin
         * @param labels class labels, can be None, or length must equal to class num, or will return err::Err
         * @return maix.err.Err if labels exists but length not equal to class num, or save file failed, or class num is 0.
         * @maixpy maix.nn.SelfLearnClassifier.save
         */
        err::Err save(const std::string &path, const std::vector<std::string> &labels = std::vector<std::string>())
        {
            // 1B: version, now only 0
            // 4B: (n) class num, int32_t type
            // 4B: (m) sample num, int32_t type
            // 4B: (f) feature length, int32_t type
            // 1B: have labels, uint8_t type, 0 mean no, 1 means have
            // *B: labels(if have), every label ends with \0
            // n*fB: n(class num) class features, every feature length is f.
            // m*fB: m(class num) sample features, every feature length is f.
            // std::vector<float *> _features;
            // std::vector<float *> _features_sample;
            if (_features.empty())
            {
                log::error("class num must > 0");
                return maix::err::ERR_ARGS;
            }

            // Check if labels size matches the number of classes
            if (!labels.empty() && labels.size() != _features.size())
            {
                log::error("labels length must equal to class num");
                return maix::err::ERR_ARGS;
            }

            fs::File *f = maix::fs::open(path, "wb");
            if (!f)
            {
                log::error("Failed to open file for saving");
                return maix::err::ERR_IO;
            }

            uint8_t version = 0;
            int32_t class_num = static_cast<int32_t>(_features.size());
            int32_t sample_num = static_cast<int32_t>(_features_sample.size());
            int32_t feature_length = static_cast<int32_t>(_feature_num);
            uint8_t have_labels = labels.empty() ? 0 : 1;

            f->write(&version, sizeof(version));
            f->write(&class_num, sizeof(class_num));
            f->write(&sample_num, sizeof(sample_num));
            f->write(&feature_length, sizeof(feature_length));
            f->write(&have_labels, sizeof(have_labels));

            if (have_labels)
            {
                for (const auto &label : labels)
                {
                    f->write(label.c_str(), label.size() + 1);
                }
            }

            for (const auto &feature : _features)
            {
                f->write(reinterpret_cast<const char *>(feature), _feature_num * sizeof(float));
            }

            for (const auto &feature : _features_sample)
            {
                f->write(reinterpret_cast<const char *>(feature), _feature_num * sizeof(float));
            }

            f->close();
            delete f; // Make sure to delete the file object to avoid memory leaks

            return maix::err::ERR_NONE;
        }

        /**
         * Load features info from binary file
         * @param path feature info binary file path, e.g. /root/my_classes.bin
         * @maixpy maix.nn.SelfLearnClassifier.load
         */

        std::vector<std::string> load(const std::string &path)
        {
            fs::File *f = maix::fs::open(path, "rb");
            if (!f)
            {
                log::error("Open failed");
                throw err::Exception(err::ERR_IO);
            }

            uint8_t version;
            int32_t class_num, sample_num, feature_length;
            uint8_t have_labels;

            f->read(&version, sizeof(version));
            f->read(&class_num, sizeof(class_num));
            f->read(&sample_num, sizeof(sample_num));
            f->read(&feature_length, sizeof(feature_length));
            f->read(&have_labels, sizeof(have_labels));
            if (feature_length != _feature_num)
            {
                log::error("feature length(%d) not equal to this model's(%d)", feature_length, _feature_num);
                throw err::Exception(err::ERR_ARGS);
            }

            std::vector<std::string> labels;
            if (have_labels)
            {
                for (int i = 0; i < class_num; ++i)
                {
                    std::string label;
                    char c;
                    while (f->read(&c, 1) == 1 && c != '\0')
                    {
                        label += c;
                    }
                    labels.push_back(label);
                }
            }
            for (auto i : _features)
            {
                delete[] i;
            }
            _features.clear();
            for (int i = 0; i < class_num; ++i)
            {
                float *feature = new float[_feature_num];
                _features.push_back(feature);
                f->read(reinterpret_cast<char *>(feature), feature_length * sizeof(float));
            }
            for (auto i : _features_sample)
            {
                delete[] i;
            }
            _features_sample.clear();
            for (int i = 0; i < sample_num; ++i)
            {
                float *feature = new float[_feature_num];
                _features_sample.push_back(feature);
                f->read(reinterpret_cast<char *>(feature), feature_length * sizeof(float));
            }

            f->close();
            delete f; // Make sure to delete the file object to avoid memory leaks

            return labels;
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.SelfLearnClassifier.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.SelfLearnClassifier.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.SelfLearnClassifier.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.SelfLearnClassifier.scale
         */
        std::vector<float> scale;

    private:
        image::Format _input_img_fmt;
        bool _input_is_img;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        image::Size _input_size;
        int _feature_num;
        bool _dual_buff;
        std::vector<nn::LayerInfo> _inputs;
        std::vector<float *> _features;
        std::vector<float *> _features_sample;

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

        /**
         * You should detele result after use features.
         */
        tensor::Tensors *_get_feature(image::Image &img, float **feature, image::Fit fit = image::FIT_COVER)
        {
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs = _model->forward_image(img, this->mean, this->scale, fit, false, true);
            if (!outputs) // can be here!
            {
                throw err::Exception("forward image failed");
            }
            tensor::Tensor *tensor = outputs->begin()->second;
            if (tensor->dtype() != tensor::DType::FLOAT32)
            {
                throw err::Exception("output tensor dtype only support float32 now");
            }
            *feature = (float *)tensor->data();
            return outputs;
        }

        void _add_feature(float *new_feature)
        {
            float *feature = new float[_feature_num];
            memcpy(feature, new_feature, _feature_num * sizeof(float));
            _features.push_back(feature);
        }

        void _add_feature_sample(float *new_feature)
        {
            float *feature = new float[_feature_num];
            memcpy(feature, new_feature, _feature_num * sizeof(float));
            _features_sample.push_back(feature);
        }

        float _get_distance(float *a, float *b)
        {
            float sum = 0.;
            for (int i = 0; i < _feature_num; i++)
            {
                sum += (a[i] - b[i]) * (a[i] - b[i]);
            }
            sum = sqrtf(sum);
            return sum;
        }
    }; // class SelfLearnClassifier

} // namespace maix::nn
