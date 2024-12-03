#include "region.hpp"

Region::Region(int x, int y, int width, int height, image::Format format, camera::Camera *camera)
{
    if (format != image::Format::FMT_BGRA8888) {
        err::check_raise(err::ERR_RUNTIME, "region support FMT_BGRA8888 only!");
    }

    if (camera == NULL) {
        err::check_raise(err::ERR_RUNTIME, "region bind a NULL camera!");
    }

    // int rgn_id = mmf_get_region_unused_channel();
    int rgn_id = 1;
    if (rgn_id < 0) {
        err::check_raise(err::ERR_RUNTIME, "no more region id!");
    }

    int flip = true;
    int mirror = true;
    auto configs = sys::device_configs(true);
    for (auto &item : configs) {
        log::info("device:%s value:%s", item.first.c_str(), item.second.c_str());
    }
    auto mirror_string = configs.find("cam_flip");
    auto flip_string = configs.find("cam_mirror");
    if (mirror_string != configs.end()) {
        mirror = !atoi(mirror_string->second.c_str());
    }

    if (flip_string != configs.end()) {
        flip = !atoi(flip_string->second.c_str());
    }

    int x2 = flip ? camera->width() - width - x : x;
    int y2 = mirror ? camera->height() - height - y : y;

    int vi_vpss = 0;
    int vi_vpss_chn = camera->get_channel();
    if (0 != mmf_add_region_channel_v2(rgn_id, 0, 6, vi_vpss, vi_vpss_chn, x2, y2, width, height, mmf_invert_format_to_mmf(format))) {
        err::check_raise(err::ERR_RUNTIME, "mmf_add_region_channel_v2 failed!");
    }
    this->_id = rgn_id;
    this->_width = width;
    this->_height = height;
    this->_x = x;
    this->_y = y;
    this->_format = format;
    this->_camera = camera;
    this->_flip = flip;
    this->_mirror = mirror;
}
Region::~Region()
{
    if (mmf_del_region_channel(this->_id) < 0) {
        err::check_raise(err::ERR_RUNTIME, "mmf_del_region_unused_channel failed!");
    }
}

image::Image *Region::get_canvas()
{
    void *data;
    if (0 != mmf_region_get_canvas(this->_id, &data, NULL, NULL, NULL)) {
        err::check_raise(err::ERR_RUNTIME, "mmf_region_get_canvas failed!");
    }

    image::Image *img = NULL;
    switch (this->_format) {
    case image::Format::FMT_BGRA8888:
        img = new image::Image(this->_width, this->_height, this->_format, (uint8_t *)data, this->_width * this->_height * 4, false);
        if (img == NULL) {
            mmf_del_region_channel(this->_id);
            err::check_raise(err::ERR_RUNTIME, "malloc failed!");
        }
        memset(img->data(), 0, img->data_size());
    break;
    default:err::check_raise(err::ERR_RUNTIME, "region format not support!");break;
    }

    this->_image = img;

    return img;
}

err::Err Region::update_canvas()
{
    image::Image *img = this->_image;
    if (img->format() == image::Format::FMT_BGRA8888) {
        uint32_t *data_u32 = (uint32_t *)img->data();
        int width = img->width();
        int height = img->height();

        if (this->_flip) {
            for (int h = 0; h < height; h ++) {
                for (int w = 0; w < width / 2; w ++) {
                    int left_idx = h * width + w;
                    int right_idx = h * width + (width - 1 - w);
                    uint32_t tmp = data_u32[left_idx];
                    data_u32[left_idx] = data_u32[right_idx];
                    data_u32[right_idx] = tmp;
                }
            }
        }

        if (this->_mirror) {
            for (int h = 0; h < height / 2; h ++) {
                for (int w = 0; w < width; w ++) {
                    int left_idx = h * width + w;
                    int right_idx = (height - 1 - h) * width + w;
                    uint32_t tmp = data_u32[left_idx];
                    data_u32[left_idx] = data_u32[right_idx];
                    data_u32[right_idx] = tmp;
                }
            }
        }
    } else {
        log::error("support FMT_BGRA888 only!\r\n");
        return err::ERR_RUNTIME;
    }

    if (0 != mmf_region_update_canvas(this->_id)) {
        log::error("mmf_region_update_canvas failed!\r\n");
        return err::ERR_RUNTIME;
    }
    return err::ERR_NONE;
}