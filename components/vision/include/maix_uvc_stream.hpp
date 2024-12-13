#ifndef __MAIX_UVC_STREAM_HPP
#define __MAIX_UVC_STREAM_HPP

#include <bits/stdc++.h>
#include <functional>

using namespace std;

#include "maix_err.hpp"
#include "maix_camera.hpp"
#include "maix_image.hpp"
#include "maix_basic.hpp"

namespace maix::uvc
{
    /**
     * cannot include it, because it brokes maixpy_wrapper.cpp:1027:38: error: expected unqualified-id before numeric constant
     *  1027 |         .value("SEEK_SET", fs::SEEK::SEEK_SET)
     */ 
    // #include <fcntl.h>
    #define O_RDWR 2
    #include <sys/mman.h>

    #define atomic_load(ptr) __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
    #define atomic_store(ptr, value) __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST)

    #define SHM_NAME "/uvc_shared_mem_yuyv"
    #define MAX_FRAMEBUFFER_SIZE (1920 * 1080 * 3 / 2)
    #define SHM_SIZE (2 + 2 * MAX_FRAMEBUFFER_SIZE) // 假设 RGB 图像大小为 1920x1080，额外的2字节用于双缓冲同步

    typedef struct {
        uint8_t current_buffer; // Current buffer (0 or 1) for the consumer to read
        uint8_t consumer_reading; // Consumer is reading (0 or 1)
        uint8_t buffer[2][MAX_FRAMEBUFFER_SIZE]; // Two buffers for video frames
    } shared_mem_t;


    // ⚠️ 辅助宏，限制值范围
    #define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

    // 🔥 RGB 转 YUYV 函数
    static void rgb_to_yuyv_pixel(unsigned char r, unsigned char g, unsigned char b, unsigned char *y, unsigned char *u, unsigned char *v) {
        *y = CLAMP(( 77 * r + 150 * g +  29 * b) >> 8, 0, 255); // Y 公式
        *u = CLAMP(((-43 * r -  85 * g + 128 * b) >> 8) + 128, 0, 255); // U 公式
        *v = CLAMP((( 128 * r - 107 * g -  21 * b) >> 8) + 128, 0, 255); // V 公式
    }

    // 🚀 将 RGB 数据拷贝到 YUYV 图
    static void copy_rgb_to_yuyv(
        unsigned char *data,      // 源 RGB 图像数据 (w1 * h1 * 3)
        int w1, int h1,           // 源 RGB 图像的宽高
        unsigned char *img_data,  // 目标 YUYV 图像数据 (w2 * h2 * 2)
        int w2, int h2,           // 目标 YUYV 图像的宽高
        int x1, int y1            // 在目标图上绘制的起始位置 (x1, y1)
    ) {
        int start_x = CLAMP(x1, 0, w2 - 1); // 限制起始位置
        int start_y = CLAMP(y1, 0, h2 - 1);
        int copy_width = CLAMP(w2 - start_x, 0, w1); // 计算可拷贝的宽度
        int copy_height = CLAMP(h2 - start_y, 0, h1); // 计算可拷贝的高度

        // printf("Copying RGB (%dx%d) to YUYV (%dx%d) at (%d, %d)\n", w1, h1, w2, h2, x1, y1);
        // printf("Copy region width: %d, height: %d\n", copy_width, copy_height);

        for (int row = 0; row < copy_height; row++) {
            for (int col = 0; col < copy_width; col += 2) {
                int src_index1 = (row * w1 + col) * 3; // RGB数据位置 (每像素 3 字节)
                int src_index2 = (row * w1 + col + 1) * 3; // 第二个像素的位置

                int dst_index = ((start_y + row) * w2 + (start_x + col)) * 2; // YUYV 位置 (每像素 2 字节)

                unsigned char r1 = data[src_index1 + 0];
                unsigned char g1 = data[src_index1 + 1];
                unsigned char b1 = data[src_index1 + 2];

                unsigned char r2 = 0, g2 = 0, b2 = 0;
                if (col + 1 < copy_width) {
                    r2 = data[src_index2 + 0];
                    g2 = data[src_index2 + 1];
                    b2 = data[src_index2 + 2];
                }

                unsigned char y0, u0, v0;
                unsigned char y1;
                rgb_to_yuyv_pixel(r1, g1, b1, &y0, &u0, &v0); // 第一个像素
                rgb_to_yuyv_pixel(r2, g2, b2, &y1, &u0, &v0); // 第二个像素

                img_data[dst_index + 0] = y0; // Y0
                img_data[dst_index + 1] = u0; // U
                img_data[dst_index + 2] = y1; // Y1
                img_data[dst_index + 3] = v0; // V
            }
        }
    }


