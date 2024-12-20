#include <bits/stdc++.h>
#include <functional>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "maix_uvc_stream.hpp"

extern "C" {

extern int uvc_main_exist;
extern int uvc_main(int argc, char *argv[]);

int uvc_video_fill_mjpg_buffer(void *buf, uint32_t *size) {
    return -1;
}

}

namespace maix::uvc
{
int helper_fill_mjpg_image(void* buf, uint32_t* size, image::Image *img) {
    return -1;
}

// 静态成员定义
UvcServer *UvcServer::_instance = nullptr;

void UvcServer::run() { }

void UvcServer::stop() { }

UvcStreamer::UvcStreamer(): _using_jpeg(0) { }

UvcStreamer::~UvcStreamer() { }


err::Err UvcStreamer::show(image::Image *img) {
    return err::ERR_NOT_IMPL;
}

}