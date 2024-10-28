/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_util.hpp"
#include "zbar.hpp"
#include "omv.hpp"

namespace maix::image
{
    static void calculate_rect(const std::vector<int>& vec, int &x, int &y, int &width, int &height) {
        int min_x = 0xffff, min_y = 0xffff;
        int max_x = 0, max_y = 0;

        for (size_t i = 0; i < vec.size(); i += 2) {
            min_x = min_x < vec[i] ? min_x : vec[i];
            max_x = max_x > vec[i] ? max_x : vec[i];
        }

        for (size_t i = 1; i < vec.size(); i += 2) {
            min_y = min_y < vec[i] ? min_y : vec[i];
            max_y = max_y > vec[i] ? max_y : vec[i];
        }

        x = min_x;
        y = min_y;
        width = max_x - min_x;
        height = max_y - min_y;
    }

    std::vector<image::QRCode> Image::find_qrcodes(std::vector<int> roi, QRCodeDecoderType decoder_type)
    {
        std::vector<image::QRCode> qrcodes;
        std::vector<int> avail_roi = _get_available_roi(roi);
        switch (decoder_type) {
        case QRCodeDecoderType::QRCODE_DECODER_TYPE_QUIRC:
        {
            image_t src_img;
            Image *gray_img = NULL;

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

            break;
        }
        case QRCodeDecoderType::QRCODE_DECODER_TYPE_ZBAR:
        {
            bool need_delete_gray_img = false;
            bool need_delete_new_img = false;
            Image *gray_img = NULL;
            if (_format == image::FMT_GRAYSCALE) {
                gray_img = this;
            } else {
                if (image::FMT_YVU420SP == _format) {
                    gray_img = new image::Image(_width, _height, image::FMT_GRAYSCALE);
                    memcpy(gray_img->data(), _data, _width * _height);
                } else {
                    gray_img = this->to_format(image::FMT_GRAYSCALE);
                }
                need_delete_gray_img = true;
            }
            image::Image *new_img = NULL;
            if (avail_roi[0] != 0 || avail_roi[1] != 0 || avail_roi[2] != gray_img->width() || avail_roi[3] != gray_img->height()) {
                new_img = gray_img->crop(avail_roi[0], avail_roi[1], avail_roi[2], avail_roi[3]);
                need_delete_new_img = true;
            } else {
                new_img = gray_img;
            }

            zbar_qrcode_result_t result;
            zbar_scan_qrcode_in_gray((uint8_t *)new_img->data(), new_img->width(), new_img->height(), &result);
            for (int i = 0; i < result.counter; i ++) {
                for (size_t j = 0; j < result.corners[i].size(); j += 2) {
                    result.corners[i][j] += avail_roi[0];
                    result.corners[i][j + 1] += avail_roi[1];
                }
                int x, y, w, h;
                calculate_rect(result.corners[i], x, y, w, h);
                std::vector<int> rect = {
                    (int)x,
                    (int)y,
                    (int)w,
                    (int)h,
                };
                std::vector<std::vector<int>> corners = {
                    {(int)result.corners[i][0], (int)result.corners[i][1]},
                    {(int)result.corners[i][6], (int)result.corners[i][7]},
                    {(int)result.corners[i][4], (int)result.corners[i][5]},
                    {(int)result.corners[i][2], (int)result.corners[i][3]},
                };
                std::string payload = result.data[i];
                image::QRCode qrcode(rect,
                                    corners,
                                    payload,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0);
                qrcodes.push_back(qrcode);
            }
            if (need_delete_gray_img) {
                delete gray_img;
            }

            if (need_delete_new_img) {
                delete new_img;
            }
            break;
        }
        }

        return qrcodes;
    }
} // namespace maix::image
