#include "maix_vision.hpp"

using namespace maix;

int test_gaussion(image::Image *img) {
    img->gaussian(2);
    return 0;
}