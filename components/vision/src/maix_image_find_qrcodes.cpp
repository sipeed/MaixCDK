/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_util.hpp"
#include "omv.hpp"

namespace maix::image
{
    std::vector<image::QRCode> Image::find_qrcodes(std::vector<int> roi)
    {
        image_t src_img;
        Image *gray_img = NULL;
        std::vector<image::QRCode> qrcodes;

        if (_format == image::FMT_GRAYSCALE) {
            convert_to_imlib_image(this, &src_img);
        } else {
            if (image::FMT_YVU420SP == _format) {
                gray_img = new image::Image(_width, _height, image::FMT_GRAYSCALE);
                memcpy(gray_img->data(), _data, _width * _height);
            } else {
                gray_img = this->to_format(image::FMT_GRAYSCALE);
            }
            convert_to_imlib_image(gray_img, &src_img);
        }

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        // This code is used to fix imlib_find_qrcodes crash bug, but this is a terrible fix
        if (roi_rect.x == 0 && roi_rect.y == 0 && roi_rect.w == src_img.w && roi_rect.h == src_img.h) {
            roi_rect.x = 1;
            roi_rect.y = 1;
            roi_rect.w = src_img.w - 2;
            roi_rect.h = src_img.h - 2;
        }

        list_t out;
        imlib_find_qrcodes(&out, &src_img, &roi_rect);
        for (size_t i = 0; list_size(&out); i ++) {
            find_qrcodes_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            std::vector<int> rect = {
                (int)lnk_data.rect.x,
                (int)lnk_data.rect.y,
                (int)lnk_data.rect.w,
                (int)lnk_data.rect.h,
            };
            std::vector<std::vector<int>> corners = {
                {(int)lnk_data.corners[0].x, (int)lnk_data.corners[0].y},
                {(int)lnk_data.corners[1].x, (int)lnk_data.corners[1].y},
                {(int)lnk_data.corners[2].x, (int)lnk_data.corners[2].y},
                {(int)lnk_data.corners[3].x, (int)lnk_data.corners[3].y},
            };

            std::string payload;
            payload.assign(lnk_data.payload, lnk_data.payload_len);
            xfree(lnk_data.payload);
            image::QRCode qrcode(rect,
                                 corners,
                                 payload,
                                 lnk_data.version,
                                 lnk_data.ecc_level,
                                 lnk_data.mask,
                                 lnk_data.data_type,
                                 lnk_data.eci);
            qrcodes.push_back(qrcode);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete gray_img;
        }

        return qrcodes;
    }
} // namespace maix::image
