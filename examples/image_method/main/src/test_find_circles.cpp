#include "test_image.hpp"

int test_find_circles(image::Image *img) {
    auto t = time::ticks_us();
    auto circles = img->find_circles();
    auto t2 = time::ticks_us() - t;
    log::info("find circles (%d), cost %lld us(%lld ms)", circles.size(), t2, t2 / 1000);
    for (auto &a : circles) {
        img->draw_circle(a.x(), a.y(), a.r(), image::COLOR_GREEN);
    }
    return 0;
}