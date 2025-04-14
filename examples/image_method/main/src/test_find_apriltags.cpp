#include "test_image.hpp"

int test_find_apriltags(image::Image *img) {
    std::vector<int> roi = {1, 1, img->width()- 1, img->height() - 1};
    auto t = time::ticks_us();
    auto apriltags = img->find_apriltags(roi);
    auto t2 = time::ticks_us();
    log::info("find %d apriltags cost %d us(%d ms)", apriltags.size(), t2 - t, (t2 - t) / 1000);
    for (auto &a : apriltags) {
        std::vector<std::vector<int>> mini_corners = a.corners();
        for (int i = 0; i < 4; i ++) {
            img->draw_line(mini_corners[i][0], mini_corners[i][1], mini_corners[(i + 1) % 4][0], mini_corners[(i + 1) % 4][1], maix::image::Color::from_rgb(0, 255, 0), 2);
        }
    }
    return 0;
}