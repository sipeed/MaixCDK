#include "test_image.hpp"

int test_find_lines(image::Image *img) {
    uint64_t t = time::ticks_ms(), t2;
    auto lines = img->find_lines();
    t2 = time::ticks_ms(), log::info("find lines use %ld ms", t2 - t);
    for (auto &l : lines) {
        img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), maix::image::Color::from_rgb(0, 255, 0), 2);
    }
    return 0;
}