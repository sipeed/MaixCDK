/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.9.19: Add PP OCR support
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include "maix_nn_ocr_object.hpp"

namespace maix::nn
{
    /**
     * PP_OCR class
     * @maixpy maix.nn.PP_OCR
    */
    class PP_OCR
    {
    public:
        /**
         * Constructor of PP_OCR class
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.PP_OCR.__init__
         * @maixcdk maix.nn.PP_OCR.PP_OCR
        */
        PP_OCR(const string &model = "")
        {
            _model = nullptr;
            _rec_model = nullptr;
            this->det = false;
            this->rec = false;
            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~PP_OCR()
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            if (_rec_model)
            {
                delete _rec_model;
                _rec_model = nullptr;
            }
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.PP_OCR.load
        */
        err::Err load(const string &model)
        {
            if (_model)
            {
                delete _model;
                _model = nullptr;
            }
            if (_rec_model)
            {
                delete _rec_model;
                _rec_model = nullptr;
            }

            _model = new nn::NN(model, false);
            if (!_model)
            {
                return err::ERR_NO_MEM;
            }
            _extra_info = _model->extra_info();
            if (_extra_info.find("model_type") != _extra_info.end())
            {
                if (_extra_info["model_type"] != "pp_ocr")
                {
                    log::error("model_type not match, expect 'pp_ocr', but got '%s'", _extra_info["model_type"].c_str());
                    return err::ERR_ARGS;
                }
            }
            else
            {
                log::error("model_type key not found");
                return err::ERR_ARGS;
            }
            log::info("model info:\n\ttype: pp_ocr");
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
            if (_extra_info.find("det") != _extra_info.end())
            {
                std::string det_str = _extra_info["det"];
                if(det_str == "true")
                {
                    this->det = true;
                    this->rec = false;
                    log::print("\tdet: true\n");
                }
                else
                {
                    this->det = false;
                    this->rec = true;
                    log::print("\tdet: false\n");
                }
            }
            else
            {
                log::error("det key not found");
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
                        if(this->det)
                            this->mean.push_back(std::stof(it));
                        else
                            this->rec_mean.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("mean value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%f ", this->det ? this->mean.back() : this->rec_mean.back());
                }
                log::print("\n");
            }
            else if(!this->rec)
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
                        if(this->det)
                            this->scale.push_back(std::stof(it));
                        else
                            this->rec_scale.push_back(std::stof(it));
                    }
                    catch (std::exception &e)
                    {
                        log::error("scale value error, should float");
                        return err::ERR_ARGS;
                    }
                    log::print("%f ", this->det ? this->scale.back() : this->rec_scale.back());
                }
                log::print("\n");
            }
            else if(!this->rec)
            {
                log::error("scale key not found");
                return err::ERR_ARGS;
            }
            std::vector<nn::LayerInfo> inputs = _model->inputs_info();
            _input_size = image::Size(inputs[0].shape[3], inputs[0].shape[2]);
            log::print("\tinput size: %dx%d\n\n", _input_size.width(), _input_size.height());

            // rec model
            if (_extra_info.find("rec_model") != _extra_info.end())
            {
                if (this->rec)
                {
                    log::error("model is rec model, shouldn't have rec_model key");
                    return err::ERR_ARGS;
                }
                std::string rec_model = fs::dirname(model) + "/" + _extra_info["rec_model"];
                log::print("\trec_model: %s\n", rec_model.c_str());
                _rec_model = new nn::NN(rec_model, false);
                if (!_rec_model)
                {
                    log::error("load rec model %s failed", rec_model.c_str());
                    return err::ERR_ARGS;
                }
                log::info("rec_model info:");
                this->rec = true;
            }
            else
            {
                log::info("only det");
            }
            if(this->rec)
            {
                if (_extra_info.find("rec_mean") != _extra_info.end())
                {
                    std::string mean_str = _extra_info["rec_mean"];
                    std::vector<std::string> mean_strs = split(mean_str, ",");
                    log::print("\trec_mean:");
                    for (auto &it : mean_strs)
                    {
                        try
                        {
                            this->rec_mean.push_back(std::stof(it));
                        }
                        catch (std::exception &e)
                        {
                            log::error("rec_mean value error, should float");
                            return err::ERR_ARGS;
                        }
                        log::print("%f ", this->rec_mean.back());
                    }
                    log::print("\n");
                }
                if (_extra_info.find("rec_scale") != _extra_info.end())
                {
                    std::string scale_str = _extra_info["rec_scale"];
                    std::vector<std::string> scale_strs = split(scale_str, ",");
                    log::print("\trec_scale:");
                    for (auto &it : scale_strs)
                    {
                        try
                        {
                            this->rec_scale.push_back(std::stof(it));
                        }
                        catch (std::exception &e)
                        {
                            log::error("rec_scale value error, should float");
                            return err::ERR_ARGS;
                        }
                        log::print("%f ", this->rec_scale.back());
                    }
                    log::print("\n");
                }
                if(this->rec_scale.empty() || this->rec_mean.empty())
                {
                    log::error("mean and scale key not found");
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

                if(!this->det)
                {
                    _rec_model = _model;
                    _model = nullptr;
                }
                std::vector<nn::LayerInfo> rec_inputs = _rec_model->inputs_info();
                _rec_input_size = image::Size(rec_inputs[0].shape[3], rec_inputs[0].shape[2]);
                std::vector<nn::LayerInfo> rec_outputs = _rec_model->outputs_info();
                size_t labels_num = rec_outputs[0].shape[2] - 2;
                if(labels_num != labels.size())
                {
                    log::error("labels size not match, model need: %lld, labels: %lld", labels_num, labels.size());
                    return err::ERR_ARGS;
                }
                if(_use_space_char)
                    labels.push_back(" ");
                _prob_num = rec_outputs[0].shape[2];
                _max_ch_num = rec_outputs[0].shape[1];
                if(_max_ch_num * 8 != _rec_input_size.width())
                {
                    log::error("input width not match, model need: %lld, actual: %lld", _max_ch_num * 8, _input_size.width());
                    return err::ERR_ARGS;
                }
            }
            return err::ERR_NONE;
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param thresh Confidence threshold where pixels have charactor, default 0.3.
         * @param box_thresh Box threshold, the box prob higher than this value will be valid, default 0.6.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @param char_box Calculate every charactor's box, default false, if true then you can get charactor's box by nn.OCR_Object's char_boxes attribute.
         * @throw If image format not match model input format or no memory, will throw err::Exception.
         * @return nn.OCR_Objects type. In C++, you should delete it after use.
         * @maixpy maix.nn.PP_OCR.detect
        */
        nn::OCR_Objects *detect(image::Image &img, float thresh = 0.3, float box_thresh = 0.6, maix::image::Fit fit = maix::image::FIT_CONTAIN, bool char_box = false)
        {
            if(!this->det)
            {
                throw err::Exception(err::ERR_ARGS, "detect method not for only rec model, please use recognize method");
            }
            this->_thresh = thresh;
            this->_box_thresh = box_thresh;
            if (img.format() != _input_img_fmt)
            {
                throw err::Exception("image format not match, input_type: " + image::fmt_names[_input_img_fmt] + ", image format: " + image::fmt_names[img.format()]);
            }
            tensor::Tensors *outputs;
            outputs = _model->forward_image(img, this->mean, this->scale, fit, true);
            if (!outputs) // not ready, return empty result.
            {
                return new nn::OCR_Objects();
            }
            nn::OCR_Objects *res = _post_process(img, outputs, img.width(), img.height(), fit);
            delete outputs;
            if(res == NULL)
            {
                throw err::Exception("post process failed, please see log before");
            }
            // rec
            return res;
        }


        /**
         * Only recognize, not detect
         * @param img image to recognize chractors, can be a stanrd cropped charactors image,
         *            if crop image not standard, you can use box_points to assgin where the charactors' 4 corner is.
         * @param box_points list type, length must be 8 or 0, default empty means not transfer image to standard image.
         *                   4 points postiion, format: [x1, y1, x2, y2, x3, y3, x4, y4], point 1 at the left-top, point 2 right-top...
         * @param char_box Calculate every charactor's box, default false, if true then you can get charactor's box by nn.OCR_Object's char_boxes attribute.
         * @maixpy maix.nn.PP_OCR.recognize
        */
        nn::OCR_Object *recognize(image::Image &img, const std::vector<int> &box_points = std::vector<int>())
        {
            nn::OCR_Box box(0, 0, img.width(), 0, img.width(), img.height(), 0, img.height());
            bool crop = false;
            if(!box_points.empty())
            {
                box.x1 = box_points[0];
                box.y1 = box_points[1];
                box.x2 = box_points[2];
                box.y2 = box_points[3];
                box.x3 = box_points[4];
                box.y3 = box_points[5];
                box.x4 = box_points[6];
                box.y4 = box_points[7];
                crop = true;
            }
            std::vector<int> idx_list;
            std::vector<std::string> char_list;
            std::vector<int> char_pos;
            nn::OCR_Object *obj = new nn::OCR_Object(box, idx_list, char_list, 1.0, char_pos);
            if(!obj)
            {
                throw err::Exception(err::ERR_NO_MEM);
            }
            _recognize(img, box, obj->idx_list, char_list, obj->char_pos, crop);
            obj->update_chars(char_list);
            return obj;
        }

        /**
         * Draw segmentation on image
         * @param img image object, maix.image.Image type.
         * @param seg_mask segmentation mask image by detect method, a grayscale image
         * @param threshold only mask's value > threshold will be draw on image, value from 0 to 255.
         * @maixpy maix.nn.PP_OCR.draw_seg_mask
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
            log::info("w: %d, h: %d, mask w: %d, h: %d", img.width(), img.height(), seg_mask.width(), seg_mask.height());
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

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.PP_OCR.input_size
         */
        image::Size input_size()
        {
            return _input_size;
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.PP_OCR.input_width
         */
        int input_width()
        {
            return _input_size.width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.PP_OCR.input_height
         */
        int input_height()
        {
            return _input_size.height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.PP_OCR.input_format
         */
        image::Format input_format()
        {
            return _input_img_fmt;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.PP_OCR.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.PP_OCR.scale
         */
        std::vector<float> scale;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.PP_OCR.rec_mean
         */
        std::vector<float> rec_mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.PP_OCR.rec_scale
         */
        std::vector<float> rec_scale;

        /**
         * labels (charactors)
         * @maixpy maix.nn.PP_OCR.labels
        */
        std::vector<std::string> labels;

        /**
         * model have detect model
         * @maixpy maix.nn.PP_OCR.det
        */
        bool det;

        /**
         * model have recognize model
         * @maixpy maix.nn.PP_OCR.rec
        */
        bool rec;

    private:
        image::Size _input_size;
        image::Size _rec_input_size;
        image::Format _input_img_fmt;
        nn::NN *_model;
        nn::NN *_rec_model;
        std::map<string, string> _extra_info;
        float _thresh = 0.3;
        float _box_thresh = 0.6;
        int   _max_candidates = 1000;
        float _unclip_ratio = 1.5;
        bool _use_dilation = false;
        bool _use_space_char = true;
        std::string _score_mode = "fast";
        int _max_ch_num;
        int _prob_num;

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

        nn::OCR_Objects *_post_process(image::Image &img, tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit);

        void _recognize(image::Image &img, const nn::OCR_Box &box, std::vector<int> &idx_list, std::vector<std::string> &char_list, std::vector<int> &char_pos, bool crop);

        // void _get_layer_objs(std::vector<nn::Object> &objs, tensor::Tensor &output, int layer_i, int layer_num)
        // {
        //     int h = output.shape()[2];
        //     int w = output.shape()[3];
        //     int class_num = this->labels.size();
        //     int box_len = class_num + 5;
        //     int s = w * h;
        //     int anchor_stride = box_len * s;
        //     int s4 = 4 * s;
        //     int s3 = 3 * s;
        //     int s2 = 2 * s;
        //     float *data = (float *)output.data();
        //     int anchor_num = this->anchors.size() / 2 / layer_num;
        //     int anchor_start = anchor_num * layer_i * 2;
        //     float scale_x = _input_size.width() / w;
        //     float scale_y = _input_size.height() / h;
        //     for (int a = 0; a < anchor_num; ++a)
        //     {
        //         for (int y = 0; y < h; ++y)
        //         {
        //             for (int x = 0; x < w; ++x)
        //             {
        //                 float *p = data + a * anchor_stride + y * w + x + s4;
        //                 float obj_score = _sigmoid(*p);
        //                 if (obj_score <= _conf_th)
        //                     continue;
        //                 float *cls_scores = p + s;
        //                 int class_id = _argmax(cls_scores, class_num, s);
        //                 obj_score *= _sigmoid(cls_scores[class_id * s]);
        //                 if (obj_score <= _conf_th)
        //                     continue;
        //                 float bbox_x = (_sigmoid(*(p - s4)) * 2 + x - 0.5) * scale_x;
        //                 float bbox_y = (_sigmoid(*(p - s3)) * 2 + y - 0.5) * scale_y;
        //                 float bbox_w = pow(_sigmoid(*(p - s2)) * 2, 2) * this->anchors[anchor_start + a * 2];
        //                 float bbox_h = pow(_sigmoid(*(p - s)) * 2, 2) * this->anchors[anchor_start + a * 2 + 1];
        //                 bbox_x -= bbox_w * 0.5; // center x to left top x
        //                 bbox_y -= bbox_h * 0.5; // center y to left top y
        //                 Object obj(bbox_x, bbox_y, bbox_w, bbox_h, class_id, obj_score);
        //                 objs.push_back(obj);
        //             }
        //         }
        //     }
        // }

        // std::vector<nn::Object> *_nms(std::vector<nn::Object> &objs)
        // {
        //     std::vector<nn::Object> *result = new std::vector<nn::Object>();
        //     std::sort(objs.begin(), objs.end(), [](const nn::Object &a, const nn::Object &b) {
        //         return a.score > b.score;
        //     });
        //     for(size_t i=0; i < objs.size(); ++i)
        //     {
        //         nn::Object &a = objs.at(i);
        //         if(a.score == 0)
        //             continue;
        //         for(size_t j = i + 1; j < objs.size(); ++j)
        //         {
        //             nn::Object &b = objs.at(j);
        //             if(b.score != 0 && a.class_id == b.class_id && _calc_iou(a, b) > this->_iou_th)
        //             {
        //                 b.score = 0;
        //             }
        //         }
        //     }
        //     for(nn::Object &a :objs)
        //     {
        //         if(a.score != 0)
        //         {
        //             if (a.x < 0)
        //             {
        //                 a.w += a.x;
        //                 a.x = 0;
        //             }
        //             if (a.y < 0)
        //             {
        //                 a.h += a.y;
        //                 a.y = 0;
        //             }
        //             if (a.x + a.w > _input_size.width())
        //             {
        //                 a.w = _input_size.width() - a.x;
        //             }
        //             if (a.y + a.h > _input_size.height())
        //             {
        //                 a.h = _input_size.height() - a.y;
        //             }
        //             result->push_back(a);
        //         }
        //     }
        //     return result;
        // }

        void _correct_bbox(nn::OCR_Objects &objs, int img_w, int img_h, maix::image::Fit fit);

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
