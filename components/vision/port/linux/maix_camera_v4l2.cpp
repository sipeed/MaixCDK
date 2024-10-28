/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_camera.hpp"
#include "maix_basic.hpp"
#include <dirent.h>


#include <stdint.h>
#include <stdbool.h>
#include <linux/videodev2.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string.h>
#include <vector>
#include <assert.h>
#include <sys/mman.h>
#include <poll.h>
#include "maix_err.hpp"
#include "maix_log.hpp"
#include "maix_image.hpp"

#ifndef V4L2_PIX_FMT_RGBA32
#define V4L2_PIX_FMT_RGBA32 v4l2_fourcc('R', 'G', 'B', 'A') /* 32  RGBA-8-8-8-8    */
#endif
#ifndef V4L2_PIX_FMT_BGRA32
#define V4L2_PIX_FMT_BGRA32 v4l2_fourcc('B', 'G', 'R', 'A') /* 32  BGRA-8-8-8-8    */
#endif

namespace maix::camera
{
    static int xioctl(int fh, int request, void *arg)
    {
        int r;
        do
        {
            r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);
        return r;
    }

    static int choose_format(int target, const std::vector<uint32_t> &formats)
    {
        int final = 0;
        if (!(target == image::FMT_RGB888 || target == image::FMT_RGBA8888 ||
              target == image::FMT_BGR888 || target == image::FMT_BGRA8888))
            throw std::runtime_error("format not support");

        for (size_t i = 1; i < formats.size(); i++)
        {
            if (target == image::FMT_RGB888 && formats[i] == V4L2_PIX_FMT_RGB24)
            {
                log::debug("raw choose RGB888 mode\n");
                final = i;
                break;
            }
            if (target == image::FMT_BGR888 && formats[i] == V4L2_PIX_FMT_BGR24)
            {
                log::debug("raw choose BGR888 mode\n");
                final = i;
                break;
            }
            if (target == image::FMT_BGRA8888 && formats[i] == V4L2_PIX_FMT_BGR32)
            {
                log::debug("raw choose BGRA8888 mode\n");
                final = i;
                break;
            }
            if (formats[i] == V4L2_PIX_FMT_YUYV)
            {
                log::debug("raw choose YUYV 422 mode\n");
                final = i;
                //     if(target == image::FMT_YUV422)
                //         break;
            }
        }
        return final;
    }

    static bool need_convert_format(uint32_t raw_format, int target_format)
    {
        if (!(target_format == image::FMT_RGB888 || target_format == image::FMT_RGBA8888 ||
              target_format == image::FMT_BGR888 || target_format == image::FMT_BGRA8888))
            throw std::runtime_error("format not support");
        return !((target_format == image::FMT_RGB888 && raw_format == V4L2_PIX_FMT_RGB24) ||
                 (target_format == image::FMT_RGBA8888 && raw_format == V4L2_PIX_FMT_RGBA32) ||
                 (target_format == image::FMT_BGR888 && raw_format == V4L2_PIX_FMT_BGR24) ||
                 (target_format == image::FMT_BGRA8888 && raw_format == V4L2_PIX_FMT_BGRA32));
    }

    static void *alloc_buffer(int width, int height, int format)
    {
        if (!(format == image::FMT_RGB888 || format == image::FMT_RGBA8888 ||
              format == image::FMT_BGR888 || format == image::FMT_BGRA8888))
            throw std::runtime_error("format not support");
        if (format == image::FMT_RGBA8888 || format == image::FMT_BGRA8888)
            return malloc(width * height * 4);
        // if (format == image::FMT_YUV422)
        //     return malloc(width * height * 2);
        return malloc(width * height * 3);
    }

