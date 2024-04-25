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
    std::vector<image::DataMatrix> Image::find_datamatrices(std::vector<int> roi, int effort)
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

        list_t out;
        std::vector<image::DataMatrix> datamatrices;
        imlib_find_datamatrices(&out, &src_img, &roi_rect, effort);
        for (size_t i = 0; list_size(&out); i ++) {
            find_datamatrices_list_lnk_data_t lnk_data;
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
            image::DataMatrix datamatrix(rect,
                                        corners,
                                        payload,
                                        lnk_data.rotation,
                                        lnk_data.rows,
                                        lnk_data.columns,
                                        lnk_data.capacity,
                                        lnk_data.padding);
            datamatrices.push_back(datamatrix);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete gray_img;
        }

        return datamatrices;
    }
} // namespace maix::image
