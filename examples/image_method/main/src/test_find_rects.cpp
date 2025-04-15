#include "test_image.hpp"

int test_find_rects(image::Image *img) {
    auto t = time::ticks_us();
    auto rects = img->find_rects();
    auto t2 = time::ticks_us() - t;
    log::info("find rects (%d), cost %lld us(%lld ms)", rects.size(), t2, t2 / 1000);
    for (auto &a : rects) {
        auto corners = a.corners();
        for (int i = 0; i < 4; i ++) {
            img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::COLOR_GREEN, 2);
        }
    }
    return 0;
}