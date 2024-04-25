/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_util.hpp"
#include <omv.hpp>
#include <valarray>

namespace maix::image
{
    #define LBP_HIST_SIZE      (59) //58 uniform hist + 1
    #define LBP_NUM_REGIONS    (7)  //7x7 regions
    #define LBP_DESC_SIZE      (LBP_NUM_REGIONS * LBP_NUM_REGIONS * LBP_HIST_SIZE)

    const static uint8_t uniform_tbl[256] = {
        0,   1,  2,  3,  4, 58,  5,  6,  7, 58, 58, 58,  8, 58,  9, 10,
        11, 58, 58, 58, 58, 58, 58, 58, 12, 58, 58, 58, 13, 58, 14, 15,
        16, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
        17, 58, 58, 58, 58, 58, 58, 58, 18, 58, 58, 58, 19, 58, 20, 21,
        22, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
        58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
        23, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
        24, 58, 58, 58, 58, 58, 58, 58, 25, 58, 58, 58, 26, 58, 27, 28,
        29, 30, 58, 31, 58, 58, 58, 32, 58, 58, 58, 58, 58, 58, 58, 33,
        58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 34,
        58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
        58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 35,
        36, 37, 58, 38, 58, 58, 58, 39, 58, 58, 58, 58, 58, 58, 58, 40,
        58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 41,
        42, 43, 58, 44, 58, 58, 58, 45, 58, 58, 58, 58, 58, 58, 58, 46,
        47, 48, 58, 49, 58, 58, 58, 50, 51, 52, 58, 53, 54, 55, 56, 57
    };

    static uint8_t *__imlib_lbp_desc(image_t *image, rectangle_t *roi, int *desc_size) {
        int s = image->w; //stride
        int RX = roi->w / LBP_NUM_REGIONS;
        int RY = roi->h / LBP_NUM_REGIONS;
        int y_idx_max = ((((roi->y + roi->h) - 3) - roi->y) / RY) * LBP_NUM_REGIONS;
        int x_max = (roi->x + roi->w) - 3;
        int desc_max_size = (y_idx_max + (x_max- roi->x) / RX) * LBP_HIST_SIZE + 256;
        uint8_t *data = image->data;
        uint8_t *desc = (uint8_t *)malloc(desc_max_size);
        for (int y = roi->y; y < (roi->y + roi->h) - 3; y++) {
            int y_idx = ((y - roi->y) / RY) * LBP_NUM_REGIONS;
            for (int x = roi->x; x < (roi->x + roi->w) - 3; x++) {
                uint8_t lbp = 0;
                uint8_t p = data[(y + 1) * s + x + 1];
                int hist_idx = y_idx + (x - roi->x) / RX;

                lbp |= (data[(y + 0) * s + x + 0] >= p) << 0;
                lbp |= (data[(y + 0) * s + x + 1] >= p) << 1;
                lbp |= (data[(y + 0) * s + x + 2] >= p) << 2;
                lbp |= (data[(y + 1) * s + x + 2] >= p) << 3;
                lbp |= (data[(y + 2) * s + x + 2] >= p) << 4;
                lbp |= (data[(y + 2) * s + x + 1] >= p) << 5;
                lbp |= (data[(y + 2) * s + x + 0] >= p) << 6;
                lbp |= (data[(y + 1) * s + x + 0] >= p) << 7;
                desc[hist_idx * LBP_HIST_SIZE + uniform_tbl[lbp]]++;
            }
        }

        *desc_size = desc_max_size;
        return desc;
    }

    LBPKeyPoint Image::find_lbp(std::vector<int> roi)
    {
        image_t src_img;
        convert_to_imlib_image(this, &src_img);

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        int desc_max_size = 0;
        uint8_t *desc = __imlib_lbp_desc(&src_img, &roi_rect, &desc_max_size);
        std::valarray<uint8_t> desc_val(desc, desc_max_size);
        LBPKeyPoint lbp_keypoint(desc_val);
        free(desc);
        return lbp_keypoint;
    }

    int Image::match_lbp_descriptor(LBPKeyPoint &desc1, LBPKeyPoint &desc2)
    {
        std::valarray<uint8_t> desc1_val = desc1.get_data();
        std::valarray<uint8_t> desc2_val = desc2.get_data();
        uint8_t *hist1 = &desc1_val[0];
        uint8_t *hist2 = &desc2_val[0];
        int res = imlib_lbp_desc_distance(hist1, hist2);
        return res;
    }
} // namespace maix::image
