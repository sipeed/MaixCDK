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

#endif