#include <bits/stdc++.h>
#include <functional>

#include "uvcServer.hpp"


extern "C" {

extern int uvc_main_exist;
extern int uvc_main(int argc, char *argv[]);

int uvc_video_fill_mjpg_buffer(void *buf, uint32_t *size) {
    return UvcServer::c_callback(buf, size);
}

}

// 静态成员定义
UvcServer *UvcServer::_instance = nullptr;
std::once_flag UvcServer::_onceFlag;

void UvcServer::run() {
    if (this->_cb == nullptr)
        return;

    if (this->uvc_daemon_thrd != nullptr)
        return;

    this->uvc_daemon_thrd = std::make_unique<std::thread>(std::thread([] () {
        const char* uvc_main_args[] = {"self", "-u", "/dev/video0", "-d", "-i", "/bin/cat_224.jpg"};
        uvc_main(sizeof(uvc_main_args)/sizeof(uvc_main_args[0]), (char**)uvc_main_args);
    }));
}

void UvcServer::stop() {
    if (this->uvc_daemon_thrd == nullptr)
        return;

    if (!uvc_main_exist)
        uvc_main_exist = 1;

    this->uvc_daemon_thrd->join();
}
