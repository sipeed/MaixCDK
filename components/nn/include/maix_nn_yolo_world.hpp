/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2025.4.24: Add yolo-world support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>
#include "maix_nn_yolo11.hpp"

namespace maix::nn
{
    /**
     * YOLOWorld class
     * @maixpy maix.nn.YOLOWorld
     */
    class YOLOWorld
    {
    public:
        /**
         * Constructor of YOLOWorld class
         * @param model model path, default empty, you can load model later by load function.
         * @param text_feature class text feature path, more info refer to load method, default empty, you can load class text feature later by load function.
         * @param labels Class labels or labels file path.
         *        If string class labels: labels split by comma, e.g. "person, car, cat".
         *        If file path: labels file path, each line is a label.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.YOLOWorld.__init__
         * @maixcdk maix.nn.YOLOWorld.YOLOWorld
         */
        YOLOWorld(const string &model = "", const string &text_feature = "", const string &labels = "", bool dual_buff = true)
        {
            _model = nullptr;
            _dual_buff = dual_buff;
            if (!model.empty())
            {
                err::Err e = load(model, text_feature, labels);
                if (e != err::ERR_NONE)
                {
                    char tmp[128] = {0};
                    snprintf(tmp, sizeof(tmp), "load model %s failed", model.c_str());
                    throw err::Exception(e, tmp);
                }
            }
        }

