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

#endif