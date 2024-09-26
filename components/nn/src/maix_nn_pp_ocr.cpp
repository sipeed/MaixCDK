#include "maix_nn_pp_ocr.hpp"
#include "pp_ocr_postprocess_op.h"

namespace maix::nn
{

    static int _min4(int &x0, int &x1, int &x2, int &x3)
    {
        // Find the minimum value among the four integers
        int minVal = x0;  // Assume x0 is the minimum initially
        if (x1 < minVal) minVal = x1; // Compare x1 with minVal
        if (x2 < minVal) minVal = x2; // Compare x2 with minVal
        if (x3 < minVal) minVal = x3; // Compare x3 with minVal
        return minVal;
    }

    static int _max4(int &x0, int &x1, int &x2, int &x3)
    {
        // Find the minimum value among the four integers
        int max_val = x0;  // Assume x0 is the minimum initially
        if (x1 > max_val) max_val = x1; // Compare x1 with max_val
        if (x2 > max_val) max_val = x2; // Compare x2 with max_val
        if (x3 > max_val) max_val = x3; // Compare x3 with max_val
        return max_val;
    }

    static inline void _get_external_box(std::vector<std::vector<int>> &box, int &x, int &y, int &w, int &h, int &width, int &height)
    {
        x = std::max(_min4(box[0][0], box[1][0], box[2][0], box[3][0]), 0);
        y = std::max(_min4(box[0][1], box[1][1], box[2][1], box[3][1]), 0);
        w = std::min(_max4(box[0][0], box[1][0], box[2][0], box[3][0]), width);
        h = std::min(_max4(box[0][1], box[1][1], box[2][1], box[3][1]), height);
        w = w - x;
        h = h - y;
    }

    nn::OCR_Objects *PP_OCR::_post_process(image::Image &img, tensor::Tensors *outputs, int img_w, int img_h, maix::image::Fit fit)
    {
        nn::OCR_Objects *objects = new nn::OCR_Objects();
        // int layer_num = outputs->size();
        // int i = 0;
        for (auto it = outputs->begin(); it != outputs->end(); it++)
        {
        //     // log::info("output: %s, tensor: %s", it->first.c_str(), it->second->to_str().c_str());
        //     _get_layer_objs(*objects, *it->second, i++, layer_num);
            tensor::Tensor *out = it->second;
            std::vector<int> shape = out->shape(); // 1, 1, h, w
            image::Image *tmp_img = new image::Image(shape[3], shape[2], image::Format::FMT_GRAYSCALE);
            float *data = (float*)out->data();
            float *p_data = data;
            uint8_t *img_data = (uint8_t*)tmp_img->data();
            uint8_t *p_img_data = img_data;
            cv::Mat bit_map(shape[2], shape[3], CV_8UC1);
            uint8_t *p_binary_data = (uint8_t*)bit_map.data;
            uint8_t _thresh_uint8 = (uint8_t)(_thresh * 255);
            for(int i=0; i < shape[3] * shape[2]; ++i)
            {
                *p_img_data = (uint8_t)(*p_data * 255);
                if(*p_img_data > _thresh_uint8)
                    *p_binary_data = 1;
                else
                    *p_binary_data = 0;
                ++p_data;
                ++p_img_data;
                ++p_binary_data;
            }

            // post process
            PaddleOCR::DBPostProcessor post_processor;
            cv::Mat cbuf_map(shape[2], shape[3], CV_8UC1, img_data);
            cv::Mat pred_map(shape[2], shape[3], CV_32F, data);
            if (_use_dilation) {
                cv::Mat dila_ele = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
                cv::dilate(bit_map, bit_map, dila_ele);
            }
            std::vector<float> scores;
            std::vector<std::vector<std::vector<int>>> boxes = post_processor.BoxesFromBitmap(pred_map, bit_map, _box_thresh, _unclip_ratio, _score_mode, scores);
            for(size_t i = 0; i < boxes.size(); ++i)
            {
                nn::OCR_Box box(boxes[i][0][0], boxes[i][0][1], boxes[i][1][0], boxes[i][1][1], boxes[i][2][0], boxes[i][2][1], boxes[i][3][0], boxes[i][3][1]);
                std::vector<int> idxes;
                std::vector<std::string> chars;
                std::vector<int> char_pos;
                objects->add(box, idxes, chars, scores[i], char_pos);
            }
            delete tmp_img;

            // correct boxes
            if(objects->size() > 0)
                _correct_bbox(*objects, img_w, img_h, fit);
            // recognize charactors
            for(size_t i = 0; i < boxes.size(); ++i)
            {
                // int x, y, w, h;
                // _get_external_box(boxes[i], x, y, w, h, shape[3], shape[2]);
                std::vector<std::string> char_list;
                nn::OCR_Object &obj = objects->at(i);
                _recognize(img, obj.box, obj.idx_list, char_list, obj.char_pos, true);
                obj.update_chars(char_list);
            }
            break;
        }

        return objects;
    }

