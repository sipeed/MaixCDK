#pragma once

#include "maix_basic.hpp"
#include "maix_image.hpp"

namespace maix
{
    bool maixvision_mode();
    image::Format maixvision_image_fmt();
    int maixvision_image_quality();

    class ImageTrans
    {
    public:
        ImageTrans(image::Format fmt = image::FMT_JPEG, int quality = 80);
        ~ImageTrans();
        err::Err send_image(image::Image &img);
        err::Err set_format(image::Format fmt);
        image::Format get_format() { return _fmt; }
        int set_quality(const int quality) {
#if PLATFORM_MAIXCAM // limit in [51, 99]
            _quality = quality > 99 ? 99 : quality;
            _quality = quality < 51 ? 51 : quality;
#else
            _quality = quality > 100 ? 100 : quality;
            _quality = quality < 0 ? 0 : quality;
#endif
            return _quality;
        }

    private:
        void *_handle;
        image::Format _fmt;
        int _quality;
    }; // class ImageTrans
} // namespace maix