        ~YOLOWorld()
        {
            if(_input_text_feature)
            {
                free(_input_text_feature);
                _input_text_feature = nullptr;
            }
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @param text_feature Class text feature bin file path.
         * @param labels Class labels or labels file path.
         *        If string class labels: labels split by comma, e.g. "person, car, cat".
         *        If file path: labels file path, each line is a label.
         * @return err::Err
         * @maixpy maix.nn.YOLOWorld.load
         */
        err::Err load(const string &model, const string &text_feature, const string &labels)
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            if (text_feature.empty())
            {
                log::error("class text feature path should not empty");
                return err::ERR_ARGS;
            }
            if (!fs::exists(text_feature))
            {
                log::error("class text feature path %s not exist", text_feature.c_str());
                return err::ERR_ARGS;
            }
            if (labels.empty())
            {
                log::error("labels should be labels string or labels file path");
                return err::ERR_ARGS;
            }
            if (labels.find(",") == std::string::npos && fs::exists(labels))
            {
                // load labels from file
                err::Err e = _load_labels_from_file(this->labels, labels);
                if (e != err::ERR_NONE)
                {
                    log::error("load labels from file %s failed", labels.c_str());
                    return e;
                }
            }
            else
            {
                // load labels from string
                std::vector<std::string> label_list = split(labels, ",");
                for (auto &it : label_list)
                {
                    it.erase(0, it.find_first_not_of(" \t\r\n"));
                    it.erase(it.find_last_not_of(" \t\r\n") + 1);
                    if(!it.empty())
                        this->labels.push_back(it);
                }
            }

            _model = new nn::NN(model, _dual_buff);
            if (!_model)
            {
                return err::ERR_NO_MEM;
            }
            _extra_info = _model->extra_info();
            if (_extra_info.find("model_type") != _extra_info.end())
            {
                if (_extra_info["model_type"] != this->_type_str)
                {
                    log::error("model_type not match, expect '%s', but got '%s'", this->_type_str.c_str(), _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }

            log::info("model info:\n\ttype: %s", this->_type_str.c_str());
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
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            int i = 0;
            for(auto &item : inputs)
            {
                _input_names.push_back(item.name);
                if(item.shape.size() == 4)
                {
                    _image_in_idx = i;
                    if(inputs[0].shape[3] <= 4) // nhwc
                        _input_size = image::Size(inputs[0].shape[2], inputs[0].shape[1]);
                    else
                        _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
                }
                else
                {
                    _text_in_idx = i;
                    _text_feature_num = item.shape[2]; // 1 4 512
                    _labels_num = item.shape[1];
                }
                ++i;
            }
            if ((size_t)_labels_num != this->labels.size())
            {
                log::error("labels num not match, model expect %d, but code expect %d", _labels_num, (int)this->labels.size());
                return err::ERR_ARGS;
            }
            log::print(log::LogLevel::LEVEL_INFO, "\tinput size: %dx%d\n", _input_size.width(), _input_size.height());
            log::print(log::LogLevel::LEVEL_INFO, "\ttext feature num: %d\n", _text_feature_num);
            log::print(log::LogLevel::LEVEL_INFO, "\tlabels num: %d\n\n", _labels_num);

            if(_input_text_feature)
                delete _input_text_feature;
            _input_text_feature = (float*)malloc(_text_feature_num * _labels_num * sizeof(float));
            if(!_input_text_feature)
            {
                log::error("malloc input text feature failed");
                return err::ERR_NO_MEM;
            }
            fs::File *f = fs::open(text_feature, "rb");
            if (!f)
            {
                log::error("open text feature file %s failed", text_feature.c_str());
                return err::ERR_ARGS;
            }
            int read_size = f->read(_input_text_feature, _text_feature_num * _labels_num * sizeof(float));
            if (read_size < 0)
            {
                log::error("read text feature file %s failed", text_feature.c_str());
                return err::ERR_IO;
            }
            if ((size_t)read_size != _text_feature_num * _labels_num * sizeof(float))
            {
                log::error("read text feature file %s size not match, expect %d, but %d", text_feature.c_str(), _text_feature_num * _labels_num * sizeof(float), read_size);
                return err::ERR_IO;
            }
            f->close();
            delete f;

            // output info
            _out_chw = true;
            _anchor_num = 0;
            for(size_t i=0; i < _stride.size(); ++i)
            {
                _anchor_num += _input_size.width()/_stride[i] * _input_size.height() / _stride[i];
            }
            std::vector<nn::LayerInfo> outputs = _model->outputs_info();
            auto print_outputs = [outputs](){
                log::info("Outputs:");
                for(auto item : outputs)
                {
                    log::print(log::LogLevel::LEVEL_INFO, "       %s\n", item.to_str().c_str());
                }
            };
            if(outputs.size() != 3)
            {
                log::error("output node size should be 3, but got %d", (int)outputs.size());
                print_outputs();
                return err::ERR_ARGS;
            }
            int ch_num = this->labels.size() + _reg_max * 4;
            for(size_t i = 0; i < outputs.size(); ++i)
            {
                nn::LayerInfo &item = outputs[i];
                if(item.shape.size() == 4)
                {
                    if(item.shape[3] == ch_num && item.shape[1] == _input_size.height() / _stride[0]
                        && item.shape[2] == _input_size.width() / _stride[0]) // hwc
                    {
                        _out_chw = false;
                        _out_idxes.det0 = i;
                        break;
                    }
                }
            }
            for(size_t i = 0; i < outputs.size(); ++i)
            {
                nn::LayerInfo &item = outputs[i];
                if(item.shape.size() == 4)
                {
                    if(item.shape[_out_chw ? 2 : 1] == _input_size.height() / _stride[0])
                    {
                        _out_idxes.det0 = i;
                    }
                    else if(item.shape[_out_chw ? 2 : 1] == _input_size.height() / _stride[1])
                    {
                        _out_idxes.det1 = i;
                    }
                    else if(item.shape[_out_chw ? 2 : 1] == _input_size.height() / _stride[2])
                    {
                        _out_idxes.det2 = i;
                    }
                    else
                    {
                        log::error("output node shape error, please check %d, chw: %d,  %d", 1, _out_chw, item.shape[_out_chw ? 2 : 1]);
                        print_outputs();
                        return err::ERR_ARGS;
                    }
                }
                else
                {
                    log::error("output node shape error, please check %d", 3);
                    print_outputs();
                    return err::ERR_ARGS;
                }
            }
            return err::ERR_NONE;
        }

        /**
         * Set detector class labels dynamically, will generate class text feature and save to text_feature path set in load method or constructor.
         * @param labels class labels you want to recognize, list type. e.g. ["person", "car", "cat"]
         * @return err::Err
         * @maixpy maix.nn.YOLOWorld.learn_text_feature
         */
        static err::Err learn_text_feature(const std::string &model, std::vector<std::string> labels, const std::string &feature_path, const std::string &labels_path)
        {
            log::error("learn_text_feature is not implemented, please read document to learn other ways");
            return err::ERR_NOT_IMPL;
            if (labels.empty())
            {
                log::error("labels should not empty");
                return err::ERR_ARGS;
            }
            if (feature_path.empty())
            {
                log::error("feature path should not empty");
                return err::ERR_ARGS;
            }
            if (labels_path.empty())
            {
                log::error("labels path should not empty");
                return err::ERR_ARGS;
            }
            nn::NN *model_ptr = new nn::NN(model, false);
            if (!model_ptr)
            {
                log::error("load model %s failed", model.c_str());
                return err::ERR_NO_MEM;
            }
            std::map<std::string, std::string> extra_info = model_ptr->extra_info();
            if (extra_info.find("model_type") == extra_info.end())
            {
                log::error("model_type key not found");
                delete model_ptr;
                return err::ERR_ARGS;
            }
            if (extra_info["model_type"] != "yolo_world")
            {
                log::error("model_type not match, expect 'yolo_world', but got '%s'", extra_info["model_type"].c_str());
                delete model_ptr;
                return err::ERR_ARGS;
            }
            if (extra_info.find("type") == extra_info.end())
            {
                log::error("type key not found");
                delete model_ptr;
                return err::ERR_ARGS;
            }
            if (extra_info["type"] != "text_feature")
            {
                log::error("type not match, expect 'text_feature', but got '%s'", extra_info["type"].c_str());
                delete model_ptr;
                return err::ERR_ARGS;
            }

            std::vector<nn::LayerInfo> inputs_info = model_ptr->inputs_info();
            int labels_num = inputs_info[0].shape[0];   // 4 77
            int label_length = inputs_info[0].shape[1];
            std::string input_name = inputs_info[0].name;

            // forward feature
            tensor::Tensors inputs;
            tensor::Tensors outputs;
            int32_t *tokens = new int32_t[labels_num * label_length];
            if(!tokens)
            {
                log::error("malloc tokens failed");
                delete model_ptr;
                return err::ERR_NO_MEM;
            }
            _labels_to_tokens(labels, tokens, labels_num, label_length);
            tensor::Tensor *tensor_text = new tensor::Tensor(std::vector<int>{labels_num, label_length}, tensor::DType::INT32, tokens, false);
            inputs.add_tensor(input_name, tensor_text, false, true);
            err::Err e = model_ptr->forward(inputs, outputs, false, true);
            delete tokens;
            if(e != err::ERR_NONE)
            {
                log::error("forward failed, err: %s", err::to_str(e).c_str());
                return e;
            }

            fs::File *f = fs::open(feature_path, "wb");
            if (!f)
            {
                log::error("open feature file %s failed", feature_path.c_str());
                delete model_ptr;
                return err::ERR_ARGS;
            }
            f->write(outputs[0].data(), outputs[0].size_int() * sizeof(int32_t));
            f->close();
            delete f;

            f = fs::open(labels_path, "w");
            if (!f)
            {
                log::error("open feature file %s failed", labels_path.c_str());
                delete model_ptr;
                return err::ERR_ARGS;
            }
            for (auto &it : labels)
            {
                it.erase(0, it.find_first_not_of(" \t\r\n"));
                it.erase(it.find_last_not_of(" \t\r\n") + 1);
                if(!it.empty())
                {
                    f->write(it.c_str(), it.size());
                    f->write("\n", 1);
                }
            }
            f->close();
            delete f;
            delete model_ptr;
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
         * @maixpy maix.nn.YOLOWorld.detect
         */
        nn::Objects *detect(image::Image &img, float conf_th = 0.5, float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN, int sort = 0)
        {
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors inputs;
            tensor::Tensors outputs;
            image::Image *img_in = &img;
            bool img_need_free = false;
            if(img.width() != _input_size.width() || img.height() != _input_size.height())
            {
                img_in = img.resize(_input_size.width(), _input_size.height(), fit);
                img_need_free = true;
            }
            tensor::Tensor *tensor = new tensor::Tensor(std::vector<int>{1, _input_size.height(), _input_size.width(), 3}, tensor::DType::UINT8, img_in->data(), false);
            if(!tensor)
            {
                if(img_need_free)
                    delete img_in;
                throw err::Exception(err::ERR_NO_MEM);
            }
            inputs.add_tensor(_input_names[_image_in_idx], tensor, false, true);
            tensor::Tensor *tensor_text = new tensor::Tensor(std::vector<int>{1, _labels_num, _text_feature_num}, tensor::DType::FLOAT32, _input_text_feature, false);
            inputs.add_tensor(_input_names[_text_in_idx], tensor_text, false, true);
            err::Err e = _model->forward(inputs, outputs, false, false);
            if(img_need_free)
                delete img_in;
            if (e == err::ERR_NOT_READY) // not ready, return empty result.
            {
                return new nn::Objects();
            }
            else if(e != err::ERR_NONE)
            {
                log::error("forward failed, err: %s", err::to_str(e).c_str());
                return nullptr;
            }
            nn::Objects *res = _post_process(&outputs, img.width(), img.height(), fit, sort);
            if(!res)
            {
                throw err::Exception("post process failed, please see log before");
            }
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.YOLOWorld.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.YOLOWorld.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.YOLOWorld.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLOWorld.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }



    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLOWorld.labels
         */
        std::vector<string> labels;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLOWorld.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLOWorld.scale
         */
        std::vector<float> scale;

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
                if(!line.empty())
                    labels.push_back(line);
            }
            f->close();
            delete f;
            return err::ERR_NONE;
        }

        nn::Objects *_post_process(tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit, int sort)
        {
            nn::Objects *objects = new nn::Objects();
            tensor::Tensor *kp_out = NULL;
            tensor::Tensor *mask_out = NULL;
            float scale_w = 1;
            float scale_h = 1;

            if(!_decode_objs(*objects, outputs, _conf_th, _input_size.width(), _input_size.height(), &kp_out, &mask_out))
            {
                delete objects;
                return NULL;
            }
            if (objects->size() > 0)
            {
                nn::Objects *objects_total = objects;
                objects = _nms(*objects);
                delete objects_total;
                if(sort != 0)
                {
                    _sort_objects(*objects, sort);
                }
            }
            if (objects->size() > 0)
            {
                _correct_bbox(*objects, img_w, img_h, fit, &scale_w, &scale_h);
            }
            return objects;
        }

        bool _decode_objs(nn::Objects &objs, tensor::Tensors *outputs, float conf_thresh, int w, int h, tensor::Tensor **kp_out, tensor::Tensor **mask_out)
        {
            int idx_start[3] = {
                0,
                (int)(h / _stride[0] * w / _stride[0]),
                (int)(h / _stride[0] * w / _stride[0] + h / _stride[1] * w / _stride[1])};
            // detect
            tensor::Tensor *dets[3] = {&(*outputs)[_out_idxes.det0], &(*outputs)[_out_idxes.det1], &(*outputs)[_out_idxes.det2]};
            if(_out_chw)
            {
                throw err::Exception(err::ERR_NOT_IMPL, "not support output chw layout");
            }
            else
            {
                int class_num = (int)labels.size();
                int prob_offset = _reg_max * 4;
                int batch_data_size = prob_offset + class_num;
                // for(int i = 0; i < 3; ++i) // 1 x nh x nw x (_reg_max * 4 + class_num)
                // {
                //     int nh = dets[i]->shape()[1];
                //     int nw = dets[i]->shape()[2];
                //     int anchor_num = nh * nw;
                //     float *feature = (float*)dets[i]->data();
                //     for (int anchor_idx = 0; anchor_idx < anchor_num; ++anchor_idx)
                    #pragma omp parallel for
                    for(int index = 0; index < _anchor_num; ++index)
                    {
                        int i = 1;
                        int anchor_idx;
                        if (index >= idx_start[2]) {
                            i = 2;
                            anchor_idx = index - idx_start[2];
                        } else if (index < idx_start[1]) {
                            i = 0;
                            anchor_idx = index;
                        }
                        else{
                            anchor_idx = index - idx_start[1];
                        }
                        // int nh = dets[i]->shape()[1];
                        int nw = dets[i]->shape()[2];
                        float *feature = (float*)dets[i]->data();

                        int ax = anchor_idx % nw;
                        int ay = anchor_idx / nw;
                        float *p = feature + batch_data_size * anchor_idx;
                        int class_id = _argmax(p + prob_offset, class_num, 1);
                        float score = _sigmoid(p[prob_offset + class_id]);
                        if (score <= conf_thresh)
                        {
                            continue;
                        }
                        float dis[4];
                        for(int k = 0; k < 4; ++k)
                        {
                            // std::vector<float> softmax_res(_reg_max, 0.f);
                            float softmax_res[_reg_max];
                            dis[k] = _softmax(p + k * _reg_max, softmax_res, _reg_max);
                        }
                        float bbox_x = (ax + 0.5 - dis[0]) * _stride[i];
                        float bbox_y = (ay + 0.5 - dis[1]) * _stride[i];
                        float bbox_w = (ax + 0.5 + dis[2]) * _stride[i] - bbox_x;
                        float bbox_h = (ay + 0.5 + dis[3]) * _stride[i] - bbox_y;
                        // _KpInfoYolo11 *kp_info = new _KpInfoYolo11(idx_start[i] + anchor_idx, ax, ay, _stride[i]);
                        #pragma omp critical
                        {
                            Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, score);
                            // obj.temp = (void *)kp_info;
                            obj.temp = nullptr;
                        }
                    }
                // }
            }
            return true;
        }

