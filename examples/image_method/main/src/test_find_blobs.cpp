#include "test_image.hpp"

int test_find_blobs(image::Image *img) {
    std::vector<maix::image::Blob> blobs;
    std::vector<std::vector<int>> thresholds = {{0, 80, -120, -10, 0, 30}};
    bool invert = false;
    int x_stride = 2;
    int y_stride = 1;
    int area_threshold = 500;
    int pixels_threshold = 500;
    std::vector<int> roi = {1, 1, img->width()- 1, img->height() - 1};
    auto t = time::ticks_us();
    blobs = img->find_blobs(thresholds, invert, roi, x_stride, y_stride, area_threshold, pixels_threshold);
    auto t2 = time::ticks_us() - t;
    log::info("find blobs (%d), cost %lld us(%lld ms)", blobs.size(), t2, t2 / 1000);
    for (auto &a : blobs) {
        std::vector<std::vector<int>> mini_corners = a.mini_corners();
        for (int i = 0; i < 4; i ++) {
            img->draw_line(mini_corners[i][0], mini_corners[i][1], mini_corners[(i + 1) % 4][0], mini_corners[(i + 1) % 4][1], maix::image::Color::from_rgb(0, 255, 0), 2);
        }
    }
    return 0;
}