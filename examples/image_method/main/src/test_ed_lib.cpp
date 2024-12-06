#include "test_image.hpp"

#include "EDLib.h"

static int ED_Lib_test(image::Image *input)
{
    auto img = input;
    auto gray_img = (image::Image *)nullptr;
    auto need_free_gray_img = false;
    if (img->format() != image::FMT_GRAYSCALE) {
        gray_img = img->to_format(image::FMT_GRAYSCALE);
        need_free_gray_img = true;
    } else {
        gray_img = img;
        need_free_gray_img = false;
    }

    auto cv_gray = cv::Mat(gray_img->height(), gray_img->width(), CV_8UC1, gray_img->data());

    uint64_t t = time::ticks_ms();
    double line_error = 1.0;
    int min_line_len = -1;
    double max_distance = 6;
    double max_error = 1.3;
    auto ed_lines = EDLines(cv_gray, line_error, min_line_len, max_distance, max_error);
    auto ed_lines_res = ed_lines.getLines();
    log::info(" EDLines use %ld ms, size:%d", time::ticks_ms() - t, ed_lines_res.size());

    for (auto &l : ed_lines_res) {
        img->draw_line(l.start.x, l.start.y, l.end.x, l.end.y, image::COLOR_GREEN);
    }

    if (need_free_gray_img) {
        delete gray_img;
    }
    return 0;
}

int test_ed_lib(image::Image *img) {
    ED_Lib_test(img);
    return 0;
}