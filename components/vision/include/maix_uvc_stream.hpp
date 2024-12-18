#ifndef __MAIX_UVC_STREAM_HPP
#define __MAIX_UVC_STREAM_HPP

#include <bits/stdc++.h>
#include <functional>


#include "maix_err.hpp"
#include "maix_camera.hpp"
#include "maix_image.hpp"
#include "maix_basic.hpp"

namespace maix::uvc
{
    #define SHM_NAME "/uvc_shared_mem_yuyv"
    #define MAX_FRAMEBUFFER_SIZE (2560 * 1440 * 3 / 2)
    #define SHM_SIZE (2 + 2 * MAX_FRAMEBUFFER_SIZE) // 假设 RGB 图像大小为 1920x1080，额外的2字节用于双缓冲同步

    typedef struct {
        uint8_t current_buffer; // Current buffer (0 or 1) for the consumer to read
        uint8_t consumer_reading; // Consumer is reading (0 or 1)
        uint8_t buffer[2][MAX_FRAMEBUFFER_SIZE]; // Two buffers for video frames
    } shared_mem_t;

    /**
        * @brief helper_fill_mjpg_image
        * @param buf to be filled
        * @param size to be set
        * @param img image::Image
        * @return int
        * @maixpy maix.uvc.helper_fill_mjpg_image
    */
    int helper_fill_mjpg_image(void* buf, uint32_t* size, image::Image *img);

    /**
        * UvcServer class
        * @maixpy maix.uvc.UvcServer
        */
    class UvcServer {
    public:
        // 设置回调函数，可以是普通函数，也可以是 lambda 或其他可调用对象
        /**
            * @brief set UvcServer's cb
            * @param cb callback function 
            * @return void
            * @maixpy maix.uvc.UvcServer.set_cb
        */
        void set_cb(std::function<int(void* buf, uint32_t* size)> cb) {
            this->_cb = cb;
        }

        /**
            * @brief run UvcServer
            * @return void
            * @maixpy maix.uvc.UvcServer.run
        */
        void run();

        /**
            * @brief stop UvcServer
            * @return void
            * @maixpy maix.uvc.UvcServer.stop
        */
        void stop();

        // 提供给 C 的回调接口
        static int c_callback(void *buf, uint32_t *size) {
            if (_instance && _instance->_cb) {
                return _instance->_cb(buf, size);  // 调用注册的 C++ 回调
            }
            return -1;  // 如果没有设置回调，返回错误码
        }

        /**
            * @brief Construct a new jpeg server object
            * @note You can get the picture stream through http://host:port/stream, you can also get it through http://ip:port, and you can add personal style through set_html() at this time
            * @maixpy maix.uvc.UvcServer.__init__
            * @maixcdk maix.uvc.UvcServer.UvcServer
            */
        UvcServer(std::function<int(void* buf, uint32_t* size)> cb = nullptr): _cb(cb) {}

        ~UvcServer() {
            maix::log::info("~UvcServer invoked\r\n");
            this->stop();
        } // 析构函数私有化
    private:
        // 禁止拷贝构造和赋值
        UvcServer(const UvcServer&) = delete;
        UvcServer& operator=(const UvcServer&) = delete;

        static UvcServer *_instance;  // 使用 _instance 管理实例

        std::unique_ptr<std::thread> uvc_daemon_thrd;
        std::function<int(void* buf, uint32_t* size)> _cb;  // 存储回调函数
    };


    /**
        * UvcStreamer class
        * @maixpy maix.uvc.UvcStreamer
        */
    class UvcStreamer
    {
    public:
        /**
            * @brief Construct a new jpeg streamer object
            * @note You can get the picture stream through http://host:port/stream, you can also get it through http://ip:port, and you can add personal style through set_html() at this time
            * @maixpy maix.uvc.UvcStreamer.__init__
            * @maixcdk maix.uvc.UvcStreamer.UvcStreamer
            */
        UvcStreamer();
        ~UvcStreamer();

        /**
            * @brief Write data to uvc
            * @param img image object
            * @return error code, err::ERR_NONE means success, others means failed
            * @maixpy maix.uvc.UvcStreamer.show
        */
        err::Err show(image::Image *img);

        /**
            * @brief use mjpg on uvc
            * @param b using mjpg: 0 for NOT, others to use
            * @return void
            * @maixpy maix.uvc.UvcStreamer.use_mjpg
        */
        void use_mjpg(uint32_t b=1) {
            this->_using_jpeg = !!b;
        }

    private:
        unsigned int _using_jpeg;
        shared_mem_t *shm_ptr;
    };
} // namespace maix::uvc


#endif // __MAIX_UVC_STREAM_HPP