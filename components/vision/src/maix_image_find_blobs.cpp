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
    void _convert_to_lab_thresholds(std::vector<std::vector<int>> &in, list_t *out)
    {
        for (size_t i = 0; i < in.size(); i ++) {
            color_thresholds_list_lnk_data_t lnk_data;
            std::vector<int> &threshold = in[i];
            int threshold_len = threshold.size();
            if (threshold_len > 0) {
                lnk_data.LMin = (threshold_len > 0) ? std::max(std::min((threshold[0]),
                                                                        std::max(COLOR_L_MAX, COLOR_GRAYSCALE_MAX)),
                                                                        std::min(COLOR_L_MIN, COLOR_GRAYSCALE_MIN))
                                                    : std::min(COLOR_L_MIN, COLOR_GRAYSCALE_MIN);
                lnk_data.LMax = (threshold_len > 1) ? std::max(std::min((threshold[1]),
                                                                        std::max(COLOR_L_MAX, COLOR_GRAYSCALE_MAX)),
                                                                        std::min(COLOR_L_MIN, COLOR_GRAYSCALE_MIN))
                                                    : std::max(COLOR_L_MAX, COLOR_GRAYSCALE_MAX);
                lnk_data.AMin =
                    (threshold_len > 2) ? std::max(std::min((threshold[2]), COLOR_A_MAX), COLOR_A_MIN)
                                        : COLOR_A_MIN;
                lnk_data.AMax =
                    (threshold_len > 3) ? std::max(std::min((threshold[3]), COLOR_A_MAX), COLOR_A_MIN)
                                        : COLOR_A_MAX;
                lnk_data.BMin =
                    (threshold_len > 4) ? std::max(std::min((threshold[4]), COLOR_B_MAX), COLOR_B_MIN)
                                        : COLOR_B_MIN;
                lnk_data.BMax =
                    (threshold_len > 5) ? std::max(std::min((threshold[5]), COLOR_B_MAX), COLOR_B_MIN)
                                        : COLOR_B_MAX;

                color_thresholds_list_lnk_data_t lnk_data_tmp;
                memcpy(&lnk_data_tmp, &lnk_data, sizeof(color_thresholds_list_lnk_data_t));
                lnk_data.LMin = std::min(lnk_data_tmp.LMin, lnk_data_tmp.LMax);
                lnk_data.LMax = std::max(lnk_data_tmp.LMin, lnk_data_tmp.LMax);
                lnk_data.AMin = std::min(lnk_data_tmp.AMin, lnk_data_tmp.AMax);
                lnk_data.AMax = std::max(lnk_data_tmp.AMin, lnk_data_tmp.AMax);
                lnk_data.BMin = std::min(lnk_data_tmp.BMin, lnk_data_tmp.BMax);
                lnk_data.BMax = std::max(lnk_data_tmp.BMin, lnk_data_tmp.BMax);
                list_push_back(out, &lnk_data);
            }
        }
    }

    std::vector<image::Blob> Image::find_blobs(std::vector<std::vector<int>> thresholds, bool invert, std::vector<int> roi, int x_stride, int y_stride, int area_threshold, int pixels_threshold, bool merge, int margin, int x_hist_bins_max, int y_hist_bins_max)
    {
        err::check_bool_raise(thresholds.size() != 0, "You need to set thresholds");
        image_t src_img;
        convert_to_imlib_image(this, &src_img);

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        list_t thresholds_list;
        list_init(&thresholds_list, sizeof(color_thresholds_list_lnk_data_t));
        _convert_to_lab_thresholds(thresholds, &thresholds_list);

        list_t out;
        std::vector<image::Blob> blobs;
        imlib_find_blobs(&out, &src_img, &roi_rect, x_stride, y_stride, &thresholds_list, invert,  area_threshold, pixels_threshold, merge, margin, NULL, NULL, NULL, NULL, x_hist_bins_max, y_hist_bins_max);
        list_free(&thresholds_list);

        for (size_t i = 0; list_size(&out); i++) {
            find_blobs_list_lnk_data_t lnk_data;
            list_pop_front(&out, &lnk_data);

            std::vector<int> rect = {lnk_data.rect.x,
                                     lnk_data.rect.y,
                                     lnk_data.rect.w,
                                     lnk_data.rect.h};
            std::vector<std::vector<int>> corners = {
                {(int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 0) / 4)].x, (int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 0) / 4)].y},
                {(int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 1) / 4)].x, (int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 1) / 4)].y},
                {(int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 2) / 4)].x, (int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 2) / 4)].y},
                {(int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 3) / 4)].x, (int)lnk_data.corners[((FIND_BLOBS_CORNERS_RESOLUTION * 3) / 4)].y},
            };

            point_t min_corners_tmp[4];
            point_min_area_rectangle(lnk_data.corners, min_corners_tmp, FIND_BLOBS_CORNERS_RESOLUTION);
            std::vector<std::vector<int>> mini_corners {
                {(int)min_corners_tmp[0].x, (int)min_corners_tmp[0].y},
                {(int)min_corners_tmp[1].x, (int)min_corners_tmp[1].y},
                {(int)min_corners_tmp[2].x, (int)min_corners_tmp[2].y},
                {(int)min_corners_tmp[3].x, (int)min_corners_tmp[3].y},
            };

            std::vector<int> hist_x_bins;
            std::vector<int> hist_y_bins;
            hist_x_bins.resize(lnk_data.x_hist_bins_count);
            for (size_t i = 0; i < lnk_data.x_hist_bins_count; i ++) {
                hist_x_bins.push_back(lnk_data.x_hist_bins[i]);
            }
            hist_y_bins.resize(lnk_data.x_hist_bins_count);
            for (size_t i = 0; i < lnk_data.x_hist_bins_count; i ++) {
                hist_y_bins.push_back(lnk_data.y_hist_bins[i]);
            }

            image::Blob blob(rect,
                            corners,
                            mini_corners,
                            lnk_data.centroid_x,
                            lnk_data.centroid_y,
                            lnk_data.pixels,
                            lnk_data.rotation,
                            lnk_data.code,
                            lnk_data.count,
                            lnk_data.perimeter,
                            lnk_data.roundness,
                            hist_x_bins,
                            hist_y_bins
            );
            blobs.push_back(blob);

            if (lnk_data.x_hist_bins) {
                xfree(lnk_data.x_hist_bins);
            }
            if (lnk_data.y_hist_bins) {
                xfree(lnk_data.y_hist_bins);
            }
        }
        return blobs;
    }
} // namespace maix::image
