/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0, model used YOLOv8 and mediepipe's.
 * @update 2025.01.07: Add face landmarks support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_image_cv.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>


namespace maix::nn
{
    /**
     * FaceLandmarksObject class
     * @maixpy maix.nn.FaceLandmarksObject
     */
    class FaceLandmarksObject
    {
    public:
        /**
         * Valid or not(score > conf_th when detect).
         * @maixpy maix.nn.FaceLandmarksObject.__init__
         * @maixcdk maix.nn.FaceLandmarksObject.FaceLandmarksObject
         */
        FaceLandmarksObject()
        {
            valid = false;
        }

        /**
         * Valid or not(score > conf_th when detect).
         * @maixpy maix.nn.FaceLandmarksObject.valid
         */
        bool valid;

        /**
         * whether face in image score, value from 0 to 1.0.
         * @maixpy maix.nn.FaceLandmarksObject.score
         */
        float score;

        /**
         * landmarks points, format: x0, y0, ..., xn-1, yn-1.
         * @maixpy maix.nn.FaceLandmarksObject.points
         */
        std::vector<int> points;

        /**
         * landmarks points, format: z0, z1, ..., zn-1.
         * @maixpy maix.nn.FaceLandmarksObject.points_z
         */
        std::vector<int> points_z;
    };

