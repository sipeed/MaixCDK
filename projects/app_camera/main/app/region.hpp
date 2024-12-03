#ifndef __MMF_REGION_H
#define __MMF_REGION_H

#include "maix_image.hpp"
#include "maix_vision.hpp"
#include "sophgo_middleware.hpp"

using namespace maix;

class Region {
private:
    int _id;
    int _x;
    int _y;
    int _width;
    int _height;
    bool _flip;
    bool _mirror;
    image::Format _format;
    image::Image *_image;
    camera::Camera *_camera;
public:
    Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera);
    ~Region();

    image::Image *get_canvas();
    err::Err update_canvas();
};

#endif // __MMF_REGION_H