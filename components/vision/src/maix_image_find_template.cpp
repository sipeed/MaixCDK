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
    std::vector<int> Image::find_template(image::Image &template_image, float threshold, std::vector<int> roi, int step, TemplateMatch search)
    {
        image_t src_img, template_img;
        Image *src_gray_img = NULL, *template_gray_img = NULL;
        if (_format == image::FMT_GRAYSCALE) {
            convert_to_imlib_image(this, &src_img);
        } else {
            src_gray_img = to_format(image::FMT_GRAYSCALE);
            convert_to_imlib_image(src_gray_img, &src_img);
        }

        if (template_image.format() == image::FMT_GRAYSCALE) {
            convert_to_imlib_image(&template_image, &template_img);
        } else {
            template_gray_img = template_image.to_format(image::FMT_GRAYSCALE);
            convert_to_imlib_image(template_gray_img, &template_img);
        }

        rectangle_t roi_rect;
        std::vector<int> avail_roi = _get_available_roi(roi);
        roi_rect.x = avail_roi[0];
        roi_rect.y = avail_roi[1];
        roi_rect.w = avail_roi[2];
        roi_rect.h = avail_roi[3];

        // Make sure ROI is bigger than or equal to template size
        if (roi_rect.w < template_img.w || roi_rect.h < template_img.h) {
            throw std::runtime_error("ROI must be bigger than or equal to template size");
        }

        // Make sure ROI is smaller than or equal to image size
        if ((roi_rect.x + roi_rect.w) > src_img.w || (roi_rect.y + roi_rect.h) > src_img.h) {
            throw std::runtime_error("ROI must be smaller than or equal to image size");
        }

        rectangle_t r;
        float corr;
        if (search == SEARCH_DS) {
            corr = imlib_template_match_ds(&src_img, &template_img, &r);
        } else {
            corr = imlib_template_match_ex(&src_img, &template_img, &roi_rect, step, &r);
        }

        if (_format != image::FMT_GRAYSCALE) {
            delete src_gray_img;
        }
        if (template_image.format() != image::FMT_GRAYSCALE) {
            delete template_gray_img;
        }

        if (corr > threshold) {
            return {(int)r.x, (int)r.y, (int)r.w, (int)r.h};
        } else {
            return {};
        }
    }
} // namespace maix::image
