/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_util.hpp"
#include "opencv2/opencv.hpp"

#include <omv.hpp>

namespace maix::image
{
    Displacement Image::find_displacement(image::Image &template_image, std::vector<int> roi, std::vector<int> template_roi, bool logpolar)
    {
#if 0
        int pixel_num = _get_cv_pixel_num(_format);
        cv::Mat src_img(_height, _width, pixel_num, _data);
        cv::Mat template_img(template_image.height(), template_image.width(), pixel_num, template_image.data());

        // 转换为灰度图像
        cv::Mat prevGray, gray;
        cv::cvtColor(template_img, prevGray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(src_img, gray, cv::COLOR_BGR2GRAY);

        cv::Rect roi_temp(roi[0], roi[1], roi[2], roi[3]);
        std::vector<cv::Point2f> prevPoints;
        cv::goodFeaturesToTrack(prevGray(roi_temp), prevPoints, 100, 0.3, 7);


        std::vector<cv::Point2f> nextPoints;
        std::vector<uchar> status;
        std::vector<float> err;
        cv::calcOpticalFlowPyrLK(prevGray, gray, prevPoints, nextPoints, status, err);

        int x = std::abs(nextPoints[0].x - prevPoints[0].x);
        int y = std::abs(nextPoints[0].y - prevPoints[0].y);
        float r = 0.0;
        float s = 0.0;
        float response = 0.0;
        Displacement displacement(x, y, r, s, response);
        return displacement;
#else
        image_t src_img, template_img;
        convert_to_imlib_image(this, &src_img);
        convert_to_imlib_image(&template_image, &template_img);

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        rectangle_t template_roi_rect;
        template_roi_rect.x = template_roi[0];
        template_roi_rect.y = template_roi[1];
        template_roi_rect.w = template_roi[2];
        template_roi_rect.h = template_roi[3];

        if (roi_rect.w != template_roi_rect.w || roi_rect.h != template_roi_rect.h) {
            throw std::runtime_error("roi and template_roi must have the same size");
        }

        bool fix_rotation_scale = false;
        float x, y, r, s, response;
        imlib_phasecorrelate(&src_img, &template_img, &roi_rect, &template_roi_rect, logpolar, fix_rotation_scale, &x, &y, &r, &s, &response);
        Displacement displacement(x, y, r, s, response);
        return displacement;
#endif
    }
} // namespace maix::image