    static void yuyv422_to_rgb888(void *src, void *dst, int width, int height)
    {
        uint8_t *rgb = (uint8_t *)dst;
        uint8_t *yuyv = (uint8_t *)src;
        uint8_t y0, u, y1, v;
        float r, g, b;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width;)
            {
                y0 = yuyv[0];
                u = yuyv[1];
                y1 = yuyv[2];
                v = yuyv[3];

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.402 * (v - 128);
                // g = y0 - 0.344 * (u - 128) - 0.714 * (v - 128);
                // b = y0 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                r = 1.164384 * (y0 - 16) + 1.79271 * (v - 128);
                g = 1.164384 * (y0 - 16) - 0.532909 * (v - 128) - 0.213249 * (u - 128);
                b = 1.164384 * (y0 - 16) + 2.112402 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.5748 * (v - 128);
                // g = y0 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y0 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.4746 * (v - 128)
                // g = y0 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y0 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);
                rgb[0] = (uint8_t)r;
                rgb[1] = (uint8_t)g;
                rgb[2] = (uint8_t)b;

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.402 * (v - 128);
                // g = y1 - 0.344 * (u - 128) - 0.714 * (v - 128);
                // b = y1 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                r = 1.164384 * (y1 - 16) + 1.79271 * (v - 128);
                g = 1.164384 * (y1 - 16) - 0.532909 * (v - 128) - 0.213249 * (u - 128);
                b = 1.164384 * (y1 - 16) + 2.112402 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.5748 * (v - 128);
                // g = y1 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y1 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.4746 * (v - 128)
                // g = y1 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y1 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);
                rgb[3] = (uint8_t)r;
                rgb[4] = (uint8_t)g;
                rgb[5] = (uint8_t)b;
                yuyv += 4;
                rgb += 6;
                x += 2;
            }
        }
    }

    static void yuyv422_to_bgr888(void *src, void *dst, int width, int height)
    {
        uint8_t *rgb = (uint8_t *)dst;
        uint8_t *yuyv = (uint8_t *)src;
        uint8_t y0, u, y1, v;
        float r, g, b;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width;)
            {
                y0 = yuyv[0];
                u = yuyv[1];
                y1 = yuyv[2];
                v = yuyv[3];

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                r = y0 + 1.402 * (v - 128);
                g = y0 - 0.344 * (u - 128) - 0.714 * (v - 128);
                b = y0 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.79271 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.532909 * (v - 128) - 0.213249 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.112402 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.5748 * (v - 128);
                // g = y0 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y0 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.4746 * (v - 128)
                // g = y0 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y0 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);
                rgb[0] = (uint8_t)b;
                rgb[1] = (uint8_t)g;
                rgb[2] = (uint8_t)r;

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                r = y1 + 1.402 * (v - 128);
                g = y1 - 0.344 * (u - 128) - 0.714 * (v - 128);
                b = y1 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.79271 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.532909 * (v - 128) - 0.213249 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.112402 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.5748 * (v - 128);
                // g = y1 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y1 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.4746 * (v - 128)
                // g = y1 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y1 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);
                rgb[3] = (uint8_t)b;
                rgb[4] = (uint8_t)g;
                rgb[5] = (uint8_t)r;
                yuyv += 4;
                rgb += 6;
                x += 2;
            }
        }
    }

    // V4L2_PIX_FMT_YUYV to RGBA8888
    static void yuyv422_to_rgba8888(void *src, void *dst, int width, int height)
    {
        uint8_t *rgba = (uint8_t *)dst;
        uint8_t *yuyv = (uint8_t *)src;
        uint8_t y0, u, y1, v;
        float r, g, b;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width;)
            {
                y0 = yuyv[0];
                u = yuyv[1];
                y1 = yuyv[2];
                v = yuyv[3];
                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.402 * (v - 128);
                // g = y0 - 0.344 * (u - 128) - 0.714 * (v - 128);
                // b = y0 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                r = 1.164384 * (y0 - 16) + 1.793 * (v - 128);
                g = 1.164384 * (y0 - 16) - 0.534 * (v - 128) - 0.213 * (u - 128);
                b = 1.164384 * (y0 - 16) + 2.115 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.5748 * (v - 128);
                // g = y0 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y0 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.4746 * (v - 128)
                // g = y0 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y0 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);
                rgba[0] = (uint8_t)r;
                rgba[1] = (uint8_t)g;
                rgba[2] = (uint8_t)b;
                rgba[3] = 255;

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.402 * (v - 128);
                // g = y1 - 0.344 * (u - 128) - 0.714 * (v - 128);
                // b = y1 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                r = 1.164384 * (y1 - 16) + 1.793 * (v - 128);
                g = 1.164384 * (y1 - 16) - 0.534 * (v - 128) - 0.213 * (u - 128);
                b = 1.164384 * (y1 - 16) + 2.115 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.5748 * (v - 128);
                // g = y1 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y1 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.4746 * (v - 128)
                // g = y1 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y1 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);
                rgba[4] = (uint8_t)r;
                rgba[5] = (uint8_t)g;
                rgba[6] = (uint8_t)b;
                rgba[7] = 255;
                yuyv += 4;
                rgba += 8;
                x += 2;
            }
        }
    }

    // V4L2_PIX_FMT_YUYV to RGBA8888
    static void yuyv422_to_bgra8888(void *src, void *dst, int width, int height)
    {
        uint8_t *rgba = (uint8_t *)dst;
        uint8_t *yuyv = (uint8_t *)src;
        uint8_t y0, u, y1, v;
        float r, g, b;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width;)
            {
                y0 = yuyv[0];
                u = yuyv[1];
                y1 = yuyv[2];
                v = yuyv[3];

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.402 * (v - 128);
                // g = y0 - 0.344 * (u - 128) - 0.714 * (v - 128);
                // b = y0 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                r = 1.164384 * (y0 - 16) + 1.793 * (v - 128);
                g = 1.164384 * (y0 - 16) - 0.534 * (v - 128) - 0.213 * (u - 128);
                b = 1.164384 * (y0 - 16) + 2.115 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.5748 * (v - 128);
                // g = y0 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y0 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y0 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y0 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y0 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y0 + 1.4746 * (v - 128)
                // g = y0 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y0 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);

                rgba[0] = (uint8_t)b;
                rgba[1] = (uint8_t)g;
                rgba[2] = (uint8_t)r;
                rgba[3] = 255;

                // BT601 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.596 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.813 * (v - 128) - 0.391 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.018 * (u - 128);
                // BT601 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.402 * (v - 128);
                // g = y1 - 0.344 * (u - 128) - 0.714 * (v - 128);
                // b = y1 + 1.772 * (u - 128);
                // BT709 YUV data is limited range, to RGB[0, 255]
                r = 1.164384 * (y1 - 16) + 1.793 * (v - 128);
                g = 1.164384 * (y1 - 16) - 0.534 * (v - 128) - 0.213 * (u - 128);
                b = 1.164384 * (y1 - 16) + 2.115 * (u - 128);
                // BT709 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.5748 * (v - 128);
                // g = y1 - 0.1873 * (u - 128) - 0.4681 * (v - 128);
                // b = y1 + 1.8556 * (u - 128);
                // BT2020 YUV data is limited range, to RGB[0, 255]
                // r = 1.164384 * (y1 - 16) + 1.67867 * (v - 128);
                // g = 1.164384 * (y1 - 16) - 0.65042 * (v - 128) - 0.187326 * (u - 128);
                // b = 1.164384 * (y1 - 16) + 2.14177 * (u - 128);
                // BT2020 YUV data is full range, to RGB[0, 255]
                // r = y1 + 1.4746 * (v - 128)
                // g = y1 - 0.164553 * (u - 128) - 0.571353 * (u - 128)
                // b = y1 + 1.8814 * (u - 128)

                r = r > 255 ? 255 : (r < 0 ? 0 : r);
                g = g > 255 ? 255 : (g < 0 ? 0 : g);
                b = b > 255 ? 255 : (b < 0 ? 0 : b);

                rgba[4] = (uint8_t)b;
                rgba[5] = (uint8_t)g;
                rgba[6] = (uint8_t)r;
                rgba[7] = 255;
                yuyv += 4;
                rgba += 8;
                x += 2;
            }
        }
    }

    static int convert_format(void *raw_buff, void *buff, uint32_t raw_format, int format, int width, int height)
    {
        int ret = 0;

        if (!(format == image::FMT_RGB888 || format == image::FMT_BGR888 ||
              format == image::FMT_RGBA8888 || format == image::FMT_BGRA8888))
            throw std::runtime_error("format not support");
        if (raw_format != V4L2_PIX_FMT_YUYV)
            throw std::runtime_error("raw format not support");

        if (raw_format == V4L2_PIX_FMT_YUYV)
        {
            switch (format)
            {
            case image::FMT_RGB888:
                yuyv422_to_rgb888(raw_buff, buff, width, height);
                break;
            case image::FMT_BGR888:
                yuyv422_to_bgr888(raw_buff, buff, width, height);
                break;
            case image::FMT_RGBA8888:
                yuyv422_to_rgba8888(raw_buff, buff, width, height);
                break;
            case image::FMT_BGRA8888:
                yuyv422_to_bgra8888(raw_buff, buff, width, height);
                break;
            default:
                ret = EINVAL;
                break;
            }
        }

        return ret;
    }

    static bool set_regs_flag = false;

    class CameraV4L2
    {
    public:
        CameraV4L2(const std::string device, int width, int height, image::Format format, int buff_num)
        {
            this->device = device;
            this->format = format;
            this->width = width;
            this->height = height;
            this->buffer_num = buff_num;
            for (int i = 0; i < buff_num; i++)
            {
                buffers.push_back(NULL);
                buffers_len.push_back(0);
            }
            fd = -1;
            queue_id = -1;
            buff = NULL;
            buff_alloc = false;
        }

        CameraV4L2(const std::string device, int ch, int width, int height, image::Format format, int buff_num)
            : CameraV4L2(device, width, height, format, 4)
        {
        }

        ~CameraV4L2()
        {
            close();
        }

        bool is_support_format(image::Format format)
        {
            // auto convert in read, so always support formats
            return true;
        }

        err::Err open(int width, int height, image::Format format, int buff_num)
        {
            struct v4l2_capability cap;
            struct v4l2_format fmt;

            if (fd >= 0)
            {
                log::error("Already open\n");
                return err::ERR_NOT_PERMIT;
            }
            log::debug("open camera device %s\n", device.c_str());

            this->format = format;
            this->width = width > 0 ? width : this->width;
            this->height = height > 0 ? height : this->height;
            this->buffer_num = buff_num;

            fd = ::open(device.c_str(), O_RDWR | O_NONBLOCK, 0);
            if (fd == -1)
            {
                log::error("open device %s failed\n", device.c_str());
                return err::ERR_ARGS;
            }
            if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
            {
                if (EINVAL == errno)
                {
                    log::error("%s is not V4L2 device\n", device.c_str());
                    return err::ERR_ARGS;
                }
                else
                {
                    log::error("\nError in ioctl VIDIOC_QUERYCAP\n\n");
                    return err::ERR_RUNTIME;
                }
            }
            log::debug("cap.driver: %s, cap.card: %s\n", cap.driver, cap.card);
            log::debug("cap.capabilities: 0x%x\n", cap.capabilities);
            if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
            {
                log::error("%s is no video capture device\n", device.c_str());
                return err::ERR_ARGS;
            }

            if (!(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE | V4L2_CAP_STREAMING)))
            {
                log::error("%s does not support read i/o\n", device.c_str());
                return err::ERR_NOT_PERMIT;
            }
            if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
                log::debug("v4l2 dev support capture\n");
            if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
                log::debug("v4l2 dev support output\n");
            if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
                log::debug("v4l2 dev support overlay\n");
            if (cap.capabilities & V4L2_CAP_STREAMING)
                log::debug("v4l2 dev support streaming\n");
            if (cap.capabilities & V4L2_CAP_READWRITE)
                log::debug("v4l2 dev support read write\n");

            struct v4l2_input input;

            input.index = 0;
            while (!ioctl(fd, VIDIOC_ENUMINPUT, &input))
            {
                log::debug("input %d: %s\n", input.index, input.name);
                ++input.index;
            }

            input.index = 0;
            if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0)
            {
                log::error("ctl VIDIOC_S_INPUT failed\n");
                return err::ERR_RUNTIME;
            }

            typedef struct
            {
                int w;
                int h;
            } frame_size_t;
            struct v4l2_fmtdesc fmtdesc;
            std::vector<uint32_t> fmts;
            std::vector<frame_size_t> frame_sizes;
            struct v4l2_frmsizeenum frmsize;
            frame_size_t max_frame_size;

            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmtdesc.index = 0;

            while (!ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc))
            {
                frame_size_t frame_size;
                log::debug("supported fmt %d: 0x%x, %s\n", fmtdesc.index, fmtdesc.pixelformat, fmtdesc.description);
                fmts.push_back(fmtdesc.pixelformat);
                // get resolution
                memset(&frmsize, 0, sizeof(frmsize));
                frmsize.pixel_format = fmtdesc.pixelformat;
                frmsize.index = fmtdesc.index;
                if (-1 == xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize))
                {
                    log::error("VIDIOC_ENUM_FRAMESIZES error: %d\n", errno);
                    return err::ERR_RUNTIME;
                }
                // printf("frmsize.type: %d, frmsize.discrete: %d %d, frmsize.stepwise: %d %d\n", frmsize.type, frmsize.discrete.width, frmsize.discrete.height, frmsize.stepwise.max_width, frmsize.stepwise.max_height);
                if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
                {
                    log::debug("type DISCRETE: %dx%d\n",
                               frmsize.discrete.width,
                               frmsize.discrete.height);
                    frame_size.w = frmsize.discrete.width;
                    frame_size.h = frmsize.discrete.height;
                }
                else if (frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
                {
                    log::debug("type STEPWISE: %dx%d\n",
                               frmsize.stepwise.max_width,
                               frmsize.stepwise.max_height);
                    frame_size.w = frmsize.stepwise.max_width;
                    frame_size.h = frmsize.stepwise.max_height;
                }
                else
                {
                    log::debug("type CONTINUOUS: %dx%d\n",
                               frmsize.stepwise.max_width,
                               frmsize.stepwise.max_height);
                    frame_size.w = frmsize.stepwise.max_width;
                    frame_size.h = frmsize.stepwise.max_height;
                }
                assert(frame_size.w > 0 && frame_size.h > 0);
                frame_sizes.push_back(frame_size);
                fmtdesc.index++;
            }
            log::debug("supported fmts num: %ld\n", fmts.size());
            int format_idx = choose_format(format, fmts);
            log::debug("choose format idx: %d\n", format_idx);
            raw_format = fmts[format_idx];
            max_frame_size = frame_sizes[format_idx];
            if (width <= 0)
                width = max_frame_size.w;
            if (height <= 0)
                height = max_frame_size.h;

            // crop VIDIOC_CROPCAP
            // struct v4l2_cropcap cropcap;
            // struct v4l2_crop crop;
            // // TODO: resize crop by driver(if driver support)
            // //  int w_resize = 0, h_resize = 0;
            // //  w_resize = 1.0 *  height / max_frame_size.h * max_frame_size.w;
            // //  printf("w_resize: %d, h_resize: %d\n", w_resize, h_resize);

            // memset(&cropcap, 0, sizeof(cropcap));
            // cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            // if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
            // {
            //     // cropcap's value will be the last time set by VIDIOC_S_CROP
            //     log::debug("cropcap: %d %d %d %d, defrect: %d %d %d %d\n", cropcap.bounds.left, cropcap.bounds.top, cropcap.bounds.width, cropcap.bounds.height,
            //         cropcap.defrect.left, cropcap.defrect.top, cropcap.defrect.width, cropcap.defrect.height);
            //     memset(&crop, 0, sizeof(crop));
            //     crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            //     if (-1 == xioctl(fd, VIDIOC_G_CROP, &crop))
            //     {
            //         log::warn("VIDIOC_G_CROP error: %d\n", errno);
            //         switch (errno)
            //         {
            //         case EINVAL:
            //             log::warn("crop not supported on this camera driver\n");
            //             /* Cropping not supported. */
            //             break;
            //         default:
            //             /* Errors ignored. */
            //             break;
            //         }
            //     }

            //     // // crop.c = cropcap.defrect; /* reset to default */
            //     // crop.c.left = (w_resize - width) / 2;
            //     // crop.c.top = 0;
            //     // crop.c.width = width;
            //     // crop.c.height = height;

            //     // if (-1 == xioctl(cam->fd, VIDIOC_S_CROP, &crop))
            //     // {
            //     //     printf("VIDIOC_S_CROP error: %d\n", errno);
            //     //     switch (errno)
            //     //     {
            //     //     case EINVAL:
            //     //         /* Cropping not supported. */
            //     //         break;
            //     //     default:
            //     //         /* Errors ignored. */
            //     //         break;
            //     //     }
            //     // }
            // }
            // else
            // {
            //     /* Errors ignored. */
            // }

            memset(&fmt, 0, sizeof(fmt));
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.width = width;
            fmt.fmt.pix.height = height;
            fmt.fmt.pix.pixelformat = raw_format;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
            if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
            {
                log::error("VIDIOC_S_FMT error: %d\n", errno);
                return err::ERR_RUNTIME;
            }

            memset(&fmt, 0, sizeof(fmt));
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
            {
                log::error("VIDIOC_S_FMT error: %d\n", errno);
                return err::ERR_RUNTIME;
            }
            if ((int)fmt.fmt.pix.width != width || (int)fmt.fmt.pix.height != height || fmt.fmt.pix.pixelformat != raw_format)
            {
                log::error("VIDIOC_S_FMT failed, set %dx%d, format 0x%x, but supported is %dx%d, format 0x%x\n",
                           width, height, raw_format, fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);
                return err::ERR_ARGS;
            }

            // set buffer
            struct v4l2_requestbuffers req = {0};

            req.count = buffer_num;
            req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            req.memory = V4L2_MEMORY_MMAP;

            if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0)
            {
                log::error("ERR(%s):VIDIOC_REQBUFS failed\n", __func__);
                return err::ERR_RUNTIME;
            }
            if (req.count < (size_t)buffer_num)
            {
                /* You may need to free the buffers here. */
                log::error("Not enough buffer memory\\n");
                return err::ERR_NO_MEM;
            }

            // map address
            struct v4l2_buffer v4l2_buffer;
            int i = 0;
            int ret = 0;

            for (i = 0; i < buffer_num; i++)
            {
                memset(&v4l2_buffer, 0, sizeof(struct v4l2_buffer));
                v4l2_buffer.index = i; // 想要查询的缓存
                v4l2_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4l2_buffer.memory = V4L2_MEMORY_MMAP;

                /* 查询缓存信息 */
                ret = ioctl(fd, VIDIOC_QUERYBUF, &v4l2_buffer);
                if (ret < 0)
                {
                    log::error("Unable to query buffer.\n");
                    return err::Err::ERR_RUNTIME;
                }

                /* 映射 */
                buffers[i] = mmap(NULL /* start anywhere */,
                                  v4l2_buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                                  fd, v4l2_buffer.m.offset);
                buffers_len[i] = v4l2_buffer.length;
                if (buffers[i] == MAP_FAILED)
                {
                    /* If you do not exit here you should unmap() and free()
                    the buffers mapped so far. */
                    for (int j = 0; j < i; j++)
                        munmap(buffers[j], buffers_len[j]);
                    log::error("Unable to map buffer.\n");
                    return err::ERR_NO_MEM;
                }
                log::debug("buffer %d: %p, len: %d, offset: %u\n", i, buffers[i], buffers_len[i], v4l2_buffer.m.offset);
            }
            for (i = 0; i < buffer_num; i++)
            {
                memset(&v4l2_buffer, 0, sizeof(struct v4l2_buffer));
                v4l2_buffer.index = i; // 想要放入队列的缓存
                v4l2_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4l2_buffer.memory = V4L2_MEMORY_MMAP;

                ret = ioctl(fd, VIDIOC_QBUF, &v4l2_buffer);
                if (ret < 0)
                {
                    log::error("Unable to queue buffer.\n");
                    return err::ERR_RUNTIME;
                }
            }

            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (ioctl(fd, VIDIOC_STREAMON, &type) < 0)
            {
                log::error("ERR(%s):VIDIOC_STREAMON failed\n", __func__);
                return err::ERR_RUNTIME;
            }

            return err::ERR_NONE;
        } // open

        // read
        image::Image *read(void *buff = NULL, size_t buff_size = 0)
        {
            struct v4l2_buffer v4l2_buf;
            if (!buff)
                buff = this->buff;
            if (fd < 0)
            {
                log::error("Camera not open\n");
                return NULL;
            }
            //
            if (queue_id >= 0)
            {
                memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
                v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4l2_buf.memory = V4L2_MEMORY_MMAP;
                v4l2_buf.index = queue_id;
                if (ioctl(fd, VIDIOC_QBUF, &v4l2_buf) < 0)
                {
                    log::error("ERR(%s):VIDIOC_QBUF failed\n", __func__);
                    queue_id = -1;
                    return NULL;
                }
                queue_id = -1;
            }

            struct pollfd poll_fds[1];

            poll_fds[0].fd = fd;
            poll_fds[0].events = POLLIN; // 等待可读

            poll(poll_fds, 1, 10000);

            struct v4l2_buffer buffer = {0};

            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;

            if (ioctl(fd, VIDIOC_DQBUF, &buffer) < 0)
            {
                log::error("ERR(%s):VIDIOC_DQBUF failed, dropped frame\n", __func__);
                return NULL;
            }

            if (need_convert_format(raw_format, format))
            {
                if (!buff)
                {
                    buff = alloc_buffer(width, height, format);
                    if (!buff)
                    {
                        log::error("alloc buffer failed\n");
                        return NULL;
                    }
                    this->buff = buff;
                    buff_alloc = true;
                }
                convert_format(buffers[buffer.index], buff, raw_format, format, width, height);

                // release buffer
                memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
                v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                v4l2_buf.memory = V4L2_MEMORY_MMAP;
                v4l2_buf.index = buffer.index;

                if (ioctl(fd, VIDIOC_QBUF, &v4l2_buf) < 0)
                {
                    log::error("ERR(%s):VIDIOC_QBUF 2 failed\n", __func__);
                    return NULL;
                }
                return new image::Image(width, height, format, (uint8_t *)buff, -1, true);
            }
            else
            {
                queue_id = buffer.index;
                return new image::Image(width, height, format, (uint8_t *)buffers[buffer.index], -1, true);
            }
        } // read

        void close()
        {
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (fd >= 0)
            {
                if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0)
                {
                    log::error("ERR(%s):VIDIOC_STREAMOFF failed\n", __func__);
                    return;
                }
                for (int i = 0; i < buffer_num; ++i)
                    munmap(buffers[i], buffers_len[i]);
                ::close(fd);
                fd = -1;
            }
            if (buff_alloc)
            {
                free(buff);
                buff = NULL;
            }
        }

        camera::CameraV4L2 *add_channel(int width, int height, image::Format forma, int buff_num)
        {
            return NULL;
        }

        void clear_buff()
        {
            // VIDIOC_DQBUF all buffer
            struct v4l2_buffer buffer;
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buffer.memory = V4L2_MEMORY_MMAP;
            while (ioctl(fd, VIDIOC_DQBUF, &buffer) >= 0)
            {
                memset(&buffer, 0, sizeof(struct v4l2_buffer));
                buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buffer.memory = V4L2_MEMORY_MMAP;
            }
            // VIDIOC_QBUF all buffer
            for (int i = 0; i < buffer_num; ++i)
            {
                memset(&buffer, 0, sizeof(struct v4l2_buffer));
                buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buffer.memory = V4L2_MEMORY_MMAP;
                buffer.index = i;
                if (ioctl(fd, VIDIOC_QBUF, &buffer) < 0)
                {
                    log::error("ERR(%s):VIDIOC_QBUF failed\n", __func__);
                    return;
                }
            }
        }

        bool is_opened()
        {
            if (fd >= 2)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        int get_ch_nums()
        {
            return 1;
        }

        int get_channel() {
            return 0;
        }

        int hmirror(int en)
        {
            return -1;
        }

        int vflip(int en)
        {
            return -1;
        }

        int luma(int value)
        {
            return -1;
        }

        int constrast(int value)
        {
            return -1;
        }

        int saturation(int value)
        {
            return -1;
        }

        int exposure(int value)
        {
            return -1;
        }

        int gain(int value)
        {
            return -1;
        }

        int awb_mode(int value)
        {
            return -1;
        }

        int set_awb(int value)
        {
            return -1;
        }

        int exp_mode(int value)
        {
            return -1;
        }

        int set_windowing(std::vector<int> roi)
        {
            return -1;
        }
    private:
        std::string device;
        image::Format format;
        int fd;
        uint32_t raw_format;
        std::vector<void *> buffers;
        std::vector<int> buffers_len;
        int buffer_num;
        int queue_id; // user directly used buffer id
        int width;
        int height;
        void *buff;
        bool buff_alloc;
    };

    std::vector<std::string> list_devices()
    {
        // find to /dev/video*
        std::vector<std::string> devices;
        std::string path = "/dev";
        DIR *dir = opendir(path.c_str());
        if (dir == NULL)
        {
            return devices;
        }
        struct dirent *ptr;
        while ((ptr = readdir(dir)) != NULL)
        {
            if (ptr->d_type == DT_CHR)
            {
                std::string name = ptr->d_name;
                if (name.find("video") != std::string::npos)
                {
                    devices.push_back( path + "/" + name );
                }
            }
        }
        closedir(dir);
        // sort devices with name
        std::sort(devices.begin(), devices.end());
        // print devices
        for (size_t i = 0; i < devices.size(); i++)
        {
            log::debug("find device: %s\n", devices[i].c_str());
        }
        return devices;
    }

    void set_regs_enable(bool enable) {
        set_regs_flag = enable;
    }

    image::Image *Camera::read_raw() {
        err::check_raise(err::ERR_NOT_IMPL, "read_raw() not impl");
        return NULL;
    }

    err::Err Camera::set_fps(double fps) {
        return err::ERR_NOT_IMPL;
    }

    err::Err Camera::show_colorbar(bool enable)
    {
        // only set variable now
        // should control camera to show colorbar
        _show_colorbar = enable;
        return err::ERR_NONE;
    }

    static void generate_colorbar(image::Image &img)
    {
        int width = img.width();
        int height = img.height();
        int step = width / 8;
        int color_step = 255 / 7;
        int color = 0;
        uint8_t colors[8][3] = {
            {255, 255, 255},
            {255, 0, 0},
            {255, 127, 0},
            {255, 255, 0},
            {0, 255, 0},
            {0, 0, 255},
            {143, 0, 255},
            {0, 0, 0},
        };
        for (int i = 0; i < 8; i++)
        {
            image::Color _color(colors[i][0], colors[i][1], colors[i][2], 0, image::FMT_RGB888);
            img.draw_rect(i * step, 0, step, height, _color, -1);
            color += color_step;
        }
    }

    static char * _get_device(const char *device)
    {
        if (device)
        {
            return (char *)device;
        }
        else
        {
            std::vector<std::string> devices = list_devices();
            err::check_bool_raise(devices.size() > 0, "No camera device");
            return (char *)devices[0].c_str();
        }
    }

    std::string get_device_name() {
        std::string device_name = "";
        return device_name;
    }

    static CameraV4L2 *_impl;
    Camera::Camera(int width, int height, image::Format format, const char *device, double fps, int buff_num, bool open, bool raw)
    {
        err::Err e;
        err::check_bool_raise(_check_format(format), "Format not support");

        if (format == image::Format::FMT_RGB888 && width * height * 3 > 640 * 640 * 3) {
            log::warn("Note that we do not recommend using large resolution RGB888 images, which can take up a lot of memory!\r\n");
        }

        _width = (width == -1) ? 640 : width;
        _height = (height == -1) ? 480 : height;
        _format = format;
        _buff_num = buff_num;
        _show_colorbar = false;
        _open_set_regs = set_regs_flag;

        _fps = (fps == -1) ? 30 : fps;
        if (device ) {
            _device = _get_device(strlen(device) == 0 ? NULL : device);
        } else {
            _device = _get_device(NULL);
        }
        _impl = new CameraV4L2(_device, _width, _height, _format, _buff_num);


        if (open) {
            e = this->open(_width, _height, _format, _fps, _buff_num);
            err::check_raise(e, "camera open failed");
        }
    }

    Camera::~Camera()
    {
        if (this->is_opened()) {
            this->close();
        }
        delete (CameraV4L2*)_impl;
    }

    int Camera::get_ch_nums()
    {
        return 1;
    }

    int Camera::get_channel()
    {
        if (_impl == NULL)
            return err::ERR_NOT_INIT;

        if (!this->is_opened()) {
            return err::ERR_NOT_OPEN;
        }

        return _impl->get_channel();
    }

    bool Camera::_check_format(image::Format format) {
        if (format == image::FMT_RGB888 || format == image::FMT_BGR888
        || format == image::FMT_RGBA8888 || format == image::FMT_BGRA8888
        || format == image::FMT_YVU420SP || format == image::FMT_GRAYSCALE) {
            return true;
        } else {
            return false;
        }
    }

    err::Err Camera::open(int width, int height, image::Format format, double fps, int buff_num)
    {
        if (_impl == NULL)
            return err::Err::ERR_RUNTIME;

        int width_tmp = (width == -1) ? _width : width;
        int height_tmp = (height == -1) ? _height : height;
        image::Format format_tmp = (format == image::FMT_INVALID) ? _format : format;
        int fps_tmp = (fps == -1) ? 30 : fps;
        int buff_num_tmp =( buff_num == -1) ? _buff_num : buff_num;

        err::check_bool_raise(_check_format(format_tmp), "Format not support");

        if (this->is_opened()) {
            if (width == width_tmp && height == height_tmp && format == format_tmp && fps == fps_tmp && buff_num == buff_num_tmp) {
                return err::ERR_NONE;
            }
            this->close();  // Get new param, close and reopen
        }

        _width = width_tmp;
        _height = height_tmp;
        _fps = fps_tmp;
        _buff_num = buff_num_tmp;
        _format = format_tmp;
        _format_impl = _format;
        if(!_impl->is_support_format(_format))
        {
            if(_impl->is_support_format(image::FMT_RGB888))
                _format_impl = image::FMT_RGB888;
            else if(_impl->is_support_format(image::FMT_BGR888))
                _format_impl = image::FMT_BGR888;
            else if(_impl->is_support_format(image::FMT_YVU420SP))
                _format_impl = image::FMT_YVU420SP;
            else if(_impl->is_support_format(image::FMT_YUV420SP))
                _format_impl = image::FMT_YUV420SP;
            else if(_impl->is_support_format(image::FMT_RGBA8888))
                _format_impl = image::FMT_RGBA8888;
            else if(_impl->is_support_format(image::FMT_BGRA8888))
                _format_impl = image::FMT_BGRA8888;
            else if(_impl->is_support_format(image::FMT_GRAYSCALE))
                _format_impl = image::FMT_GRAYSCALE;
            else
                return err::ERR_ARGS;
        }

        return _impl->open(_width, _height, _format_impl, _buff_num);;
    }

    void Camera::close()
    {
        if (this->is_closed())
            return;
    }

    camera::Camera *Camera::add_channel(int width, int height, image::Format format, double fps, int buff_num, bool open)
    {
        return NULL;
    }

    bool Camera::is_opened()
    {
        return _is_opened;
    }

    image::Image *Camera::read(void *buff, size_t buff_size, bool block, int block_ms)
    {
        (void)block_ms;
        if (!this->is_opened()) {
            err::Err e = open(_width, _height, _format, _buff_num);
            err::check_raise(e, "open camera failed");
        }

        if (_show_colorbar) {
            image::Image *img = new image::Image(_width, _height);
            generate_colorbar(*img);
            err::check_null_raise(img, "camera read failed");
            return img;
        } else {
            // it's better all done by impl to faster read, but if impl not support, we have to convert it
            if(_format_impl == _format)
            {
                image::Image *img = _impl->read(buff, buff_size);
                err::check_null_raise(img, "camera read failed");

                // FIXME: delete me and fix driver bug
                uint64_t wait_us = 1000000 / _fps;
                while (time::ticks_us() - _last_read_us < wait_us) {
                    time::sleep_us(50);
                }
                _last_read_us = time::ticks_us();
                return img;
            }
            else
            {
                image::Image *img = _impl->read();
                image::Image *img2 = img->to_format(_format, buff, buff_size);
                delete img;
                err::check_null_raise(img2, "camera read failed");

                // FIXME: delete me and fix driver bug
                uint64_t wait_us = 1000000 / _fps;
                while (time::ticks_us() - _last_read_us < wait_us) {
                    time::sleep_us(50);
                }
                _last_read_us = time::ticks_us();
                return img2;
            }
        }
    }

    void Camera::clear_buff()
    {

    }

    void Camera::skip_frames(int num)
    {
        for(int i = 0; i < num; i++)
        {
            image::Image *img = read();
            delete img;
        }
    }

    err::Err Camera::set_resolution(int width, int height)
    {
        return err::ERR_NOT_IMPL;
    }

    int Camera::exposure(int value) {
        return -1;
    }

    int Camera::gain(int value) {
        return -1;
    }

    int Camera::hmirror(int value) {
        return -1;
    }

    int Camera::vflip(int value) {
        return -1;
    }

    int Camera::luma(int value) {
        return -1;
    }

    int Camera::constrast(int value) {
        return -1;
    }

    int Camera::saturation(int value) {
        return -1;
    }

    int Camera::awb_mode(int value) {
        return -1;
    }

    int Camera::set_awb(int value) {
        return -1;
    }

    int Camera::exp_mode(int value) {
        return -1;
    }

    err::Err Camera::set_windowing(std::vector<int> roi) {
        return err::ERR_NOT_IMPL;
    }


    std::vector<int> Camera::get_sensor_size() {
        return {0, 0};
    }

    err::Err Camera::write_reg(int addr, int data, int bit_width)
    {
        (void)addr;
        (void)data;
        (void)bit_width;
        return err::ERR_NOT_IMPL;
    }

    int Camera::read_reg(int addr, int bit_width)
    {
        (void)addr;
        (void)bit_width;
        return -1;
    }
}

