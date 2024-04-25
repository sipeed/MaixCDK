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
    std::vector<int> Image::find_features(int cascade, float threshold, float scale, std::vector<int> roi)
    {
        // image_t src_img;
        // convert_to_imlib_image(this, &src_img);

        // rectangle_t roi_rect;
        // roi_rect.x = roi[0];
        // roi_rect.y = roi[1];
        // roi_rect.w = roi[2];
        // roi_rect.h = roi[3];

        // // Make sure ROI is bigger than feature size
        // if (roi_rect.w < cascade->w || roi_rect.h < cascade->h) {
        //     throw std::runtime_error("ROI must be bigger than feature size");
        // }

        // array_t *objects_array = imlib_detect_objects(&src_img, cascade, &roi_rect);
        // for (size_t i = 0; i < array_length(objects_array); i ++) {
        //     rectangle_t *r = array_at(objects_array, i);
        //     std::vector<int> rect = {(int)r->x, (int)r->y, (int)r->w, (int)r->h};
        //     rects.push_back(rect);
        // }
        // list_t out;
        // std::vector<image::Line> lines;
        // imlib_find_lines(&out, &src_img, &roi_rect, x_stride, y_stride, threshold, theta_margin, rho_margin);
        // for (size_t i = 0; list_size(&out); i ++) {
        //     find_lines_list_lnk_data_t lnk_data;
        //     list_pop_front(&out, &lnk_data);

        //     image::Line line(lnk_data.line.x1,
        //                      lnk_data.line.y1,
        //                      lnk_data.line.x2,
        //                      lnk_data.line.y2,
        //                      lnk_data.magnitude,
        //                      lnk_data.theta,
        //                      lnk_data.rho);
        //     lines.push_back(line);
        // }

        // return lines;
        return std::vector<int>();
    }
} // namespace maix::image
