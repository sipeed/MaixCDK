#ifndef __AX_MIDDLEWARE_HPP__
#define __AX_MIDDLEWARE_HPP__

#include <stdint.h>
#include "maix_basic.hpp"
using namespace maix;

namespace maix::middleware::maixcam2 {
    class Frame {
    public:
        void *data;
        int len;
        int w;
        int h;
        int fmt;
        void *frame;
        Frame(void *frame);
        ~Frame();
    };

    class VI {
    public:
        VI(char *sensor_name, bool raw);
        ~VI();
        err::Err init();
        err::Err deinit();
        err::Err add_channel(int ch, int width, int height, int format, int fps, int depth, bool mirror, bool vflip, int fit);
        err::Err del_channel(int ch);
        err::Err del_channel_all();
        maixcam2::Frame *pop(int ch);
    private:
        void *_param = nullptr;
    };
};

#endif