    /**
     * get_supported_formats function
     * @maixpy maix.uvc.get_supported_formats
     */
    std::vector<std::string> get_supported_formats() {
        vector<string> vec_str;

        vec_str.push_back(string("yuyv\t640x360"));
        vec_str.push_back(string("mjpg\t640x360"));
        vec_str.push_back(string("sadad\t1280x720"));

        return vec_str;
    }

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
         * @param host http host
         * @param port http port, default is 8000
         * @param client_number the max number of client
         * @maixpy maix.uvc.UvcStreamer.__init__
         * @maixcdk maix.uvc.UvcStreamer.UvcStreamer
         */
        UvcStreamer(unsigned  index=0): _index(index), _using_jpeg(0) {
            // 1. 打开共享内存
            int shm_fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR);
            if (shm_fd == -1) {
                log::error("shm_open failed");
                exit(EXIT_FAILURE);
            }

            // 2. 将共享内存映射到进程的地址空间
            void *shared_mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            close(shm_fd);
            if (shared_mem == MAP_FAILED) {
                log::error("mmap failed");
                exit(EXIT_FAILURE);
            }
            this->shm_ptr = (shared_mem_t *)shared_mem;  // 传递原始指针
        }
        ~UvcStreamer() {
            munmap(this->shm_ptr, SHM_SIZE);
        }

        /**
         * @brief Write data to uvc
         * @param img image object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.uvc.UvcStreamer.show
        */
        err::Err show(image::Image *img) {
            uint8_t *img_fb= NULL;
            

            log::error("_using_jpeg: %u\r\n", this->_using_jpeg);
            if (!this->_using_jpeg) {
                image::Image *img_rgb888 = img;
                if (img->format() != image::Format::FMT_RGB888)
                    img_rgb888 = img->to_format(image::Format::FMT_RGB888);

                atomic_store(&shm_ptr->current_buffer, 0xef & (atomic_load(&shm_ptr->consumer_reading) ^ shm_ptr->current_buffer));
                img_fb = (uint8_t *)(shm_ptr->buffer[0xf & shm_ptr->current_buffer]);
                copy_rgb_to_yuyv((unsigned char *)img_rgb888->data(),img_rgb888->width(), img_rgb888->height(), img_fb, 640, 360, 0, 0);

                if (img->format() != image::Format::FMT_RGB888)
                    delete img_rgb888;
            } else {
                image::Image *img_mjpg = img;
                if (img->format() != image::Format::FMT_JPEG)
                    img_mjpg = img->to_jpeg(80);
                
                if (img_mjpg) {
                    // log::error("width: %d, height: %d, format: %d, data: %p, size: %d\r\n", img_mjpg->width(), img_mjpg->height(), img_mjpg->format(), img_mjpg->data(), img_mjpg->data_size());

                    atomic_store(&shm_ptr->current_buffer, 0x80 | (atomic_load(&shm_ptr->consumer_reading) ^ shm_ptr->current_buffer));
                    img_fb = (uint8_t *)(shm_ptr->buffer[0xf & shm_ptr->current_buffer]) + 4;
                    ((uint32_t*)img_fb)[-1] = img_mjpg->data_size();
                    memcpy(img_fb, img_mjpg->data(), img_mjpg->data_size());

                    // log::error("width: %d, height: %d, format: %d, to_addr: %p\r\n", img_rgb888->width(), img_rgb888->height(), img_rgb888->format(), img_fb);
                    if (img->format() != image::Format::FMT_JPEG)
                        delete img_mjpg;
                }
            }
            return err::ERR_NONE;
        }
    
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
        unsigned int _index;
        unsigned int _using_jpeg;
        shared_mem_t *shm_ptr;
    };
} // namespace maix::uvc


#endif // __MAIX_UVC_STREAM_HPP