    static void _recognize_stdimg(nn::NN *_rec_model, image::Image &std_img, std::vector<float> &mean, std::vector<float> &scale, std::vector<int> &max_idxes, const int &_max_ch_num, const int &_prob_num)
    {
        // foward image
        tensor::Tensors *outputs;
        outputs = _rec_model->forward_image(std_img, mean, scale, image::Fit::FIT_FILL, true);
        if (!outputs) // not happen here
        {
            throw err::Exception(err::ERR_RUNTIME);
        }

        // rec postprocess, outputs shape: _max_ch_num x (_prob_num)
        // get all max prob, totally _max_ch_num sections,
        // then remove dumplicate and empty section, get charactors store in chars var.
        for (auto it = outputs->begin(); it != outputs->end(); it++)
        {
            tensor::Tensor *out = it->second;
            float *data = (float*)out->data();
            float max_score;
            for(int i = 0; i < _max_ch_num; ++i)
            {
                max_score = 0;
                for(int j = 0; j < _prob_num; ++j)
                {
                    if(*data > max_score)
                    {
                        max_score = *data;
                        max_idxes[i] = j;
                    }
                    ++data;
                }
            }
            break;
        }
        delete outputs;
    }

    void PP_OCR::_recognize(image::Image &img, const nn::OCR_Box &box, std::vector<int> &idx_list, std::vector<std::string> &char_list, std::vector<int> &char_pos, bool crop)
    {
        idx_list.clear();
        char_list.clear();
        cv::Mat img_src(img.height(), img.width(), CV_8UC3, img.data());
        cv::Mat *std_img = &img_src;
        cv::Mat img_dst;
        cv::Mat srcCopy;
        if(crop)
        {
            // crop and get std
            cv::Point2f pts_std[4];
            int img_crop_width = int(sqrt(pow(box.x1 - box.x2, 2) +
                                    pow(box.y1 - box.y2, 2)));
            int img_crop_height = int(sqrt(pow(box.x1 - box.x4, 2) +
                                            pow(box.y1 - box.y4, 2)));
            pts_std[0] = cv::Point2f(0., 0.);
            pts_std[1] = cv::Point2f(img_crop_width, 0.);
            pts_std[2] = cv::Point2f(img_crop_width, img_crop_height);
            pts_std[3] = cv::Point2f(0.f, img_crop_height);
            cv::Point2f pointsf[4];
            pointsf[0] = cv::Point2f(box.x1, box.y1);
            pointsf[1] = cv::Point2f(box.x2, box.y2);
            pointsf[2] = cv::Point2f(box.x3, box.y3);
            pointsf[3] = cv::Point2f(box.x4, box.y4);
            cv::Mat M = cv::getPerspectiveTransform(pointsf, pts_std);
            cv::warpPerspective(img_src, img_dst, M,
                        cv::Size(img_crop_width, img_crop_height),
                        cv::BORDER_REPLICATE);
            std_img = &img_dst;
            if (float(img_dst.rows) >= float(img_dst.cols) * 1.5) {
                srcCopy = cv::Mat(img_dst.rows, img_dst.cols, img_dst.depth());
                cv::transpose(img_dst, srcCopy);
                cv::flip(srcCopy, srcCopy, 0);
                std_img = &srcCopy;
            }
        }

        // resize pad image to model input size(_rec_input_size) like 320x48
        // keep ratio resize image's height to _rec_input_size.height(),
        // if new image width > _rec_input_size.width()， resize new image width to _rec_input_size.width(), padding black color at bottom,
        // else if new image width < _rec_input_size.width(), keep new image content at left, right padding black color.
        // finally we got _rec_input_size image
        float aspect_ratio = float(std_img->cols) / float(std_img->rows);
        cv::Mat resized_img;
        // Height is the limiting factor
        cv::resize(*std_img, resized_img, cv::Size(static_cast<int>(_rec_input_size.height() * aspect_ratio),
                    _rec_input_size.height()));
        if (aspect_ratio > float(_rec_input_size.width()) / float(_rec_input_size.height())) {
            // Width > input width
            // slice and recognize
            for(int i = 0; i < std::ceil((float)resized_img.cols / _rec_input_size.width()); ++i)
            {
                // crop
                cv::Mat crop_img = resized_img(cv::Rect(i * _rec_input_size.width(), 0, std::min(_rec_input_size.width(), resized_img.cols - i * _rec_input_size.width()), resized_img.rows));
                cv::Mat final_img;
                if(crop_img.cols < _rec_input_size.width())
                {
                    cv::Mat padded_img(_rec_input_size.height(), _rec_input_size.width(), CV_8UC3, cv::Scalar(0, 0, 0));
                    crop_img.copyTo(padded_img(cv::Rect(0, 0, crop_img.cols, crop_img.rows)));
                    padded_img.copyTo(final_img);
                }
                else
                {
                    crop_img.copyTo(final_img);
                }
                // show image on left-top
                image::Image std_img(final_img.cols, final_img.rows, image::Format::FMT_BGR888, final_img.data, -1, false);
                // if(i == 0)
                //     img.draw_image(0, 0, std_img);

                std::vector<int> max_idxes(_max_ch_num);
                _recognize_stdimg(_rec_model, std_img, this->rec_mean, this->rec_scale, max_idxes, _max_ch_num, _prob_num);

                int last_idx = 0;
                for(int j = 0; j < _max_ch_num; ++j)
                {
                    if((max_idxes[j] != last_idx) && (max_idxes[j] != 0))
                    {
                        idx_list.push_back(max_idxes[j] - 1);
                        char_list.push_back(labels[max_idxes[j] - 1]);
                        char_pos.push_back(j + i * _max_ch_num);
                    }
                    last_idx = max_idxes[j];
                }
            }
        } else {
            // pad
            cv::Mat padded_img(_rec_input_size.height(), _rec_input_size.width(), CV_8UC3, cv::Scalar(0, 0, 0));
            resized_img.copyTo(padded_img(cv::Rect(0, 0, resized_img.cols, resized_img.rows)));
            image::Image pad_img(padded_img.cols, padded_img.rows, image::Format::FMT_BGR888, padded_img.data, -1, false);

            // show image on left-top
            // img.draw_image(0, 0, pad_img);

            std::vector<int> max_idxes(_max_ch_num);
            _recognize_stdimg(_rec_model, pad_img, this->rec_mean, this->rec_scale, max_idxes, _max_ch_num, _prob_num);

            int last_idx = 0;
            for(int i = 0; i < _max_ch_num; ++i)
            {
                if((max_idxes[i] != last_idx) && (max_idxes[i] != 0))
                {
                    idx_list.push_back(max_idxes[i] - 1);
                    char_list.push_back(labels[max_idxes[i] - 1]);
                    char_pos.push_back(i);
                }
                last_idx = max_idxes[i];
            }
        }
    }

