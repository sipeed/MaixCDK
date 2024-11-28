/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.11.27: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_obj.hpp"
#include "maix_nn_yolov8.hpp"
#include "maix_image_extra.hpp"

using namespace maix;

namespace maix::image
{
    typedef struct {
        std::string path = "";
        nn::YOLOv8 *detector = nullptr;
        bool dual_buffer = true;
    } param_t;

    QRCodeDetector::QRCodeDetector() {
        _param = new param_t();
        param_t *param = (param_t *)_param;
        param->path = "/root/models/yolov8_qr_det_seg_320.mud";
        param->dual_buffer = true;
        param->detector = new nn::YOLOv8("", param->dual_buffer);
        err::check_raise(param->detector->load(param->path), "load model failed");
    }

    QRCodeDetector::~QRCodeDetector() {
        if (_param) {
            param_t *param = (param_t *)_param;
            if (param->detector) {
                delete param->detector;
                param->detector = nullptr;
            }

            delete param;
            _param = nullptr;
        }
    }

    std::vector<image::QRCode> QRCodeDetector::detect(image::Image *img, std::vector<int> roi, image::QRCodeDecoderType decoder_type)
    {
        param_t *param = (param_t *)_param;
        auto detector = param->detector;
        auto out = std::vector<image::QRCode>();

        auto result = detector->detect(*img);
        if (result) {
            for (auto &r : *result) {
                float scale = 0.1;
                std::vector<int> new_roi = {r->x, r->y, r->w, r->h};
                new_roi[0] = new_roi[0] - r->w * scale;
                new_roi[0] = new_roi[0] > 0 ? new_roi[0] : 0;
                new_roi[1] = new_roi[1] - r->h * scale;
                new_roi[1] = new_roi[1] > 0 ? new_roi[1] : 0;
                new_roi[2] = r->w * (1 + scale * 2);
                new_roi[2] = new_roi[2] >= img->width() ? img->width() - r->x : new_roi[2];
                new_roi[3] = r->h * (1 + scale * 2);
                new_roi[3] = new_roi[3] >= img->height() ? img->height() - r->y : new_roi[3];

                auto qrcode_result = img->find_qrcodes(new_roi, decoder_type);
                for (auto &qrcode : qrcode_result) {
                    out.push_back(qrcode);
                }
            }
            delete result;
        }

        return out;
    }
}