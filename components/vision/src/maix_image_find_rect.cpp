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
    std::vector<image::Rect> Image::find_rects(std::vector<int> roi, int threshold)
    {
        std::vector<image::Rect> rects;
        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];
        if (roi_rect.w < 4 || roi_rect.h < 4) {
            log::warn("roi width or height is too small, must be larger than 4");
            return rects;
        }

        image_t src_img;
        Image *gray_img = NULL;
        if (_format == image::FMT_GRAYSCALE) {
            convert_to_imlib_image(this, &src_img);
        } else {
            gray_img = to_format(image::FMT_GRAYSCALE);
            convert_to_imlib_image(gray_img, &src_img);
        }

        // This code is used to fix crash bug, but this is a terrible fix
        if (roi_rect.x == 0 && roi_rect.y == 0 && roi_rect.w == src_img.w && roi_rect.h == src_img.h) {
            roi_rect.x = 1;
            roi_rect.y = 1;
            roi_rect.w = src_img.w - 2;
            roi_rect.h = src_img.h - 2;
        }

        list_t out;
        imlib_find_rects(&out, &src_img, &roi_rect, threshold);
        for (size_t i = 0; list_size(&out); i ++) {
            find_rects_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);
            std::vector<std::vector<int>> corners = {
                {(int)lnk_data.corners[0].x, (int)lnk_data.corners[0].y},
                {(int)lnk_data.corners[1].x, (int)lnk_data.corners[1].y},
                {(int)lnk_data.corners[2].x, (int)lnk_data.corners[2].y},
                {(int)lnk_data.corners[3].x, (int)lnk_data.corners[3].y},
            };
            image::Rect rect(corners,
                             lnk_data.rect.x,
                             lnk_data.rect.y,
                             lnk_data.rect.w,
                             lnk_data.rect.h,
                             lnk_data.magnitude);
            rects.push_back(rect);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete gray_img;
        }

        return rects;
    }
} // namespace maix::image