        nn::Objects *_nms(nn::Objects &objs)
        {
            nn::Objects *result = new nn::Objects();
            std::sort(objs.begin(), objs.end(), [](const nn::Object *a, const nn::Object *b)
                      { return a->score > b->score; });
            for (size_t i = 0; i < objs.size(); ++i)
            {
                nn::Object &a = objs.at(i);
                if (a.score == 0)
                    continue;
                for (size_t j = i + 1; j < objs.size(); ++j)
                {
                    nn::Object &b = objs.at(j);
                    {
                        if (b.score != 0 && a.class_id == b.class_id && _calc_iou(a, b) > this->_iou_th)
                        {
                            b.score = 0;
                        }
                    }
                }
            }
            for (nn::Object *a : objs)
            {
                if (a->score != 0)
                {
                    Object &obj = result->add(a->x, a->y, a->w, a->h, a->class_id, a->score, a->points, a->angle);
                    if (obj.x < 0)
                    {
                        obj.w += obj.x;
                        obj.x = 0;
                    }
                    if (obj.y < 0)
                    {
                        obj.h += obj.y;
                        obj.y = 0;
                    }
                    if (obj.x + obj.w > _input_size.width())
                    {
                        obj.w = _input_size.width() - obj.x;
                    }
                    if (obj.y + obj.h > _input_size.height())
                    {
                        obj.h = _input_size.height() - obj.y;
                    }
                    obj.temp = a->temp;
                }
                else
                {
                    // delete (_KpInfoYolo11 *)a->temp;
                    a->temp = NULL;
                }
            }
            return result;
        }

