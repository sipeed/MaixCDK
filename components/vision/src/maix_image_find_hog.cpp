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
    image::Image* Image::find_hog(std::vector<int> roi, int size)
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

        // This code is used to fix crash bug, but this is a terrible fix
        if (roi_rect.x == 0 && roi_rect.y == 0 && roi_rect.w == src_img.w && roi_rect.h == src_img.h) {
            roi_rect.x = 1;
            roi_rect.y = 1;
            roi_rect.w = src_img.w - 2;
            roi_rect.h = src_img.h - 2;
        }

        imlib_find_hog(&src_img, &roi_rect, size);

        if (_format != image::FMT_GRAYSCALE) {
            Image *out = gray_img->to_format(image::FMT_RGB888);
            delete this;
            delete gray_img;
            return out;
        }

        return this;
    }
} // namespace maix::image
