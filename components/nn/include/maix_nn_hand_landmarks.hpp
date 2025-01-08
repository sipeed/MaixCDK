/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.12.27: Add hand keypoints support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_image_cv.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>

#define DRAW_STD_IMG 0

namespace maix::nn
{
    /**
     * HandLandmarks class
     * @maixpy maix.nn.HandLandmarks
     */
    class HandLandmarks
    {
    public:
        /**
         * Constructor of HandLandmarks class
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.HandLandmarks.__init__
         * @maixcdk maix.nn.HandLandmarks.HandLandmarks
         */
        HandLandmarks(const string &model = "")
        {
            _model = nullptr;
            _model_detect = nullptr;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~HandLandmarks()
        {
            if (_model_detect)
            {
                delete _model_detect;
                _model_detect = nullptr;
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
         * @return err::Err
         * @maixpy maix.nn.HandLandmarks.load
         */
        err::Err load(const string &model)
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            _model = new nn::NN(model, false);
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
            err::Err e = _model->extra_info_labels(labels);
            if (e == err::Err::ERR_NONE)
            {
                log::print("\tlabels num: %ld\n", labels.size());
            }
            else
            {
                log::error("labels key not found: %s", err::to_str(e).c_str());
                return err::ERR_ARGS;
            }
            if (_extra_info.find("detect_model") != _extra_info.end())
            {
                std::string model_file = fs::dirname(model) + "/" + _extra_info["detect_model"];
                _model_detect = new nn::NN(model_file, false);
                if (!_model_detect)
                {
                    log::error("load detect model failed");
                    return err::ERR_NO_MEM;
                }
            }
            else
            {
                log::error("detect_model key not found");
                return err::ERR_ARGS;
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            std::vector<nn::LayerInfo> inputs_detect = _model_detect->inputs_info();
            _input_size = image::Size(inputs[0].shape[2], inputs[0].shape[1]);
            _input_size_detect = image::Size(inputs_detect[0].shape[2], inputs_detect[0].shape[1]);
            log::print("\tinput size: %dx%d\n\n", _input_size_detect.width(), _input_size_detect.height());

            // anchors
            if (_extra_info.find("anchors") != _extra_info.end())
            {
                std::string anchors_file = fs::dirname(model) + "/" + _extra_info["anchors"];
                fs::File *f = fs::open(anchors_file, "r");
                if(!f)
                {
                    log::error("open anchor file %s failed", anchors_file.c_str());
                    return err::ERR_ARGS;
                }

                while(1)
                {
                    std::string *line = f->readline();
                    if(!line)
                        break;
                    if(!_parse_anchor_line(*line, _anchors))
                    {
                        log::error("parse_anchor_line %s failed", line->c_str());
                        delete line;
                        delete f;
                        return err::ERR_ARGS;
                    }
                    delete line;
                }
                delete f;
            }
            else
            {
                log::error("detect_model key not found");
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Hand detect confidence threshold, default 0.7.
         * @param iou_th IoU threshold, default 0.45.
         * @param conf_th2 Hand detect confidence second time check threshold, default 0.8.
         * @param landmarks_rel outputs the relative coordinates of 21 points with respect to the top-left vertex of the hand.
         *                      In obj.points, the last 21x2 values are arranged as x0y0x1y1...x20y20.
         *                      Value from 0 to obj.w.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         *         Object's points value format: box_topleft_x, box_topleft_y, box_topright_x, box_topright_y, box_bottomright_x, box_bottomright_y， box_bottomleft_x, box_bottomleft_y,
         *                          x0, y0, z1, x1, y1, z2, ..., x20, y20, z20.
         *         If landmarks_rel is True, will be box_topleft_x, box_topleft_y...,x20,y20,z20,x0_rel,y0_rel,...,x20_rel,y20_rel.
         *         Z is depth, the larger the value, the farther away from the palm, and the positive value means closer to the camera.
         * @maixpy maix.nn.HandLandmarks.detect
         */
        nn::Objects *detect(image::Image &img, float conf_th = 0.7, float iou_th = 0.45, float conf_th2 = 0.8, bool landmarks_rel = false)
        {
            maix::image::Fit fit = maix::image::FIT_CONTAIN;
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            this->_conf_th2 = conf_th2;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            image::Image *detect_img_input = &img;
            bool resized = false;
            if (img.width() != _input_size_detect.width() || img.height() != _input_size_detect.height())
            {
                detect_img_input = img.resize(_input_size_detect.width(), _input_size_detect.height(), fit);
                resized = true;
            }
            nn::Objects *objs = new nn::Objects();
            tensor::Tensors *outputs;
            outputs = _model_detect->forward_image(*detect_img_input, this->mean, this->scale, fit, false, true, false);
            if (resized)
                delete detect_img_input;
            if (!outputs) // not ready, return empty result.
            {
                return objs;
            }
            _decode_objs(*objs, outputs, conf_th, _input_size_detect.width(), _input_size_detect.height(), resized, img.width(), img.height());
            delete outputs;
            if (objs->size() > 0)
            {
                nn::Objects *objects_total = objs;
                objs = _nms(*objs);
                delete objects_total;
            }
            std::vector<cv::Mat> M_inverse;
            std::vector<image::Image *> landmarks_input = _crop_reize_hand(*objs, img, _input_size.width(), _input_size.height(), M_inverse, landmarks_rel);
            #if DRAW_STD_IMG
                int y=0;
            #endif
            bool have_invalid = false;
            for(size_t i=0; i<landmarks_input.size(); ++i)
            {
                #if DRAW_STD_IMG
                    img.draw_image(0, y, *landmarks_input[i]);
                    y += landmarks_input[i]->height();
                #endif
                outputs = _model->forward_image(*landmarks_input[i], this->mean, this->scale, fit, false, true, false);
                delete landmarks_input[i];
                if (!outputs) // not ready, return empty result.
                {
                    return objs;
                }
                have_invalid |= _decode_landmarks(*objs, i, outputs, conf_th2, M_inverse, _input_size.width(), _input_size.height(), img.width(), img.height(), landmarks_rel);
                delete outputs;
            }
            if(have_invalid)
            {
                nn::Objects *res = new nn::Objects();
                for(size_t i = 0; i < objs->size(); ++i)
                {
                    nn::Object &obj = objs->at(i);
                    if(obj.score > 0)
                        res->add(obj);
                }
                delete objs;
                return res;
            }
            return objs;
        }

        /**
         * Get model input size
         * @param detect detect or landmarks model, default true.
         * @return model input size
         * @maixpy maix.nn.HandLandmarks.input_size
         */
        image::Size input_size(bool detect = true)
        {
            if (detect)
                return _input_size_detect;
            return _input_size;
        }

        /**
         * Get model input width
         * @param detect detect or landmarks model, default true.
         * @return model input size of width
         * @maixpy maix.nn.HandLandmarks.input_width
         */
        int input_width(bool detect = true)
        {
            if (detect)
                return _input_size_detect.width();
            return _input_size.width();
        }

        /**
         * Get model input height
         * @param detect detect or landmarks model, default true.
         * @return model input size of height
         * @maixpy maix.nn.HandLandmarks.input_height
         */
        int input_height(bool detect = true)
        {
            if (detect)
                return _input_size_detect.height();
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.HandLandmarks.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Draw hand and landmarks on image
         * @param img image object, maix.image.Image type.
         * @param leftright, 0 means left, 1 means right
         * @param points points result from detect method: box_topleft_x, box_topleft_y, box_topright_x, box_topright_y, box_bottomright_x, box_bottomright_y， box_bottomleft_x, box_bottomleft_y,
         *                          x0, y0, z1, x1, y1, z2, ..., x20, y20, z20
         * @param r_min min radius of points.
         * @param r_max min radius of points.
         * @param box draw box or not, default true.
         * @param box_color color of box.
         * @maixpy maix.nn.HandLandmarks.draw_hand
         */
        void draw_hand(image::Image &img, int leftright, const std::vector<int> &points, int r_min = 4, int r_max = 10, bool box = true, int box_thickness = 1, image::Color box_color_l = image::COLOR_RED, image::Color box_color_r = image::COLOR_GREEN)
        {
            if (points.size() < 71)
            {
                throw std::runtime_error("keypoints size must 72");
            }
            if (box)
            {
                img.draw_line(points[0], points[1], points[2], points[3], leftright == 0 ? box_color_l : box_color_r, box_thickness);
                img.draw_line(points[2], points[3], points[4], points[5], leftright == 0 ? box_color_l : box_color_r, box_thickness);
                img.draw_line(points[4], points[5], points[6], points[7], leftright == 0 ? box_color_l : box_color_r, box_thickness);
                img.draw_line(points[6], points[7], points[0], points[1], leftright == 0 ? box_color_l : box_color_r, box_thickness);
            }
            int max_z = INT_MIN, min_z = INT_MAX;
            for (int i = 0; i < 21; ++i)
            {
                if (points[10 + i * 3] > max_z)
                    max_z = points[10 + i * 3];
                else if (points[10 + i * 3] < min_z)
                    min_z = points[10 + i * 3];
            }
            float range = 1.0 / (max_z - min_z);
            for (int i = 0; i < 5; ++i)
            {
                float radius = (points[10 + i * 3] - min_z) * range;
                uint8_t c = (uint8_t)(radius * 255);
                auto color = image::Color::from_rgb(c, c, c);
                int r = (int)(radius * r_max);
                if (r < r_min)
                    r = r_min;
                img.draw_circle(points[8 + i * 3], points[9 + i * 3], r, color, -1);
            }
            for (int i = 5; i < 9; ++i)
            {
                float radius = (points[10 + i * 3] - min_z) * range;
                auto color = image::Color::from_rgb(radius * 255, 0, 0);
                int r = (int)(radius * r_max);
                if (r < r_min)
                    r = r_min;
                img.draw_circle(points[8 + i * 3], points[9 + i * 3], r, color, -1);
            }
            for (int i = 9; i < 13; ++i)
            {
                float radius = (points[10 + i * 3] - min_z) * range;
                auto color = image::Color::from_rgb(0, radius * 255, 0);
                int r = (int)(radius * r_max);
                if (r < r_min)
                    r = r_min;
                img.draw_circle(points[8 + i * 3], points[9 + i * 3], r, color, -1);
            }
            for (int i = 13; i < 17; ++i)
            {
                float radius = (points[10 + i * 3] - min_z) * range;
                auto color = image::Color::from_rgb(0, 0, radius * 255);
                int r = (int)(radius * r_max);
                if (r < r_min)
                    r = r_min;
                img.draw_circle(points[8 + i * 3], points[9 + i * 3], r, color, -1);
            }
            for (int i = 17; i < 21; ++i)
            {
                float radius = (points[10 + i * 3] - min_z) * range;
                uint8_t c = (uint8_t)(radius * 255);
                auto color = image::Color::from_rgb(0, c, c);
                int r = (int)(radius * r_max);
                if (r < r_min)
                    r = r_min;
                img.draw_circle(points[8 + i * 3], points[9 + i * 3], r, color, -1);
            }
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.HandLandmarks.labels
         */
        std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.HandLandmarks.label_path
         */
        std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.HandLandmarks.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.HandLandmarks.scale
         */
        std::vector<float> scale;

    protected:
        std::string type_str = "hand_landmarks";

    private:
        image::Size _input_size;
        image::Size _input_size_detect;
        image::Format _input_img_fmt;
        nn::NN *_model;
        nn::NN *_model_detect;
        std::map<string, string> _extra_info;
        float _conf_th = 0.7;
        float _iou_th = 0.45;
        float _conf_th2 = 0.8;
        std::vector<std::vector<float>> _anchors;

    private:
        bool _parse_anchor_line(std::string &line, std::vector<std::vector<float>> &anchors)
        {
            std::vector<std::string> tokens;
            split0(tokens, line, ",");
            if(tokens.size() != 4)
                return false;
            std::vector<float> anchor = {std::stof(tokens[0]) * _input_size_detect.width(), std::stof(tokens[1]) * _input_size_detect.height(), std::stof(tokens[2]), std::stof(tokens[3])};
            anchors.push_back(anchor);
            return true;
        }

        void _decode_objs(nn::Objects &objects, tensor::Tensors *outputs, const float &conf_th, int input_w, int input_h, bool resized, int img_w, int img_h)
        {
            tensor::Tensor *score_out = NULL; // shape 1, 2016, 18, 1
            tensor::Tensor *box_out = NULL;   // shape 1,  2016, 1, 1
            int bbox_size = 2016;
            for (auto i : *outputs)
            {
                if (i.second->shape()[2] == 18)
                {
                    box_out = i.second;
                    bbox_size = i.second->shape()[1];
                }
                else
                {
                    score_out = i.second;
                }
            }
            if (!box_out)
            {
                throw err::Exception(err::ERR_ARGS, "wrong model");
            }
            float *scores = (float*)score_out->data();
            float *bboxes = (float*)box_out->data();
            for (int i = 0; i < bbox_size; ++i)
            {
                scores[i] = _sigmoid(scores[i]);
                if(scores[i] >= conf_th)
                {
                    float *p = bboxes + i*18;
                    int w = p[2] * _anchors[i][2];
                    int h = p[3] * _anchors[i][3];
                    int x = p[0] + _anchors[i][0] - w * 0.5;
                    int y = p[1] + _anchors[i][1] - h * 0.5;
                    int p0_x = _anchors[i][0] + p[4];
                    int p0_y = _anchors[i][1] + p[5];
                    int p2_x = _anchors[i][0] + p[8];
                    int p2_y = _anchors[i][1] + p[9];
                    float angle = atan2(p2_y - p0_y, p2_x - p0_x) + M_PI;
                    if(resized)
                    {
                        auto bbox = image::resize_map_pos_reverse(img_w, img_h, input_w, input_h, image::FIT_CONTAIN, x, y, w, h);
                        auto p2 = image::resize_map_pos_reverse(img_w, img_h, input_w, input_h, image::FIT_CONTAIN, p2_x, p2_y);
                        objects.add(bbox[0], bbox[1], bbox[2], bbox[3], 0, scores[i], std::vector<int>({p2[0], p2[1]}), angle);
                    }
                    else
                        objects.add(x, y, w, h, 0, scores[i], std::vector<int>({p2_x, p2_y}), angle);
                }
            }
        }

        std::vector<image::Image *> _crop_reize_hand(nn::Objects &objs, image::Image &img, int input_w, int input_h, std::vector<cv::Mat> &M_inverse, bool landmarks_rel)
        {
            std::vector<image::Image *> res;
            float dscale = 2.6;
            for (size_t i = 0; i < objs.size(); ++i)
            {
                nn::Object &obj = objs.at(i);
                float dy = -0.5 * sin(obj.angle);
                int cx = (int)((obj.x + obj.w * 0.5 + obj.points[0]) * 0.5);
                int cy = (int)((obj.y + obj.h * 0.5 + obj.w * dy + obj.points[1]) * 0.5);
                // img.draw_circle(cx, cy, 5, image::COLOR_RED, -1);
                // img.draw_circle(obj.points[0], obj.points[1], 5, image::COLOR_YELLOW, -1);
                obj.points.resize(71 + (landmarks_rel ? 21 * 2 : 0), 0);
                int hand_size = (int)(obj.w * dscale);
                float theta = obj.angle - M_PI * 0.5;
                cv::Mat A = (cv::Mat_<float>(4, 2) << -1, -1, -1, 1, 1, 1, 1, -1);
                cv::Mat R = (cv::Mat_<float>(2, 2) << cos(theta), sin(theta), -sin(theta), cos(theta));
                cv::Mat C;
                float half_w = hand_size * 0.5;
                cv::gemm(A, R, half_w, cv::Mat(), 0.0, C);
                for (int i = 0; i < C.rows; ++i) {
                    C.at<float>(i, 0) += cx;
                    C.at<float>(i, 1) += cy;
                    obj.points[i*2] = C.at<float>(i, 0);
                    obj.points[1 + i*2] = C.at<float>(i, 1);
                }
                obj.x = cx - half_w;
                obj.y = cy - half_w;
                obj.w = hand_size;
                obj.h = hand_size;
                cv::Mat element = C(cv::Range(0, 3), cv::Range(0, 2));
                cv::Mat dst = (cv::Mat_<float>(3, 2) << 0, 0, 0, input_h, input_w, input_h);
                auto M = cv::getAffineTransform(element, dst);
                image::Image *img_dst = new image::Image(input_w, input_h, image::FMT_RGB888);
                if(!img_dst)
                {
                    log::error("no memory");
                    break;
                }
                cv::Mat img_dst2(input_h, input_w, CV_8UC3, img_dst->data());
                cv::Mat img_src;
                image::image2cv(img, img_src, false, false);
                cv::warpAffine(img_src, img_dst2, M, cv::Size(input_w, input_h), cv::INTER_NEAREST);
                assert(img_dst2.data() == img_dst->data());
                res.push_back(img_dst);
                cv::Mat iM;
                cv::invertAffineTransform(M, iM);
                M_inverse.push_back(iM);
            }
            return res;
        }

        bool _decode_landmarks(nn::Objects &objs, int idx, tensor::Tensors *outputs, float conf_th2, std::vector<cv::Mat> &M_inverse, int input_w, int input_h, int img_w, int img_h, bool landmarks_rel)
        {
            tensor::Tensor *leftright_out = NULL;   // shape 1,  1, 1, 1
            tensor::Tensor *score_out = NULL; // shape 1, 1, 1, 1
            tensor::Tensor *points_out = NULL;   // shape 1,  63, 1, 1
            for (auto i : *outputs)
            {
                if (i.second->shape()[1] == 63)
                {
                    points_out = i.second;
                }
                else if(i.first.find("score") != std::string::npos)
                {
                    score_out = i.second;
                }
                else
                {
                    leftright_out = i.second;
                }
            }
            if (!leftright_out || !score_out || !points_out)
            {
                throw err::Exception(err::ERR_ARGS, "wrong model");
            }
            auto &obj = objs.at(idx);
            float score = _sigmoid(*(float*)score_out->data());
            float leftright = _sigmoid(*(float*)leftright_out->data());
            float *points = (float*)points_out->data();
            if(score < conf_th2)
            {
                obj.score = 0;
                return true; // invalid
            }
            obj.score = score;
            obj.class_id = leftright > 0.5 ? 1 : 0;
            cv::Mat iM;
            cv::transpose(M_inverse[idx], iM);
            cv::Mat A = cv::Mat_<double>(21, 3);
            for (int i = 0; i < A.rows; ++i) {
                A.at<double>(i, 0) = points[i*3];
                A.at<double>(i, 1) = points[i*3 + 1];
                A.at<double>(i, 2) = 1;
                obj.points[8 + i*3 + 2] = (int)(-points[i*3 + 2] * obj.w);
            }
            cv::Mat C;
            cv::gemm(A, iM, 1.0, cv::Mat(), 0.0, C);
            for (int i = 0; i < C.rows; ++i) {
                obj.points[8 + 3*i] = C.at<double>(i, 0);
                obj.points[9 + 3*i] = C.at<double>(i, 1);
            }
            if(landmarks_rel)
            {
                for (int i = 0; i < 21; ++i)
                {
                    obj.points[71 + i * 2] = points[i*3];
                    obj.points[72 + i * 2] = points[i*3 + 1];
                }
            }
            // update angle
            obj.angle = atan2(obj.points[36] - obj.points[9], obj.points[35] - obj.points[8]) + M_PI;
            return false; // valid
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
                    result->add(a->x, a->y, a->w, a->h, a->class_id, a->score, a->points, a->angle);
                }
            }
            return result;
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
    };

} // namespace maix::nn
