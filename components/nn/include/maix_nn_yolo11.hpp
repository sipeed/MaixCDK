/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.10.10: Add yolo11 support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>
#include <omp.h>

namespace maix::nn
{
    enum class YOLO11_Type
    {
        DETECT = 0,
        POSE = 1,
        SEG = 2,
        OBB = 3
    };

    class _KpInfoYolo11
    {
    public:
        _KpInfoYolo11(int idx, int ax, int ay, float stride)
            : idx(idx), anchor_x(ax), anchor_y(ay), stride(stride)
        {
        }
        int idx;
        int anchor_x;
        int anchor_y;
        float stride;
    };

    struct _OutIdxes
    {
        // mode 1
        int det0;     // 1x144x80x80
        int det1;     // 1x144x40x40
        int det2;     // 1x144x20x20

        // mode 2
        int dfl;      // 1x1x4x8400
        int sigmoid;  // 1x80x8400

        // optional node
        // seg mask        1x32x160x160
        // seg mask weight 1x32x8400
        int seg_mask;
        int seg_mask_weight;

        // obb  1x1x8400
        int obb_angle;
        // pose 1x51x8400
        int pose;
    };

    /**
     * YOLO11 class
     * @maixpy maix.nn.YOLO11
     */
    class YOLO11
    {
    public:
        /**
         * Constructor of YOLO11 class
         * @param model model path, default empty, you can load model later by load function.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.YOLO11.__init__
         * @maixcdk maix.nn.YOLO11.YOLO11
         */
        YOLO11(const string &model = "", bool dual_buff = true)
        {
            _model = nullptr;
            _type = YOLO11_Type::DETECT;
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

        YOLO11(const string &model, const std::string &type_str, bool dual_buff = true)
        {
            this->type_str = type_str;
            _model = nullptr;
            _type = YOLO11_Type::DETECT;
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

        ~YOLO11()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
        }

        std::string type()
        {
            return _extra_info["type"];
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.YOLO11.load
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
                if (_extra_info["model_type"] != this->type_str)
                {
                    log::error("model_type not match, expect '%s', but got '%s'", this->type_str.c_str(), _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: %s", this->type_str.c_str());
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
            err::Err e = _model->extra_info_labels(labels);
            if(e == err::Err::ERR_NONE)
            {
                log::print(log::LogLevel::LEVEL_INFO, "\tlabels num: %ld\n", labels.size());
            }
            else
            {
                log::error("labels key not found: %s", err::to_str(e).c_str());
                return err::ERR_ARGS;
            }
            if (_extra_info.find("type") != _extra_info.end())
            {
                if (_extra_info["type"] == "pose")
                {
                    _type = YOLO11_Type::POSE;
                }
                else if (_extra_info["type"] == "seg")
                {
                    _type = YOLO11_Type::SEG;
                }
                else if (_extra_info["type"] == "obb")
                {
                    _type = YOLO11_Type::OBB;
                }
                else if (_extra_info["type"] != "detector")
                {
                    log::error("type [%s] not support, suport [detector, pose, seg, obb]", _extra_info["type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                _type = YOLO11_Type::DETECT;
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            if(inputs[0].shape[3] <= 4) // nhwc
                _input_size = image::Size(inputs[0].shape[2], inputs[0].shape[1]);
            else
                _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print(log::LogLevel::LEVEL_INFO, "\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());

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
            // mode 1
            if((outputs.size() == 5 && _type == YOLO11_Type::SEG) ||
            (outputs.size() == 4 && (_type == YOLO11_Type::OBB || _type == YOLO11_Type::POSE)) ||
            (outputs.size() == 3 && _type == YOLO11_Type::DETECT))
            {
                _out_node_mode = 1;
                int ch_num = labels.size() + _reg_max * 4;
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
                    int shape_size = item.shape.size();
                    if(shape_size == 4 && item.shape[3] == 1)
                        shape_size = 3;
                    if(shape_size == 4)
                    {
                        if(_type == YOLO11_Type::SEG &&
                            ((item.shape[1] == 32 && // 1 32 160 160
                                item.shape[2] == _input_size.height() / 4 &&
                                item.shape[3] == _input_size.width() / 4) || // 1 160 160 32
                            (item.shape[3] == 32 &&
                                item.shape[1] == _input_size.height() / 4 &&
                                item.shape[2] == _input_size.width() / 4)
                            ))
                        {
                            _out_idxes.seg_mask = i;
                        }
                        else if(item.shape[_out_chw ? 2 : 1] == _input_size.height() / _stride[0])
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
                    else if(shape_size == 3)
                    {
                        if (_type == YOLO11_Type::SEG)
                            _out_idxes.seg_mask_weight = i;
                        else if (_type == YOLO11_Type::OBB)
                        {
                            if(item.name.find("Sigmoid") == std::string::npos)
                                _obb_need_sigmoid = true;
                            else
                                _obb_need_sigmoid = false;
                            _out_idxes.obb_angle = i;
                        }
                        else if (_type == YOLO11_Type::POSE)
                            _out_idxes.pose = i;
                        else
                        {
                            log::error("output node shape error, please check %d", 2);
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
            }
            else // mode 2
            {
                _out_node_mode = 2;
                memset(&_out_idxes, 999, sizeof(_out_idxes));
                for(int i = 0; i < (int)outputs.size(); ++i)
                {
                    nn::LayerInfo &item = outputs[i];
                    if(item.shape.size() == 4)
                    {
                        if(item.shape[1] == 4 && item.shape[2] == _anchor_num && item.shape[3] == 1) // hwc
                        {
                            _out_chw = false;
                            _out_idxes.dfl = i;
                            break;
                        }

                    }
                }
                for(int i = 0; i < (int)outputs.size(); ++i)
                {
                    nn::LayerInfo &item = outputs[i];
                    if(i == _out_idxes.dfl)
                        continue;
                    if(_type == YOLO11_Type::DETECT)
                    {
                        if(item.shape[_out_chw ? 1 : 2] == (int)labels.size() && item.shape[_out_chw ? 2 : 1] == _anchor_num)
                            _out_idxes.sigmoid = i;
                        else
                            _out_idxes.dfl = i;
                    }
                    else if(_type == YOLO11_Type::SEG)
                    {
                        if(item.shape[_out_chw ? 1 : 3] == 1 && item.shape[_out_chw ? 2 : 1] == 4 && item.shape[_out_chw ? 3 : 2] == _anchor_num)
                            _out_idxes.dfl = i;
                        else if(item.shape[_out_chw ? 1 : 2] == 32 && item.shape[_out_chw ? 2 : 1] == _anchor_num && (item.shape.size() == 3 || item.shape[3] == 1))
                        {
                            if(item.name.find("Sigmoid") != std::string::npos)
                                _out_idxes.sigmoid = i;
                            else
                                _out_idxes.seg_mask_weight = i;
                        }
                        else if (item.shape[_out_chw ? 1 : 2] == (int)labels.size() && item.shape[_out_chw ? 2 : 1] == _anchor_num && (item.shape.size() == 3 || item.shape[3] == 1))
                            _out_idxes.sigmoid = i;
                        else
                        _out_idxes.seg_mask = i;
                    }
                    else if(_type == YOLO11_Type::OBB)
                    {
                        if(item.shape[_out_chw ? 1 : 3] == 1 && item.shape[_out_chw ? 2 : 1] == 4 && item.shape[_out_chw ? 3 : 2] == _anchor_num)
                            _out_idxes.dfl = i;
                        else if(item.shape[_out_chw ? 1 : 2] > 1)
                            _out_idxes.sigmoid = i;
                        else if(item.name.find("Sigmoid_1") != std::string::npos)
                            _out_idxes.sigmoid = i;
                        else
                        {
                            if(item.name.find("Sigmoid") == std::string::npos)
                                _obb_need_sigmoid = true;
                            else
                                _obb_need_sigmoid = false;
                            _out_idxes.obb_angle = i;
                        }
                    }
                    else if(_type == YOLO11_Type::POSE)
                    {
                        if(item.shape[_out_chw ? 1 : 3] == 1 && item.shape[_out_chw ? 2 : 1] == 4 && item.shape[_out_chw ? 3 : 2] == _anchor_num)
                            _out_idxes.dfl = i;
                        else if((item.shape[_out_chw ? 1 : 2] % 3 == 0)&& item.shape[_out_chw ? 2 : 1] == _anchor_num)
                            _out_idxes.pose = i;
                        else
                            _out_idxes.sigmoid = i;
                    }
                    else
                    {
                        log::error("not implement yet");
                        print_outputs();
                        return err::ERR_NOT_IMPL;
                    }
                }
                // check node
                if (_out_idxes.dfl == 999 || _out_idxes.sigmoid == 999)
                {
                    print_outputs();
                    log::error("can't find DFL or sigmoid output, now: %d, %d", _out_idxes.dfl, _out_idxes.sigmoid);
                    return err::ERR_ARGS;
                }
                switch(_type)
                {
                    case YOLO11_Type::SEG:
                        if (_out_idxes.seg_mask_weight == 999 || _out_idxes.seg_mask == 999)
                        {
                            print_outputs();
                            log::error("can't find seg_mask_weight or seg_mask output, now: %d, %d", _out_idxes.seg_mask_weight, _out_idxes.seg_mask);
                            return err::ERR_ARGS;
                        }
                        break;
                    case YOLO11_Type::OBB:
                        if (_out_idxes.obb_angle == 999)
                        {
                            print_outputs();
                            log::error("can't find obb_angle, now: %d, %d", _out_idxes.obb_angle);
                            return err::ERR_ARGS;
                        }
                        break;
                    case YOLO11_Type::POSE:
                        if (_out_idxes.pose == 999)
                        {
                            print_outputs();
                            log::error("can't find pose, now: %d, %d", _out_idxes.pose);
                            return err::ERR_ARGS;
                        }
                        break;
                    case YOLO11_Type::DETECT:
                    default:
                    break;
                        log::error("not implement yet");
                        return err::ERR_NOT_IMPL;
                        break;
                }
            }
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Confidence threshold, default 0.5.
         * @param iou_th IoU threshold, default 0.45.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @param keypoint_th keypoint threshold, default 0.5, only for yolo11-pose model.
         * @param sort sort result according to object size, default 0 means not sort, 1 means bigger in front, -1 means smaller in front.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         *         If model is yolo11-pose, object's points have value, and if points' value < 0 means that point is invalid(conf < keypoint_th).
         * @maixpy maix.nn.YOLO11.detect
         */
        nn::Objects *detect(image::Image &img, float conf_th = 0.5, float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN, float keypoint_th = 0.5, int sort = 0)
        {
#define SHOW_DETECT_TIME 0
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            this->_keypoint_th = keypoint_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
#if SHOW_DETECT_TIME
            uint64_t start = time::ticks_ms();
#endif
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false);
#if SHOW_DETECT_TIME
            log::info("forward time: %ld", time::ticks_ms() - start);
            start = time::ticks_ms();
#endif
            if (!outputs) // not ready, return empty result.
            {
                return new nn::Objects();
            }
            nn::Objects *res = _post_process(outputs, img.width(), img.height(), fit, sort);
#if SHOW_DETECT_TIME
            log::info("postprocess time: %ld", time::ticks_ms() - start);
#endif
            delete outputs;
            if(!res)
            {
                throw err::Exception("post process failed, please see log before");
            }
            return res;
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.YOLO11.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.YOLO11.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.YOLO11.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLO11.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Draw pose keypoints on image
         * @param img image object, maix.image.Image type.
         * @param points keypoits, int list type, [x, y, x, y ...]
         * @param radius radius of points.
         * @param color color of points.
         * @param colors assign colors for points, list type, element is image.Color object.
         * @param body true, if points' length is 17*2 and body is ture, will draw lines as human body, if set to false won't draw lines, default true.
         * @param close connect all points to close a polygon, default false.
         * @maixpy maix.nn.YOLO11.draw_pose
         */
        void draw_pose(image::Image &img, std::vector<int> points, int radius = 4, image::Color color = image::COLOR_RED, const std::vector<image::Color> &colors = std::vector<image::Color>(), bool body = true, bool close = false)
        {
            bool line_drawed = false;
            if (points.size() < 2 || points.size() % 2 != 0)
            {
                throw std::runtime_error("keypoints size must >= 2 and multiple of 2");
                return;
            }
            if (points.size() == 17 * 2 && body)
            {
                int pos[] = {9, 7, 7, 5, 6, 8, 8, 10, 5, 11, 6, 12, 5, 6, 11, 12, 11, 13, 15, 13, 14, 12, 14, 16};
                for (int i = 0; i < 12; ++i)
                {
                    int x1 = points[pos[i * 2] * 2];
                    int y1 = points[pos[i * 2] * 2 + 1];
                    int x2 = points[pos[i * 2 + 1] * 2];
                    int y2 = points[pos[i * 2 + 1] * 2 + 1];
                    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
                        continue;
                    img.draw_line(x1, y1, x2, y2, color, 2);
                }
                int x = (points[5 * 2] + points[6 * 2]) / 2;
                int y = (points[5 * 2 + 1] + points[6 * 2 + 1]) / 2;
                if (!(points[5 * 2] < 0 || points[5 * 2 + 1] < 0 || points[6 * 2] < 0 || points[6 * 2 + 1] < 0 || x < 0 || y < 0 || points[0] < 0 || points[1] < 0))
                    img.draw_line(points[0], points[1], x, y, color, 2);
                line_drawed = true;
            }
            for (size_t i = 0; i < points.size() / 2; ++i)
            {
                int x = points[i * 2];
                int y = points[i * 2 + 1];
                if (x < 0 || y < 0)
                    continue;
                auto &_color = color;
                if (colors.size() > i)
                {
                    _color = colors[i];
                }
                img.draw_circle(x, y, radius, _color, -1);
            }
            if(close && !line_drawed)
            {
                for (size_t i = 0; i < points.size() / 2; ++i)
                {
                    int x1 = points[i * 2];
                    int y1 = points[i * 2 + 1];
                    int x2 = points[(i + 1) % (points.size() / 2) * 2];
                    int y2 = points[(i + 1) % (points.size() / 2) * 2 + 1];
                    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
                        continue;
                    img.draw_line(x1, y1, x2, y2, color, 2);
                }
            }
        }

        /**
         * Draw segmentation on image
         * @param img image object, maix.image.Image type.
         * @param seg_mask segmentation mask image by detect method, a grayscale image
         * @param threshold only mask's value > threshold will be draw on image, value from 0 to 255.
         * @maixpy maix.nn.YOLO11.draw_seg_mask
         */
        void draw_seg_mask(image::Image &img, int x, int y, image::Image &seg_mask, int threshold = 127)
        {
            if (seg_mask.format() != image::FMT_GRAYSCALE)
            {
                throw err::Exception(err::ERR_ARGS, "seg_mask only support grascale");
            }
            if (img.format() != image::FMT_RGB888 &&
                img.format() != image::FMT_BGR888 &&
                img.format() != image::FMT_RGBA8888 &&
                img.format() != image::FMT_BGRA8888 &&
                img.format() != image::FMT_GRAYSCALE)
            {
                throw err::Exception(err::ERR_ARGS, "img not support");
            }
            int fmt_size = image::fmt_size[img.format()];
            uint8_t *to = (uint8_t*)img.data();
            uint8_t *from = (uint8_t*)seg_mask.data();
            for (int i = 0; i < seg_mask.height(); ++i)
            {
                for (int j = 0; j < seg_mask.width(); ++j)
                {
                    uint8_t data = from[i * seg_mask.width() + j];
                    if(data > threshold)
                    {
                        to[((y + i) * img.width() + x + j) * fmt_size] = data;
                    }
                }
            }
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLO11.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.YOLO11.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLO11.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLO11.scale
         */
        std::vector<float> scale;

    protected:
        std::string type_str = "yolo11";

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        float _iou_th = 0.45;
        float _keypoint_th = 0.5;
        YOLO11_Type _type;
        bool _dual_buff;
        _OutIdxes _out_idxes;
        int _out_node_mode;
        bool _out_chw;
        int  _reg_max = 16;
        std::vector<float> _stride = {8, 16, 32};
        int _anchor_num = 0;
        bool _obb_need_sigmoid;

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
            // decode keypoints
            if (_type == YOLO11_Type::POSE)
            {
                _decode_keypoints(*objects, kp_out);
            }
            else if (_type == YOLO11_Type::SEG)
            {
                _decode_seg_points(*objects, kp_out, mask_out);
            }
            if (objects->size() > 0)
            {
                _correct_bbox(*objects, img_w, img_h, fit, &scale_w, &scale_h);
            }
            return objects;
        }

        bool _decode_objs(nn::Objects &objs, tensor::Tensors *outputs, float conf_thresh, int w, int h, tensor::Tensor **kp_out, tensor::Tensor **mask_out)
        {
            if(_type == YOLO11_Type::SEG)
            {
                *mask_out = &(*outputs)[_out_idxes.seg_mask];
                *kp_out = &(*outputs)[_out_idxes.seg_mask_weight];
            }
            else if (_type == YOLO11_Type::OBB)
            {
                *kp_out = &(*outputs)[_out_idxes.obb_angle];
            }
            else if (_type == YOLO11_Type::POSE)
            {
                *kp_out = &(*outputs)[_out_idxes.pose];
            }
            int idx_start[3] = {
                0,
                (int)(h / _stride[0] * w / _stride[0]),
                (int)(h / _stride[0] * w / _stride[0] + h / _stride[1] * w / _stride[1])};
            // detect
            if(_out_node_mode == 1)
            {
                tensor::Tensor *dets[3] = {&(*outputs)[_out_idxes.det0], &(*outputs)[_out_idxes.det1], &(*outputs)[_out_idxes.det2]};
                if (_type == YOLO11_Type::OBB)
                {
                    if(_out_chw)
                    {
                        int class_num = (int)labels.size();
                        int prob_offset = _reg_max * 4;
                        float* angle_data = (float*)(*kp_out)->data(); // CHW, same layout

                        #pragma omp parallel for
                        for (int index = 0; index < _anchor_num; ++index)
                        {
                            int i = 1;
                            int anchor_idx;
                            if (index >= idx_start[2]) {
                                i = 2;
                                anchor_idx = index - idx_start[2];
                            } else if (index < idx_start[1]) {
                                i = 0;
                                anchor_idx = index;
                            } else {
                                anchor_idx = index - idx_start[1];
                            }

                            int nh = dets[i]->shape()[2]; // H
                            int nw = dets[i]->shape()[3]; // W
                            int s = nh * nw;
                            float* feature = (float*)dets[i]->data(); // shape: (1, C, H, W)

                            int ax = anchor_idx % nw;
                            int ay = anchor_idx / nw;
                            int offset = idx_start[i] + anchor_idx;
                            float* p = feature + anchor_idx;

                            // class
                            int class_id = _argmax(p + prob_offset * s, class_num, s);
                            float score = _sigmoid(p[(prob_offset + class_id) * s]);
                            if (score <= conf_thresh)
                                continue;

                            float dis[4];
                            for(int k = 0; k < 4; ++k)
                            {
                                // float softmax_res[_reg_max];
                                float* p_dis = p + k * _reg_max * s;
                                // dis[k] = _softmax_step(p_dis, softmax_res, _reg_max, s);
                                dis[k] = _softmax_expectation(p_dis, _reg_max, s);
                            }


                            float lt_x = dis[0];
                            float lt_y = dis[1];
                            float rb_x = dis[2];
                            float rb_y = dis[3];

                            // angle
                            float angle_val = angle_data[offset];
                            float angle = (_obb_need_sigmoid ? _sigmoid(angle_val) : angle_val) - 0.25f;
                            float angle_rad = angle * M_PI;

                            float stride = _stride[i];
                            float cos_angle = cosf(angle_rad);
                            float sin_angle = sinf(angle_rad);
                            float xf = (rb_x - lt_x) / 2.f;
                            float yf = (rb_y - lt_y) / 2.f;
                            float bbox_w = (lt_x + rb_x) * stride;
                            float bbox_h = (lt_y + rb_y) * stride;
                            float bbox_x = ((xf * cos_angle - yf * sin_angle) + ax + 0.5f) * stride - bbox_w * 0.5f;
                            float bbox_y = ((xf * sin_angle + yf * cos_angle) + ay + 0.5f) * stride - bbox_h * 0.5f;

                            _KpInfoYolo11* kp_info = new _KpInfoYolo11(offset, ax, ay, stride);
                            #pragma omp critical
                            {
                                Object& obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, score, {}, angle);
                                obj.temp = (void*)kp_info;
                            }
                        }
                    }
                    else
                    {
                        int class_num = (int)labels.size();
                        int prob_offset = _reg_max * 4;
                        int batch_data_size = prob_offset + class_num;
                        float *angle_ptr = (float *)(*kp_out)->data();
                        // for(int i = 0; i < 3; ++i) // 1 x nh x nw x (_reg_max * 4 + class_num)
                        // {
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
                                int offset = idx_start[i] + anchor_idx;
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
                                float lt_x = dis[0];
                                float lt_y = dis[1];
                                float rb_x = dis[2];
                                float rb_y = dis[3];
                                float angle = ((_obb_need_sigmoid ? _sigmoid(angle_ptr[offset]) : angle_ptr[offset]) - 0.25);
                                float angle_rad = angle * M_PI;
                                float cos_angle = cosf(angle_rad);
                                float sin_angle = sinf(angle_rad);
                                float xf = (rb_x - lt_x) / 2.0;
                                float yf = (rb_y - lt_y) / 2.0;
                                float bbox_w = (lt_x + rb_x) * _stride[i];
                                float bbox_h = (lt_y + rb_y) * _stride[i];
                                float bbox_x = ((xf * cos_angle - yf * sin_angle) + ax + 0.5) * _stride[i] - bbox_w * 0.5;
                                float bbox_y = ((xf * sin_angle + yf * cos_angle) + ay + 0.5) * _stride[i] - bbox_h * 0.5;
                                _KpInfoYolo11 *kp_info = new _KpInfoYolo11(offset, ax, ay, _stride[i]);
                                #pragma omp critical
                                {
                                    Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, score, {}, angle);
                                    obj.temp = (void *)kp_info;
                                }
                            }
                        // }
                    }
                }
                else
                {
                    if(_out_chw)
                    {
                        int class_num = (int)labels.size();
                        int prob_offset = _reg_max * 4;
                        struct LayerInfo {
                            int nh;
                            int nw;
                            float* feature;
                        };

                        LayerInfo layer_info[3];
                        for (int i = 0; i < 3; ++i)
                        {
                            layer_info[i].nh = dets[i]->shape()[2];
                            layer_info[i].nw = dets[i]->shape()[3];
                            layer_info[i].feature = (float*)dets[i]->data();
                        }


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
                            } else {
                                anchor_idx = index - idx_start[1];
                            }

                            int nh = layer_info[i].nh;
                            int nw = layer_info[i].nw;
                            float* feature = layer_info[i].feature;

                            int ax = anchor_idx % nw;
                            int ay = anchor_idx / nw;

                            int s = nh * nw;
                            float *p = feature + anchor_idx;

                            int class_id = _argmax(p + prob_offset * s, class_num, s);
                            float score = _sigmoid(p[prob_offset * s + class_id * s]);

                            if (score <= conf_thresh)
                                continue;

                            float dis[4];
                            for(int k = 0; k < 4; ++k)
                            {
                                // float softmax_res[_reg_max];
                                float* p_dis = p + k * _reg_max * s;
                                // dis[k] = _softmax_step(p_dis, softmax_res, _reg_max, s);
                                dis[k] = _softmax_expectation(p_dis, _reg_max, s);
                            }

                            float bbox_x = (ax + 0.5 - dis[0]) * _stride[i];
                            float bbox_y = (ay + 0.5 - dis[1]) * _stride[i];
                            float bbox_w = (ax + 0.5 + dis[2]) * _stride[i] - bbox_x;
                            float bbox_h = (ay + 0.5 + dis[3]) * _stride[i] - bbox_y;

                            _KpInfoYolo11 *kp_info = new _KpInfoYolo11(idx_start[i] + anchor_idx, ax, ay, _stride[i]);
                            #pragma omp critical
                            {
                                Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, score);
                                obj.temp = (void *)kp_info;
                            }
                        }
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
                                    continue;

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
                                _KpInfoYolo11 *kp_info = new _KpInfoYolo11(idx_start[i] + anchor_idx, ax, ay, _stride[i]);
                                #pragma omp critical
                                {
                                    Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, score);
                                    obj.temp = (void *)kp_info;
                                }
                            }
                        // }
                    }
                }
            }
            else // mode 2
            {
                tensor::Tensor *score_out = NULL; // shape 1, 80, 8400       hwc: 1, 8400, 80
                tensor::Tensor *box_out = NULL;   // shape 1,  1,    4, 8400 hwc: 1,    1, 8400, 4
                score_out = &(*outputs)[_out_idxes.sigmoid];
                box_out = &(*outputs)[_out_idxes.dfl];
                int class_num = (int)labels.size();
                float *scores_ptr = (float *)score_out->data();
                float *dets_ptr = (float *)box_out->data();
                if (_type == YOLO11_Type::OBB)
                {
                    if(!_out_chw)
                    {
                        throw err::Exception(err::ERR_NOT_IMPL, "not support output hwc layout");
                    }
                    float *angle_ptr = (float *)(*kp_out)->data();
                    for (int i = 0; i < 3; i++)
                    {
                        int nh = h / _stride[i];
                        int nw = w / _stride[i];
                        for (int ay = 0; ay < nh; ++ay)
                        {
                            for (int ax = 0; ax < nw; ++ax)
                            {
                                int offset = idx_start[i] + ay * nw + ax;
                                int class_id = _argmax(scores_ptr + offset, class_num, _anchor_num);
                                // int max_idx = _argmax2(scores_ptr + offset, class_num * _anchor_num - offset, _anchor_num);
                                // int class_id = (offset + max_idx) / _anchor_num;
                                float obj_score = scores_ptr[offset + class_id * _anchor_num];
                                if (obj_score <= conf_thresh)
                                {
                                    continue;
                                }
                                float angle = ((_obb_need_sigmoid ? _sigmoid(angle_ptr[offset]) : angle_ptr[offset]) - 0.25);
                                float angle_rad = angle * M_PI;
                                float cos_angle = cosf(angle_rad);
                                float sin_angle = sinf(angle_rad);
                                float lt_x = dets_ptr[offset];
                                float lt_y = dets_ptr[offset + _anchor_num];
                                float rb_x = dets_ptr[offset + _anchor_num * 2];
                                float rb_y = dets_ptr[offset + _anchor_num * 3];
                                float xf = (rb_x - lt_x) / 2.0;
                                float yf = (rb_y - lt_y) / 2.0;
                                float bbox_w = (lt_x + rb_x) * _stride[i];
                                float bbox_h = (lt_y + rb_y) * _stride[i];
                                float bbox_x = ((xf * cos_angle - yf * sin_angle) + ax + 0.5) * _stride[i] - bbox_w * 0.5;
                                float bbox_y = ((xf * sin_angle + yf * cos_angle) + ay + 0.5) * _stride[i] - bbox_h * 0.5;
                                _KpInfoYolo11 *kp_info = new _KpInfoYolo11(offset, ax, ay, _stride[i]);
                                Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score, {}, angle);
                                obj.temp = (void *)kp_info;
                            }
                        }
                    }
                }
                else
                {
                    if(_out_chw)
                    {
                        for (int i = 0; i < 3; i++)
                        {
                            int nh = h / _stride[i];
                            int nw = w / _stride[i];
                            for (int ay = 0; ay < nh; ++ay)
                            {
                                for (int ax = 0; ax < nw; ++ax)
                                {
                                    int offset = idx_start[i] + ay * nw + ax;
                                    int class_id = _argmax(scores_ptr + offset, class_num, _anchor_num);
                                    // int max_idx = _argmax2(scores_ptr + offset, class_num * _anchor_num - offset, _anchor_num);
                                    // int class_id = (offset + max_idx) / _anchor_num;
                                    float obj_score = scores_ptr[offset + class_id * _anchor_num];
                                    if (obj_score <= conf_thresh)
                                    {
                                        continue;
                                    }
                                    float bbox_x = (ax + 0.5 - dets_ptr[offset]) * _stride[i];
                                    float bbox_y = (ay + 0.5 - dets_ptr[offset + _anchor_num]) * _stride[i];
                                    float bbox_w = (ax + 0.5 + dets_ptr[offset + _anchor_num * 2]) * _stride[i] - bbox_x;
                                    float bbox_h = (ay + 0.5 + dets_ptr[offset + _anchor_num * 3]) * _stride[i] - bbox_y;
                                    _KpInfoYolo11 *kp_info = new _KpInfoYolo11(offset, ax, ay, _stride[i]);
                                    Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score);
                                    obj.temp = (void *)kp_info;
                                }
                            }
                        }
                    }
                    else
                    {
                        typedef struct
                        {
                            float bbox_x;
                            float bbox_y;
                            float bbox_w;
                            float bbox_h;
                            int class_id;
                            float score;
                        } valid_item_t;
                        for (int i = 0; i < 3; i++)
                        {
                            int nh = h / _stride[i];
                            int nw = w / _stride[i];
                            for (int ay = 0; ay < nh; ++ay)
                            {
                                for (int ax = 0; ax < nw; ++ax)
                                {
                                    valid_item_t item;
                                    int offset = idx_start[i] + ay * nw + ax;
                                    float *p = scores_ptr + offset * class_num;
                                    item.class_id = _argmax(p, class_num, 1);
                                    item.score = p[item.class_id];
                                    if (item.score <= conf_thresh)
                                    {
                                        continue;
                                    }
                                    item.bbox_x = (ax + 0.5 - dets_ptr[offset * 4]) * _stride[i];
                                    item.bbox_y = (ay + 0.5 - dets_ptr[offset * 4 + 1]) * _stride[i];
                                    item.bbox_w = (ax + 0.5 + dets_ptr[offset * 4 + 2]) * _stride[i] - item.bbox_x;
                                    item.bbox_h = (ay + 0.5 + dets_ptr[offset * 4 + 3]) * _stride[i] - item.bbox_y;
                                    _KpInfoYolo11 *kp_info = new _KpInfoYolo11(offset, ax, ay, _stride[i]);
                                    Object &obj = objs.add(item.bbox_x, item.bbox_y, item.bbox_w, item.bbox_h, item.class_id, item.score);
                                    obj.temp = (void *)kp_info;
                                }
                            }
                        }
                    }
                }
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
                    delete (_KpInfoYolo11 *)a->temp;
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

        void _decode_keypoints(nn::Objects &objs, tensor::Tensor *kp_out)
        {
            float *data = (float *)kp_out->data();
            if(_out_chw)
            {
                int keypoint_num = kp_out->shape()[1] / 3; // 1, 51, 8400
                for (size_t i = 0; i < objs.size(); ++i)
                {
                    nn::Object &o = objs.at(i);
                    _KpInfoYolo11 *kp_info = (_KpInfoYolo11 *)o.temp;
                    float *p = data + kp_info->idx;
                    for (int k = 0; k < keypoint_num; ++k)
                    {
                        float score = _sigmoid(p[(k * 3 + 2) * _anchor_num]);
                        int x = -1;
                        int y = -1;
                        if (score > _keypoint_th)
                        {
                            x = (p[(k * 3) * _anchor_num] * 2.0 + kp_info->anchor_x) * kp_info->stride;
                            y = (p[(k * 3 + 1) * _anchor_num] * 2.0 + kp_info->anchor_y) * kp_info->stride;
                        }
                        o.points.push_back(x);
                        o.points.push_back(y);
                    }
                    delete (_KpInfoYolo11 *)o.temp;
                    o.temp = NULL;
                }
            }
            else
            {
                int keypoint_num = kp_out->shape()[2] / 3; // 1, 8400, 51
                for (size_t i = 0; i < objs.size(); ++i)
                {
                    nn::Object &o = objs.at(i);
                    _KpInfoYolo11 *kp_info = (_KpInfoYolo11 *)o.temp;
                    float *p = data + kp_info->idx * kp_out->shape()[2];
                    for (int k = 0; k < keypoint_num; ++k)
                    {
                        float score = _sigmoid(p[k * 3 + 2]);
                        int x = -1;
                        int y = -1;
                        if (score > _keypoint_th)
                        {
                            x = (p[k * 3] * 2.0 + kp_info->anchor_x) * kp_info->stride;
                            y = (p[k * 3 + 1] * 2.0 + kp_info->anchor_y) * kp_info->stride;
                        }
                        o.points.push_back(x);
                        o.points.push_back(y);
                    }
                    delete (_KpInfoYolo11 *)o.temp;
                    o.temp = NULL;
                }
            }
        }

        void _decode_seg_points(nn::Objects &objs, tensor::Tensor *kp_out, tensor::Tensor *mask_out)
        {
            float *data = (float *)kp_out->data();
            float *mask_data = (float *)mask_out->data();
            if(_out_chw)
            {
                int mask_h = mask_out->shape()[2];  // 1, 32, 160, 160
                int mask_w = mask_out->shape()[3];
                int mask_squre = mask_h * mask_w;
                int mask_num = kp_out->shape()[1];  // 1, 32, 8400
                float mask_weights[mask_num];
                for (size_t i = 0; i < objs.size(); ++i)
                {
                    nn::Object &o = objs.at(i);
                    int mask_x = o.x * mask_w / _input_size.width();
                    int mask_y = o.y * mask_h / _input_size.height();
                    int mask_x2 = (o.x + o.w) * mask_w / _input_size.width();
                    int mask_y2 = (o.y + o.h) * mask_h / _input_size.height();
                    _KpInfoYolo11 *kp_info = (_KpInfoYolo11 *)o.temp;
                    float *p = data + kp_info->idx;
                    for (int k = 0; k < mask_num; ++k)
                    {
                        mask_weights[k] = p[k * _anchor_num];
                    }
                    o.seg_mask = new image::Image(mask_x2 - mask_x, mask_y2 - mask_y, image::Format::FMT_GRAYSCALE);
                    uint8_t *p_img_data = (uint8_t *)o.seg_mask->data();
                    for (int j = mask_y; j < mask_y2; ++j)
                    {
                        for (int k = mask_x; k < mask_x2; ++k)
                        {
                            mask_data[j * mask_w + k] *= mask_weights[0];
                        }
                    }
                    for (int n = 1; n < mask_num; ++n)
                    {
                        for (int j = mask_y; j < mask_y2; ++j)
                        {
                            for (int k = mask_x; k < mask_x2; ++k)
                            {
                                mask_data[j * mask_w + k] += mask_weights[n] * mask_data[n * mask_squre + j * mask_w + k];
                            }
                        }
                    }
                    for (int j = mask_y; j < mask_y2; ++j)
                    {
                        for (int k = mask_x; k < mask_x2; ++k)
                        {
                            *p_img_data++ = (uint8_t)(_sigmoid(mask_data[j * mask_w + k]) * 255);
                        }
                    }
                    delete (_KpInfoYolo11 *)o.temp;
                    o.temp = NULL;
                }
            }
            else
            {
                bool mask_chw = false;
                int mask_h = _input_size.height() / 4;
                int mask_w = _input_size.width() / 4;
                int mask_num = 32;
                int mask_squre = mask_h * mask_w;
                // 1, 32, 160, 160 or 1 160 160 32
                if(mask_out->shape()[1] == mask_num && mask_out->shape()[2] == mask_h && mask_out->shape()[3] == mask_w)
                {
                    mask_chw = true;
                }
                for (size_t i = 0; i < objs.size(); ++i)
                {
                    nn::Object &o = objs.at(i);
                    int mask_x = o.x * mask_w / _input_size.width();
                    int mask_y = o.y * mask_h / _input_size.height();
                    int mask_x2 = (o.x + o.w) * mask_w / _input_size.width();
                    int mask_y2 = (o.y + o.h) * mask_h / _input_size.height();
                    _KpInfoYolo11 *kp_info = (_KpInfoYolo11 *)o.temp;
                    float *mask_weights = data + kp_info->idx * mask_num; //1 8400 32
                    o.seg_mask = new image::Image(mask_x2 - mask_x, mask_y2 - mask_y, image::Format::FMT_GRAYSCALE);
                    uint8_t *p_img_data = (uint8_t *)o.seg_mask->data();
                    if(mask_chw) // 1 32 160 160
                    {
                        for (int j = mask_y; j < mask_y2; ++j)
                        {
                            for (int k = mask_x; k < mask_x2; ++k)
                            {
                                mask_data[j * mask_w + k] *= mask_weights[0];
                            }
                        }
                        for (int n = 1; n < mask_num; ++n)
                        {
                            for (int j = mask_y; j < mask_y2; ++j)
                            {
                                for (int k = mask_x; k < mask_x2; ++k)
                                {
                                    mask_data[j * mask_w + k] += mask_weights[n] * mask_data[n * mask_squre + j * mask_w + k];
                                }
                            }
                        }
                        for (int j = mask_y; j < mask_y2; ++j)
                        {
                            for (int k = mask_x; k < mask_x2; ++k)
                            {
                                *p_img_data++ = (uint8_t)(_sigmoid(mask_data[j * mask_w + k]) * 255);
                            }
                        }
                    }
                    else // 1 160 160 32
                    {
                        for (int j = mask_y; j < mask_y2; ++j)
                        {
                            for (int k = mask_x; k < mask_x2; ++k)
                            {
                                mask_data[(j * mask_w + k) * mask_num] *= mask_weights[0];
                            }
                        }
                        for (int j = mask_y; j < mask_y2; ++j)
                        {
                            for (int k = mask_x; k < mask_x2; ++k)
                            {
                                for (int n = 1; n < mask_num; ++n)
                                {
                                    mask_data[(j * mask_w + k) * mask_num] += mask_weights[n] * mask_data[(j * mask_w + k) * mask_num + n];
                                }
                            }
                        }
                        for (int j = mask_y; j < mask_y2; ++j)
                        {
                            for (int k = mask_x; k < mask_x2; ++k)
                            {
                                *p_img_data++ = (uint8_t)(_sigmoid(mask_data[(j * mask_w + k) * mask_num]) * 255);
                            }
                        }
                    }
                    delete (_KpInfoYolo11 *)o.temp;
                    o.temp = NULL;
                }
            }
        }

        void _correct_bbox(nn::Objects &objs, int img_w, int img_h, maix::image::Fit fit, float *scale_w, float *scale_h)
        {
#define CORRECT_BBOX_RANGE_YOLO11(obj)      \
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

            for (nn::Object *obj : objs)
            {
                if (obj->temp)
                {
                    delete (_KpInfoYolo11 *)obj->temp;
                    obj->temp = NULL;
                }
            }
            if (img_w == _input_size.width() && img_h == _input_size.height())
            {
                if (_type == YOLO11_Type::SEG)
                {
                    for (nn::Object *obj : objs)
                    {
                        if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                        {
                            image::Image *old = obj->seg_mask;
                            obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                            delete old;
                        }
                    }
                }
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
                    CORRECT_BBOX_RANGE_YOLO11(obj);
                    if (_type == YOLO11_Type::SEG)
                    {
                        if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                        {
                            image::Image *old = obj->seg_mask;
                            obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                            delete old;
                        }
                    }
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
                    CORRECT_BBOX_RANGE_YOLO11(obj);
                    if (_type == YOLO11_Type::SEG)
                    {
                        if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                        {
                            image::Image *old = obj->seg_mask;
                            obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                            delete old;
                        }
                    }
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
                    CORRECT_BBOX_RANGE_YOLO11(obj);
                    if (_type == YOLO11_Type::SEG)
                    {
                        if (obj->w != obj->seg_mask->width() || obj->h != obj->seg_mask->height())
                        {
                            image::Image *old = obj->seg_mask;
                            obj->seg_mask = old->resize(obj->w, obj->h, image::FIT_FILL);
                            delete old;
                        }
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

        template <typename T>
        static int _argmax2(const T *data, size_t end, size_t stride = 1)
        {
            int maxIndex = 0;
            for (size_t i = stride; i < end; i += stride)
            {
                if (data[maxIndex] < data[i])
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

        static float _softmax_step(const float* src, float* dst, int length, int step)
        {
            // 找最大值用于稳定性
            float alpha = src[0];
            for (int i = 1; i < length; ++i)
            {
                float val = src[i * step];
                if (val > alpha)
                    alpha = val;
            }

            // softmax 计算（先减最大值再归一化）
            float denominator = 0.0f;
            for (int i = 0; i < length; ++i)
            {
                dst[i] = std::exp(src[i * step] - alpha);
                denominator += dst[i];
            }

            // 归一化并计算加权和
            float dis_sum = 0.0f;
            for (int i = 0; i < length; ++i)
            {
                dst[i] /= denominator;
                dis_sum += i * dst[i];
            }

            return dis_sum;
        }

        static float _softmax_expectation(const float* src, int length, int step)
        {
            float alpha = src[0];
            for (int i = 1; i < length; ++i)
            {
                float val = src[i * step];
                if (val > alpha)
                    alpha = val;
            }

            float denominator = 0.f;
            float numerator = 0.f;

            for (int i = 0; i < length; ++i)
            {
                float e = std::exp(src[i * step] - alpha);
                denominator += e;
                numerator += i * e;
            }

            return numerator / denominator;
        }

    };

} // namespace maix::nn
