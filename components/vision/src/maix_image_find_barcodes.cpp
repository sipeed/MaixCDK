/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_util.hpp"
#include <omv.hpp>

namespace maix::image
{
    std::vector<image::BarCode> Image::find_barcodes(std::vector<int> roi)
    {
        image_t src_img;
        Image *gray_img = NULL;
        if (_format == image::FMT_GRAYSCALE) {
            convert_to_imlib_image(this, &src_img);
        } else {
            gray_img = to_format(image::FMT_GRAYSCALE);
            convert_to_imlib_image(gray_img, &src_img);
        }

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        list_t out;
        std::vector<image::BarCode> barcodes;
        imlib_find_barcodes(&out, &src_img, &roi_rect);
        for (size_t i = 0; list_size(&out); i ++) {
            find_barcodes_list_lnk_data_t lnk_data;
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
            image::BarCode barcode(rect,
                                corners,
                                payload,
                                lnk_data.type,
                                IM_DEG2RAD(lnk_data.rotation),
                                lnk_data.quality);
            barcodes.push_back(barcode);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete gray_img;
        }

        return barcodes;
    }
} // namespace maix::image
