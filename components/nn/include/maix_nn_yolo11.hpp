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
            if(e == err::Err::ERR_NONE)
            {
                log::print("\tlabels num: %ld\n", labels.size());
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
                    log::error("type [%s] not support, suport detector and pose", _extra_info["type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                _type = YOLO11_Type::DETECT;
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print("\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());
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
            this->_conf_th = conf_th;
            this->_iou_th = iou_th;
            this->_keypoint_th = keypoint_th;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, false);
            if (!outputs) // not ready, return empty result.
            {
                return new nn::Objects();
            }
            nn::Objects *res = _post_process(outputs, img.width(), img.height(), fit, sort);
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
            float stride[3] = {8, 16, 32};
            tensor::Tensor *score_out = NULL; // shape 1, 80, 8400, 1
            tensor::Tensor *box_out = NULL;   // shape 1,  1,    4, 8400
            for (auto i : *outputs)
            {
                if (i.second->shape()[2] == 4 && !box_out)
                {
                    box_out = i.second;
                }
                else if (strstr(i.first.c_str(), "Sigmoid") != NULL && !score_out)
                {
                    score_out = i.second;
                    if((size_t)score_out->shape()[1] != labels.size())
                    {
                        log::error("MUD labels(%d) must equal model's(%d)", score_out->shape()[1], labels.size());
                        return false;
                    }
                }
                else if (strstr(i.first.c_str(), "output1") != NULL)
                {
                    *mask_out = i.second;
                }
                else
                {
                    *kp_out = i.second;
                }
            }
            if (!score_out || !box_out)
            {
                throw err::Exception(err::ERR_ARGS, "model output not valid");
            }
            int total_box_num = box_out->shape()[3];
            // int class_num = this->labels.size();
            int class_num = score_out->shape()[1];
            float *scores_ptr = (float *)score_out->data();
            float *dets_ptr = (float *)box_out->data();
            int idx_start[3] = {
                0,
                (int)(h / stride[0] * w / stride[0]),
                (int)(h / stride[0] * w / stride[0] + h / stride[1] * w / stride[1])};
            if (_type == YOLO11_Type::OBB)
            {
                float *angle_ptr = (float *)(*kp_out)->data();
                for (int i = 0; i < 3; i++)
                {
                    int nh = h / stride[i];
                    int nw = w / stride[i];
                    for (int ay = 0; ay < nh; ++ay)
                    {
                        for (int ax = 0; ax < nw; ++ax)
                        {
                            int offset = idx_start[i] + ay * nw + ax;
                            int class_id = _argmax(scores_ptr + offset, class_num, total_box_num);
                            // int max_idx = _argmax2(scores_ptr + offset, class_num * total_box_num - offset, total_box_num);
                            // int class_id = (offset + max_idx) / total_box_num;
                            float obj_score = scores_ptr[offset + class_id * total_box_num];
                            if (obj_score <= conf_thresh)
                            {
                                continue;
                            }
                            float angle = (angle_ptr[offset] - 0.25);
                            float angle_rad = angle * M_PI;
                            float cos_angle = cosf(angle_rad);
                            float sin_angle = sinf(angle_rad);
                            float lt_x = dets_ptr[offset];
                            float lt_y = dets_ptr[offset + total_box_num];
                            float rb_x = dets_ptr[offset + total_box_num * 2];
                            float rb_y = dets_ptr[offset + total_box_num * 3];
                            float xf = (rb_x - lt_x) / 2.0;
                            float yf = (rb_y - lt_y) / 2.0;
                            float bbox_w = (lt_x + rb_x) * stride[i];
                            float bbox_h = (lt_y + rb_y) * stride[i];
                            float bbox_x = ((xf * cos_angle - yf * sin_angle) + ax + 0.5) * stride[i] - bbox_w * 0.5;
                            float bbox_y = ((xf * sin_angle + yf * cos_angle) + ay + 0.5) * stride[i] - bbox_h * 0.5;
                            _KpInfoYolo11 *kp_info = new _KpInfoYolo11(offset, ax, ay, stride[i]);
                            Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score, {}, angle);
                            obj.temp = (void *)kp_info;
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    int nh = h / stride[i];
                    int nw = w / stride[i];
                    for (int ay = 0; ay < nh; ++ay)
                    {
                        for (int ax = 0; ax < nw; ++ax)
                        {
                            int offset = idx_start[i] + ay * nw + ax;
                            int class_id = _argmax(scores_ptr + offset, class_num, total_box_num);
                            // int max_idx = _argmax2(scores_ptr + offset, class_num * total_box_num - offset, total_box_num);
                            // int class_id = (offset + max_idx) / total_box_num;
                            float obj_score = scores_ptr[offset + class_id * total_box_num];
                            if (obj_score <= conf_thresh)
                            {
                                continue;
                            }
                            float bbox_x = (ax + 0.5 - dets_ptr[offset]) * stride[i];
                            float bbox_y = (ay + 0.5 - dets_ptr[offset + total_box_num]) * stride[i];
                            float bbox_w = (ax + 0.5 + dets_ptr[offset + total_box_num * 2]) * stride[i] - bbox_x;
                            float bbox_h = (ay + 0.5 + dets_ptr[offset + total_box_num * 3]) * stride[i] - bbox_y;
                            _KpInfoYolo11 *kp_info = new _KpInfoYolo11(offset, ax, ay, stride[i]);
                            Object &obj = objs.add(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score);
                            obj.temp = (void *)kp_info;
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
            int keypoint_num = kp_out->shape()[1] / 3; // 1, 51, 8400, 1
            int total_box_num = kp_out->shape()[2];    // 1, 51, 8400, 1
            for (size_t i = 0; i < objs.size(); ++i)
            {
                nn::Object &o = objs.at(i);
                _KpInfoYolo11 *kp_info = (_KpInfoYolo11 *)o.temp;
                float *p = data + kp_info->idx;
                for (int k = 0; k < keypoint_num; ++k)
                {
                    float score = _sigmoid(p[(k * 3 + 2) * total_box_num]);
                    int x = -1;
                    int y = -1;
                    if (score > _keypoint_th)
                    {
                        x = (p[(k * 3) * total_box_num] * 2.0 + kp_info->anchor_x) * kp_info->stride;
                        y = (p[(k * 3 + 1) * total_box_num] * 2.0 + kp_info->anchor_y) * kp_info->stride;
                    }
                    o.points.push_back(x);
                    o.points.push_back(y);
                }
                delete (_KpInfoYolo11 *)o.temp;
                o.temp = NULL;
            }
        }

        void _decode_seg_points(nn::Objects &objs, tensor::Tensor *kp_out, tensor::Tensor *mask_out)
        {
            float *data = (float *)kp_out->data();
            float *mask_data = (float *)mask_out->data(); // 1, 32, 160, 160
            int mask_h = mask_out->shape()[2];
            int mask_w = mask_out->shape()[3];
            int mask_squre = mask_h * mask_w;
            int mask_num = kp_out->shape()[1];      // 1, 32, 8400, 1
            int total_box_num = kp_out->shape()[2]; // 1, 32, 8400, 1
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
                    mask_weights[k] = p[k * total_box_num];
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
    };

} // namespace maix::nn
