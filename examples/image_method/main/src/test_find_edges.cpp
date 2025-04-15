#include "test_image.hpp"

int test_find_edges(image::Image *img) {
    auto t = time::ticks_us();
    img->find_edges(image::EDGE_CANNY, {}, {50, 100});
    auto t2 = time::ticks_us() - t;
    log::info("find edges cost %lld us(%lld ms)", t2, t2 / 1000);img->save("/root/test.jpg");
    return 0;
}