        void _sort_objects(nn::Objects &objects, int sort)
        {
            if (sort > 0)
                std::sort(objects.begin(), objects.end(), [](const nn::Object *a, const nn::Object *b)
                      { return (a->w * a->h) > (b->w * b->h); });
            else
                std::sort(objects.begin(), objects.end(), [](const nn::Object *a, const nn::Object *b)
                      { return (a->w * a->h) < (b->w * b->h); });
        }

        void _correct_bbox(nn::Objects &objs, int img_w, int img_h, maix::image::Fit fit, float *scale_w, float *scale_h)
        {
#define CORRECT_BBOX_RANGE_YOLO11_2(obj)      \
    do                               \
    {                                \
        if (obj->x < 0)              \
        {                            \
            obj->w += obj->x;        \
            obj->x = 0;              \
        }                            \
        if (obj->y < 0)              \
        {                            \
            obj->h += obj->y;        \
            obj->y = 0;              \
        }                            \
        if (obj->x + obj->w > img_w) \
        {                            \
            obj->w = img_w - obj->x; \
        }                            \
        if (obj->y + obj->h > img_h) \
        {                            \
            obj->h = img_h - obj->y; \
        }                            \
    } while (0)

            // for (nn::Object *obj : objs)
            // {
            //     if (obj->temp)
            //     {
            //         delete (_KpInfoYolo11 *)obj->temp;
            //         obj->temp = NULL;
            //     }
            // }
            if (img_w == _input_size.width() && img_h == _input_size.height())
            {
                return;
            }
            if (fit == maix::image::FIT_FILL)
            {
                *scale_w = (float)img_w / _input_size.width();
                *scale_h = (float)img_h / _input_size.height();
                for (nn::Object *obj : objs)
                {
                    obj->x *= *scale_w;
                    obj->y *= *scale_h;
                    obj->w *= *scale_w;
                    obj->h *= *scale_h;
                    for (size_t i = 0; i < obj->points.size() / 2; ++i)
                    {
                        obj->points.at(i * 2) *= *scale_w;
                        obj->points.at(i * 2 + 1) *= *scale_h;
                    }
                    CORRECT_BBOX_RANGE_YOLO11_2(obj);
                    // if (_type == YOLO11_Type::SEG)
                    // {
                    //     if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                    //     {
                    //         image::Image *old = obj->seg_mask;
                    //         obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                    //         delete old;
                    //     }
                    // }
                }
            }
            else if (fit == maix::image::FIT_CONTAIN)
            {
                *scale_w = ((float)_input_size.width()) / img_w;
                *scale_h = ((float)_input_size.height()) / img_h;
                float scale = std::min(*scale_w, *scale_h);
                float scale_reverse = 1.0 / scale;
                *scale_w = scale_reverse;
                *scale_h = scale_reverse;
                float pad_w = (_input_size.width() - img_w * scale) / 2.0;
                float pad_h = (_input_size.height() - img_h * scale) / 2.0;
                for (nn::Object *obj : objs)
                {
                    obj->x = (obj->x - pad_w) * scale_reverse;
                    obj->y = (obj->y - pad_h) * scale_reverse;
                    obj->w *= scale_reverse;
                    obj->h *= scale_reverse;
                    for (size_t i = 0; i < obj->points.size() / 2; ++i)
                    {
                        obj->points.at(i * 2) = (obj->points.at(i * 2) - pad_w) * scale_reverse;
                        obj->points.at(i * 2 + 1) = (obj->points.at(i * 2 + 1) - pad_h) * scale_reverse;
                    }
                    CORRECT_BBOX_RANGE_YOLO11_2(obj);
                    // if (_type == YOLO11_Type::SEG)
                    // {
                    //     if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                    //     {
                    //         image::Image *old = obj->seg_mask;
                    //         obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                    //         delete old;
                    //     }
                    // }
                }
            }
            else if (fit == maix::image::FIT_COVER)
            {
                *scale_w = ((float)_input_size.width()) / img_w;
                *scale_h = ((float)_input_size.height()) / img_h;
                float scale = std::max(*scale_w, *scale_h);
                float scale_reverse = 1.0 / scale;
                *scale_w = scale_reverse;
                *scale_h = scale_reverse;
                float pad_w = (img_w * scale - _input_size.width()) / 2.0;
                float pad_h = (img_h * scale - _input_size.height()) / 2.0;
                for (nn::Object *obj : objs)
                {
                    obj->x = (obj->x + pad_w) * scale_reverse;
                    obj->y = (obj->y + pad_h) * scale_reverse;
                    obj->w *= scale_reverse;
                    obj->h *= scale_reverse;
                    for (size_t i = 0; i < obj->points.size() / 2; ++i)
                    {
                        obj->points.at(i * 2) = (obj->points.at(i * 2) - pad_w) * scale_reverse;
                        obj->points.at(i * 2 + 1) = (obj->points.at(i * 2 + 1) - pad_h) * scale_reverse;
                    }
                    CORRECT_BBOX_RANGE_YOLO11_2(obj);
                    // if (_type == YOLO11_Type::SEG)
                    // {
                    //     if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                    //     {
                    //         image::Image *old = obj->seg_mask;
                    //         obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                    //         delete old;
                    //     }
                    // }
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

        static float _softmax(const float* src, float* dst, int length)
        {
            const float alpha = *std::max_element(src, src + length);
            float denominator = 0;
            float dis_sum = 0;
            for (int i = 0; i < length; ++i)
            {
                dst[i] = exp(src[i] - alpha);
                denominator += dst[i];
            }
            for (int i = 0; i < length; ++i)
            {
                dst[i] /= denominator;
                dis_sum += i * dst[i];
            }
            return dis_sum;
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

        static void _labels_to_tokens(std::vector<std::string> &labels, int32_t *tokens, int labels_num, int label_length)
        {
            // TODO:
        }

    private:
        bool _dual_buff;
        int _labels_num;
        nn::NN *_model;
        std::string _text_feature_path;
        tensor::Tensor _text_feature;
        std::string _type_str = "yolo-world";
        image::Size _input_size;
        int _text_feature_num;
        image::Format _input_img_fmt;
        bool _out_chw;
        int  _reg_max = 16;
        std::vector<float> _stride = {8, 16, 32};
        int _anchor_num = 0;
        _OutIdxes _out_idxes;
        int _image_in_idx = 0;
        int _text_in_idx = 0;
        std::vector<std::string> _input_names;
        float *_input_text_feature = nullptr;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
    };

} // namespace maix::nn
