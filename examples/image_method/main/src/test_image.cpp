#include "test_image.hpp"
#include "maix_image.hpp"
#include "maix_image_obj.hpp"
#include "maix_image_extra.hpp"

extern param_t g_param;

static void __test_to_format(image::Image *img, image::Format format) {
    auto t = time::ticks_us();
    auto new_img = img->to_format(format);
    auto t2 = time::ticks_us();
    log::info("image format %s to %s use time:%d us\n", image::fmt_names[img->format()].c_str(), image::fmt_names[new_img->format()].c_str(), t2 - t);
    err::check_bool_raise(new_img->data_size() == image::fmt_size[format] * img->width() * img->height(), "image format to_format error");
    err::check_bool_raise(new_img->width() == img->width(), "image format to_format error");
    err::check_bool_raise(new_img->height() == img->height(), "image format to_format error");
    err::check_bool_raise(new_img->format() == format, "image format to_format error");
    delete new_img;
}

int test_to_format(image::Image *img) {
    __test_to_format(img, image::Format::FMT_YVU420SP);
    __test_to_format(img, image::Format::FMT_GRAYSCALE);
    return 0;
}

int test_draw_image(image::Image *img) {
    bool need_delete_base_img = false;
    image::Image *base_img = nullptr;
    if (img->format() != image::Format::FMT_RGBA8888) {
        base_img = img->to_format(image::Format::FMT_RGBA8888);
        need_delete_base_img = true;
    } else {
        base_img = img;
    }

    auto new_img = image::Image(img->width() / 2, img->height() / 2, image::Format::FMT_RGBA8888);

    auto t = time::ticks_us();
    base_img->draw_image(0, 0, new_img);
    auto t2 = time::ticks_us();
    log::info("draw %dx%d on %dx%d format used time:%d us\n", new_img.width(), new_img.height(), base_img->width(), base_img->height(), t2 - t);

    if (need_delete_base_img) {
        delete base_img;
    }
    return 0;
}

int test_ccm(image::Image *img) {
    auto t = time::ticks_us();
    std::vector<float> martix = {   0, 1, 1,
                                    0, 1, 1,
                                    0, 1,1};
    auto new_img = img->ccm(martix);
    auto t2 = time::ticks_us();
    log::info("ccm cost time:%d us\n", t2 - t);
    delete new_img;
    return 0;
}

int test_binary(image::Image *img) {
    std::vector<std::vector<int>> thresholds = {{0, 50}};
    auto t = time::ticks_us();
    img->binary(thresholds);
    auto t2 = time::ticks_us();
    log::info("binary cost time:%d us\n", t2 - t);
    return 0;
}

int test_get_regression(image::Image *img) {
    std::vector<std::vector<int>> thresholds = {{0, 80, -120, -10, 0, 30}};
    auto t = time::ticks_us();
    auto lines = img->get_regression(thresholds);
    auto t2 = time::ticks_us();
    log::info("get regression %d, cost %ld us(%ld ms)", lines.size(), t2 - t, (t2 - t) / 1000);
    for (auto &l : lines) {
        img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::COLOR_GREEN, 2);
    }
    return 0;
}