    /**
     * FaceLandmarks class
     * @maixpy maix.nn.FaceLandmarks
     */
    class FaceLandmarks
    {
    public:
        /**
         * Constructor of FaceLandmarks class
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.FaceLandmarks.__init__
         * @maixcdk maix.nn.FaceLandmarks.FaceLandmarks
         */
        FaceLandmarks(const string &model = "")
        {
            _model = nullptr;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~FaceLandmarks()
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
         * @maixpy maix.nn.FaceLandmarks.load
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
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            std::vector<nn::LayerInfo> outputs = _model->outputs_info();
            _input_size = image::Size(inputs[0].shape[2], inputs[0].shape[1]);
            log::print("\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());
            for(auto &l : outputs)
            {
                if(l.name.find("points") != std::string::npos)
                {
                    landmarks_num = l.shape[1] / 3;
                    break;
                }
            }
            log::info("landmarks num: %d", landmarks_num);
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Hand detect confidence threshold, default 0.7.
         * @param landmarks_rel outputs the relative coordinates of 21 points with respect to the top-left vertex of the hand.
         *                      In obj.points, the last 21x2 values are arranged as x0y0x1y1...x20y20.
         *                      Value from 0 to obj.w.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         *         Object's points value format: box_topleft_x, box_topleft_y, box_topright_x, box_topright_y, box_bottomright_x, box_bottomright_y， box_bottomleft_x, box_bottomleft_y,
         *                          x0, y0, z1, x1, y1, z2, ..., x20, y20, z20.
         *         If landmarks_rel is True, will be box_topleft_x, box_topleft_y...,x20,y20,z20,x0_rel,y0_rel,...,x20_rel,y20_rel.
         *         Z is depth, the larger the value, the farther away from the palm, and the positive value means closer to the camera.
         * @maixpy maix.nn.FaceLandmarks.detect
         */
        nn::FaceLandmarksObject *detect(image::Image &img, float conf_th = 0.5, bool landmarks_abs = true, bool landmarks_rel = false)
        {
            maix::image::Fit fit = maix::image::FIT_CONTAIN;
            this->_conf_th = conf_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            if (img.width() != _input_size.width() || img.height() != _input_size.height())
            {
                throw err::Exception("image size not match model input size");
            }
            if(!landmarks_abs && !landmarks_rel)
            {
                throw err::Exception(err::ERR_ARGS);
            }
            nn::FaceLandmarksObject *obj = new nn::FaceLandmarksObject();
            if(!obj)
            {
                log::error("no memory");
                return nullptr;
            }
            obj->points.resize((landmarks_abs && landmarks_rel)? landmarks_num * 4 : landmarks_num * 2, 0);
            obj->points_z.resize(landmarks_num, 0);
            tensor::Tensors *outputs = _model->forward_image(img, this->mean, this->scale, fit, false, true, false);
            if (!outputs) // not ready, return empty result.
            {
                return obj;
            }
            _decode_landmarks(*obj, outputs, this->_conf_th, _input_size.width(), _input_size.height(), img.width(), img.height(), landmarks_abs, landmarks_rel);
            delete outputs;
            return obj;
        }

        /**
         * Crop image from source image by 2 points(2 eyes)
         * @param x,y,w,h face rectangle, x,y is left-top point.
         * @param img source image
         * @param points  2 points, eye_left_x, eye_left_y, eye_right_x, eye_right_y
         * @param scale crop size scale relative to rectangle's max side length(w or h), final value is `scale *max(w, h)`,default 1.2.
         * @maixpy maix.nn.FaceLandmarks.crop_image
         */
        maix::image::Image *crop_image(maix::image::Image &img, int x, int y, int w, int h, std::vector<int> points, int new_width = -1, int new_height = -1, float scale = 1.2)
        {
            if(new_width == -1)
                new_width = _input_size.width();
            if(new_height == -1)
                new_height = _input_size.height();
            int cx = x + w * 0.5;
            int cy = y + h * 0.5;
            int new_size = w > h ? w : h;
            new_size *= scale;
            float theta = atan2(points[3] - points[1], points[2] - points[0]);
            cv::Mat A = (cv::Mat_<float>(4, 2) << -1, -1, -1, 1, 1, 1, 1, -1);
            cv::Mat R = (cv::Mat_<float>(2, 2) << cos(theta), sin(theta), -sin(theta), cos(theta));
            cv::Mat C;
            float half_w = new_size * 0.5;
            cv::gemm(A, R, half_w, cv::Mat(), 0.0, C);
            for (int i = 0; i < C.rows; ++i) {
                C.at<float>(i, 0) += cx;
                C.at<float>(i, 1) += cy;
            }
            cv::Mat element = C(cv::Range(0, 3), cv::Range(0, 2));
            cv::Mat dst = (cv::Mat_<float>(3, 2) << 0, 0, 0, new_height, new_width, new_height);
            auto M = cv::getAffineTransform(element, dst);
            image::Image *img_dst = new image::Image(new_width, new_height, image::FMT_RGB888);
            if(!img_dst)
            {
                log::error("no memory");
                return nullptr;
            }
            cv::Mat img_dst2(new_height, new_width, CV_8UC3, img_dst->data());
            cv::Mat img_src;
            image::image2cv(img, img_src, false, false);
            cv::warpAffine(img_src, img_dst2, M, cv::Size(new_width, new_height), cv::INTER_NEAREST);
            assert(img_dst2.data() == img_dst->data());

            cv::invertAffineTransform(M, _M_inverse);
            return img_dst;
        }

        /**
         * Get model input size
         * @param detect detect or landmarks model, default true.
         * @return model input size
         * @maixpy maix.nn.FaceLandmarks.input_size
         */
        image::Size input_size(bool detect = true)
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @param detect detect or landmarks model, default true.
         * @return model input size of width
         * @maixpy maix.nn.FaceLandmarks.input_width
         */
        int input_width(bool detect = true)
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @param detect detect or landmarks model, default true.
         * @return model input size of height
         * @maixpy maix.nn.FaceLandmarks.input_height
         */
        int input_height(bool detect = true)
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.FaceLandmarks.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

        /**
         * Draw hand and landmarks on image
         * @param img image object, maix.image.Image type.
         * @param leftright, 0 means left, 1 means right
         * @param points points result from detect method: x0, y0, x1, y1, ..., xn-1, yn-1.
         * @param points_z points result from detect method: z0, z1, ..., zn-1.
         * @param r_min min radius of points.
         * @param r_max min radius of points.
         * @maixpy maix.nn.FaceLandmarks.draw_face
         */
        void draw_face(image::Image &img, const std::vector<int> &points, int num, const std::vector<int> &points_z=std::vector<int>(), int r_min = 2, int r_max = 4)
        {
            if (points.size() == 0 || points.size() % 2 != 0 || num * 2 > (int)points.size())
            {
                throw std::runtime_error("keypoints size must > 0 and x,y,...,x,y format, num must <= points size");
            }
            bool have_z = points_z.size() != 0;
            if(!have_z)
            {
                int r = (r_max - r_min) / 2;
                auto color = image::Color::from_rgb(255, 255, 255);
                for (int i = 0; i < num; ++i)
                {
                    img.draw_circle(points[i * 2], points[i * 2 + 1], r, color, -1);
                }
            }
            else
            {
                if(num > (int)points_z.size())
                {
                    throw std::runtime_error("num must <= points_z size");
                }
                int max_z = INT_MIN, min_z = INT_MAX;
                for (size_t i = 0; i < points_z.size(); ++i)
                {
                    if (points_z[i] > max_z)
                        max_z = points_z[i];
                    else if (points_z[i] < min_z)
                        min_z = points_z[i];
                }
                float range = 1.0 / (max_z - min_z);
                for (int i = 0; i < num; ++i)
                {
                    float radius = (points_z[i] - min_z) * range;
                    uint8_t c = (uint8_t)(radius * 255);
                    auto color = image::Color::from_rgb(c, c, c);
                    int r = (int)(radius * r_max);
                    if (r < r_min)
                        r = r_min;
                    img.draw_circle(points[i * 2], points[i * 2 + 1], r, color, -1);
                }
            }
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.FaceLandmarks.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.FaceLandmarks.scale
         */
        std::vector<float> scale;

        /**
         * landmarks number.
         * @maixpy maix.nn.FaceLandmarks.landmarks_num
         */
        int landmarks_num;

    protected:
        std::string type_str = "face_landmarks";

    private:
        image::Size _input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        std::map<string, string> _extra_info;
        float _conf_th = 0.5;
        cv::Mat _M_inverse;

    private:
        void _decode_landmarks(nn::FaceLandmarksObject &obj, tensor::Tensors *outputs, float conf_th, int input_w, int input_h, int img_w, int img_h, bool landmarks_abs, bool landmarks_rel)
        {
            bool z_filled = false;
            tensor::Tensor *score_out = NULL; // shape 1, 1, 1, 1
            tensor::Tensor *points_out = NULL;   // shape 1,  1, 1, 1
            for (auto i : *outputs)
            {
                if(i.first.find("points") != std::string::npos)
                {
                    points_out = i.second;
                }
                else if(i.first.find("score") != std::string::npos)
                {
                    score_out = i.second;
                }
            }
            if (!score_out || !points_out)
            {
                throw err::Exception(err::ERR_ARGS, "wrong model");
            }
            obj.score = _sigmoid(*(float*)score_out->data());
            float *points = (float*)points_out->data();
            if(obj.score < conf_th)
            {
                return;
            }
            obj.valid = true;
            if(_M_inverse.empty() || !landmarks_abs)
            {
                landmarks_abs = false;
                landmarks_rel = true;
            }
            else
            {
                cv::Mat iM;
                cv::transpose(_M_inverse, iM);
                cv::Mat A = cv::Mat_<double>(landmarks_num, 3);
                for (int i = 0; i < A.rows; ++i) {
                    A.at<double>(i, 0) = points[i*3];
                    A.at<double>(i, 1) = points[i*3 + 1];
                    A.at<double>(i, 2) = 1;
                    obj.points_z[i] = (int)roundf(-points[i*3 + 2]);
                }
                z_filled = true;
                cv::Mat C;
                cv::gemm(A, iM, 1.0, cv::Mat(), 0.0, C);
                for (int i = 0; i < C.rows; ++i) {
                    obj.points[2*i] = C.at<double>(i, 0);
                    obj.points[2*i + 1] = C.at<double>(i, 1);
                }
                _M_inverse.release();
            }
            if(landmarks_rel)
            {
                int offset = landmarks_abs ? landmarks_num * 2 : 0;
                if(z_filled)
                {
                    for (int i = 0; i < landmarks_num; ++i)
                    {
                        obj.points[offset + i * 2] = points[i*3];
                        obj.points[offset + i * 2 + 1] = points[i*3 + 1];
                    }
                }
                else
                {
                    for (int i = 0; i < landmarks_num; ++i)
                    {
                        obj.points[offset + i * 2] = points[i*3];
                        obj.points[offset + i * 2 + 1] = points[i*3 + 1];
                        obj.points_z[i] = (int)roundf(-points[i*3 + 2]);
                    }
                }
            }
        }

        inline static float _sigmoid(float x) { return 1.0 / (1 + expf(-x)); }

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
