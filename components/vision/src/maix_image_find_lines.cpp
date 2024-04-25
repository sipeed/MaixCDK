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
    std::vector<image::Line> Image::find_lines(std::vector<int> roi, int x_stride, int y_stride, double threshold, double theta_margin, double rho_margin)
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

        list_t out;
        std::vector<image::Line> lines;
        imlib_find_lines(&out, &src_img, &roi_rect, x_stride, y_stride, threshold, theta_margin, rho_margin);
        for (size_t i = 0; list_size(&out); i ++) {
            find_lines_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            image::Line line(lnk_data.line.x1,
                             lnk_data.line.y1,
                             lnk_data.line.x2,
                             lnk_data.line.y2,
                             lnk_data.magnitude,
                             lnk_data.theta,
                             lnk_data.rho);
            lines.push_back(line);
        }

        return lines;
    }

    std::vector<image::Line> Image::find_line_segments(std::vector<int> roi, int merge_distance, int max_theta_difference)
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
        std::vector<image::Line> lines;
        imlib_lsd_find_line_segments(&out, &src_img, &roi_rect, merge_distance, max_theta_difference);
        for (size_t i = 0; list_size(&out); i ++) {
            find_lines_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            image::Line line(lnk_data.line.x1,
                             lnk_data.line.y1,
                             lnk_data.line.x2,
                             lnk_data.line.y2,
                             lnk_data.magnitude,
                             lnk_data.theta,
                             lnk_data.rho);
            lines.push_back(line);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete gray_img;
        }

        return lines;
    }
} // namespace maix::image
