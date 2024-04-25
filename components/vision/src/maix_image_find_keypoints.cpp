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
    // static corner_detector_t convert_to_imlib_corner_detector(CornerDetector &corner_detector)
    // {
    //     switch (corner_detector)
    //     {
    //     case CornerDetector::CORNER_FAST:
    //         return ::CORNER_FAST;
    //     case CornerDetector::CORNER_AGAST:
    //         return ::CORNER_AGAST;
    //     default:
    //         return ::CORNER_AGAST;
    //     }
    // }

    ORBKeyPoint Image::find_keypoints(std::vector<int> roi, int threshold, bool normalized, float scale_factor, int max_keypoints, CornerDetector corner_detector)
    {
#if 0
        image_t src_img;
        convert_to_imlib_image(this, &src_img);

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        corner_detector_t detector = convert_to_imlib_corner_detector(corner_detector);

        array_t *kpts = orb_find_keypoints(&src_img, normalized, threshold, scale_factor, max_keypoints, detector, &roi_rect);
        std::vector<KeyPoint> keypoints;

        if (array_length(kpts))
        {
            for (int i = 0; i < array_length(kpts); i++)
            {
                kp_t *kp = (kp_t *)array_at(kpts, i);

                // Makesure desc size of kp_t is 32
                std::vector<uint8_t> desc(kp->desc, kp->desc + 32);
                KeyPoint keypoint(kp->x, kp->y, kp->score, kp->octave, kp->angle, kp->matched, desc);
                keypoints.push_back(keypoint);
            }

            ORBKeyPoint orb_keypoints(keypoints, threshold, normalized);
            return orb_keypoints;
        } else {
            return ORBKeyPoint(keypoints, 0, 0);
        }
#else
        return ORBKeyPoint();
#endif
    }

    KPTMatch Image::match_orb_descriptor(ORBKeyPoint &desc1, ORBKeyPoint &desc2, int threshold, bool filter_outliers)
    {
        if (threshold < 0 || threshold > 100) {
            throw std::runtime_error("threshold must be between 0 and 100");
        }

        return KPTMatch();
    }
} // namespace maix::image
