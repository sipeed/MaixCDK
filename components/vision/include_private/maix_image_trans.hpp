#pragma once

#include "maix_basic.hpp"
#include "maix_image.hpp"

namespace maix
{
    bool maixvision_mode();
    image::Format maixvision_image_fmt();

    class ImageTrans
    {
    public:
        ImageTrans(image::Format fmt = image::FMT_JPEG, int quality = 95);
        ~ImageTrans();
        err::Err send_image(image::Image &img);
        err::Err set_format(image::Format fmt, int quality = 95);
        image::Format get_format() { return _fmt; }
        void set_quality(const int quality) { _quality = quality; }

    private:
        void *_handle;
        image::Format _fmt;
        int _quality;
    }; // class ImageTrans
} // namespace maix

