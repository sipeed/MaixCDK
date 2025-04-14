#include "test_image.hpp"

int test_find_lines(image::Image *img) {
    auto t = time::ticks_us();
    auto lines = img->find_lines();
    auto t2 = time::ticks_us();
    log::info("find lines %d, cost %ld us(%ld ms)", lines.size(), t2 - t, (t2 - t) / 1000);
    for (auto &l : lines) {
        img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), maix::image::Color::from_rgb(0, 255, 0), 2);
    }
    return 0;
}