/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_util.hpp"

namespace maix::image
{
    std::vector<image::Circle> Image::find_circles(std::vector<int> roi, int x_stride, int y_stride, int threshold, int x_margin, int y_margin, int r_margin, int r_min, int r_max, int r_step)
    {
        image_t src_img;
        convert_to_imlib_image(this, &src_img);

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        // This code is used to fix crash bug, but this is a terrible fix
        if (roi_rect.x == 0 && roi_rect.y == 0 && roi_rect.w == src_img.w && roi_rect.h == src_img.h) {
            roi_rect.x = 1;
            roi_rect.y = 1;
            roi_rect.w = src_img.w - 2;
            roi_rect.h = src_img.h - 2;
        }

        r_min = std::max(r_min, 2);

        if (r_max < 0) {
            r_max = std::min(roi_rect.w / 2, roi_rect.h / 2);
        } else {
            r_max = std::min(r_max, std::min(roi_rect.w / 2, roi_rect.h / 2));
        }

        list_t out;
        std::vector<image::Circle> circles;
        imlib_find_circles(&out, &src_img, &roi_rect, x_stride, y_stride, threshold, x_margin, y_margin, r_margin, r_min, r_max, r_step);
        for (size_t i = 0; list_size(&out); i ++) {
            find_circles_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            image::Circle circle(lnk_data.p.x,
                                lnk_data.p.y,
                                lnk_data.r,
                                lnk_data.magnitude);
            circles.push_back(circle);
        }

        return circles;
    }
} // namespace maix::image
