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
    static apriltag_families_t convert_to_imlib_apriltag_families(ApriltagFamilies families) {
        switch (families) {
            case ApriltagFamilies::TAG16H5:
                return ::TAG16H5;
            case ApriltagFamilies::TAG25H7:
                return ::TAG25H7;
            case ApriltagFamilies::TAG25H9:
                return ::TAG25H9;
            case ApriltagFamilies::TAG36H10:
                return ::TAG36H10;
            case ApriltagFamilies::TAG36H11:
                return ::TAG36H11;
            case ApriltagFamilies::ARTOOLKIT:
                return ::ARTOOLKIT;
            default:
                return ::TAG16H5;
        }
    }

    std::vector<image::AprilTag> Image::find_apriltags(std::vector<int> roi, ApriltagFamilies families, float fx, float fy, int cx, int cy)
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

        // This code is used to fix imlib_find_apriltags crash bug, but this is a terrible fix
        if (roi_rect.x == 0 && roi_rect.y == 0 && roi_rect.w == src_img.w && roi_rect.h == src_img.h) {
            roi_rect.x = 1;
            roi_rect.y = 1;
            roi_rect.w = src_img.w - 2;
            roi_rect.h = src_img.h - 2;
        }

        // FIXME: the default value(fx, fy) is related to the params of lens
        if (fx == -1) {
            fx = (2.8 / 3.984) * src_img.w;
        }

        if (fy == -1) {
            fy = (2.8 / 2.952) * src_img.h;
        }

        if (cx == -1) {
            cx = src_img.w / 2;
        }

        if (cy == -1) {
            cy = src_img.h / 2;
        }

        apriltag_families_t families_enum = convert_to_imlib_apriltag_families(families);

        list_t out;
        std::vector<image::AprilTag> apriltags;
        imlib_find_apriltags(&out, &src_img, &roi_rect, families_enum, fx, fy, cx, cy);
        for (size_t i = 0; list_size(&out); i ++) {
            find_apriltags_list_lnk_data_t lnk_data;
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

            image::AprilTag apriltag(rect,
                                     corners,
                                     lnk_data.id,
                                     lnk_data.family,
                                     lnk_data.centroid_x,
                                     lnk_data.centroid_y,
                                     lnk_data.z_rotation,
                                     lnk_data.decision_margin,
                                     lnk_data.hamming,
                                     lnk_data.goodness,
                                     lnk_data.x_translation,
                                     lnk_data.y_translation,
                                     lnk_data.z_translation,
                                     lnk_data.x_rotation,
                                     lnk_data.y_rotation,
                                     lnk_data.z_rotation);
            apriltags.push_back(apriltag);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete gray_img;
        }

        return apriltags;
    }
} // namespace maix::image
