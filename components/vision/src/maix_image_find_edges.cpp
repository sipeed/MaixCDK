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
    image::Image* Image::find_edges(EdgeDetector edge_type, std::vector<int> roi, std::vector<int> threshold)
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

        switch (edge_type)
        {
            case EdgeDetector::EDGE_SIMPLE:
                imlib_edge_simple(&src_img, &roi_rect, threshold[0], threshold[1]);
                break;
            case EdgeDetector::EDGE_CANNY:
                imlib_edge_canny(&src_img, &roi_rect, threshold[0], threshold[1]);
                break;
        }

        if (_format != image::FMT_GRAYSCALE) {
            Image *out = gray_img->to_format(image::FMT_RGB888);
            memcpy(this->data(), out->data(), out->data_size());
            delete gray_img;
            delete out;
        }

        return this;
    }
} // namespace maix::image
