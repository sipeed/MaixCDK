#ifndef __TEST_IMAGE__HPP
#define __TEST_IMAGE__HPP

#include "maix_vision.hpp"

using namespace maix;

typedef struct {
    camera::Camera *cam;
} param_t;

int test_find_blobs(image::Image *img);
int test_gaussion(image::Image *img);
int test_find_qrcode(image::Image *img);
int test_qrcode_detector(image::Image *img);
int test_find_lines(image::Image *img);
int test_ed_lib(image::Image *img);
int test_tracking_line(image::Image *img);
int test_find_barcode(image::Image *img);
int test_to_format(image::Image *img);
int test_draw_image(image::Image *img);
int test_ccm(image::Image *img);
int test_find_apriltags(image::Image *img);

#endif