    void PP_OCR::_correct_bbox(nn::OCR_Objects &objs, int img_w, int img_h, maix::image::Fit fit)
    {
#define CORRECT_BBOX_RANGE_PP_OCR(obj)      \
do                               \
{                                \
    if (obj->box.x1 < 0)              \
        obj->box.x1 = 0;              \
    if (obj->box.x4< 0)               \
        obj->box.x4 = 0;              \
    if (obj->box.y1 < 0)              \
        obj->box.y1 = 0;              \
    if (obj->box.y2 < 0)              \
        obj->box.y2 = 0;              \
    if (obj->box.x2 > img_w)              \
        obj->box.x2 = img_w;              \
    if (obj->box.x3 > img_w)               \
        obj->box.x3  = img_w;              \
    if (obj->box.y3 > img_h)              \
        obj->box.y3 = img_h;              \
    if (obj->box.y4 > img_h)               \
        obj->box.y4  = img_h;              \
} while (0)

        if(img_w == _input_size.width() && img_h == _input_size.height())
            return;
        if (fit == maix::image::FIT_FILL)
        {
            float scale_x = (float)img_w / _input_size.width();
            float scale_y = (float)img_h / _input_size.height();
            for (nn::OCR_Object *obj : objs)
            {
                obj->box.x1 *= scale_x;
                obj->box.y1 *= scale_y;
                obj->box.x2 *= scale_x;
                obj->box.y2 *= scale_y;
                obj->box.x3 *= scale_x;
                obj->box.y3 *= scale_y;
                obj->box.x4 *= scale_x;
                obj->box.y4 *= scale_y;
                CORRECT_BBOX_RANGE_PP_OCR(obj);
            }
        }
        else if(fit == maix::image::FIT_CONTAIN)
        {
            float scale_x = ((float)_input_size.width()) / img_w ;
            float scale_y = ((float)_input_size.height()) / img_h ;
            float scale = std::min(scale_x, scale_y);
            float scale_reverse = 1.0 / scale;
            float pad_w = (_input_size.width() - img_w * scale) / 2.0;
            float pad_h = (_input_size.height() - img_h * scale) / 2.0;
            for (nn::OCR_Object *obj : objs)
            {
                obj->box.x1 = (obj->box.x1 - pad_w) * scale_reverse;
                obj->box.y1 = (obj->box.y1 - pad_h) * scale_reverse;
                obj->box.x2 = (obj->box.x2 - pad_w) * scale_reverse;
                obj->box.y2 = (obj->box.y2 - pad_h) * scale_reverse;
                obj->box.x3 = (obj->box.x3 - pad_w) * scale_reverse;
                obj->box.y3 = (obj->box.y3 - pad_h) * scale_reverse;
                obj->box.x4 = (obj->box.x4 - pad_w) * scale_reverse;
                obj->box.y4 = (obj->box.y4 - pad_h) * scale_reverse;
                CORRECT_BBOX_RANGE_PP_OCR(obj);
            }
        }
        else if(fit == maix::image::FIT_COVER)
        {
            float scale_x = ((float)_input_size.width()) / img_w ;
            float scale_y = ((float)_input_size.height()) / img_h ;
            float scale = std::max(scale_x, scale_y);
            float scale_reverse = 1.0 / scale;
            float pad_w = (img_w * scale - _input_size.width()) / 2.0;
            float pad_h = (img_h * scale - _input_size.height()) / 2.0;
            for (nn::OCR_Object *obj : objs)
            {
                obj->box.x1 = (obj->box.x1 + pad_w) * scale_reverse;
                obj->box.y1 = (obj->box.y1 + pad_h) * scale_reverse;
                obj->box.x2 = (obj->box.x2 + pad_w) * scale_reverse;
                obj->box.y2 = (obj->box.y2 + pad_h) * scale_reverse;
                obj->box.x3 = (obj->box.x3 + pad_w) * scale_reverse;
                obj->box.y3 = (obj->box.y3 + pad_h) * scale_reverse;
                obj->box.x4 = (obj->box.x4 + pad_w) * scale_reverse;
                obj->box.y4 = (obj->box.y4 + pad_h) * scale_reverse;
                CORRECT_BBOX_RANGE_PP_OCR(obj);
            }
        }
        else
        {
            throw err::Exception(err::ERR_ARGS, "fit type not support");
        }
    }
}

