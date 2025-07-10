#include "region.hpp"
Region::Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera)
{

}
Region::~Region()
{

}

image::Image *Region::get_canvas()
{
    image::Image *img = NULL;
    return img;
}

err::Err Region::update_canvas()
{
    return err::ERR_NONE;
}