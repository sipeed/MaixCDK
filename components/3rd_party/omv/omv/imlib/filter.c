/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Image filtering functions.
 */
#include "fsort.h"
#include "imlib.h"

void imlib_histeq(image_t *img, image_t *mask) {
    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            int a = img->w * img->h;
            float s = (COLOR_BINARY_MAX - COLOR_BINARY_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_BINARY_MAX - COLOR_BINARY_MIN + 1) * sizeof(uint32_t), FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    hist[IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x) - COLOR_BINARY_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_BINARY_MAX - COLOR_BINARY_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        continue;
                    }
                    int pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, x,
                                                fast_floorf((s * hist[pixel - COLOR_BINARY_MIN]) + COLOR_BINARY_MIN));
                }
            }

            if (hist) fb_free(hist);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            int a = img->w * img->h;
            float s = (COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1) * sizeof(uint32_t), FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    hist[IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x) - COLOR_GRAYSCALE_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        continue;
                    }
                    int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, x,
                                                   fast_floorf((s * hist[pixel - COLOR_GRAYSCALE_MIN]) + COLOR_GRAYSCALE_MIN));
                }
            }

            if (hist) fb_free(hist);
            break;
        }
        case PIXFORMAT_RGB565: {
            int a = img->w * img->h;
            float s = (COLOR_Y_MAX - COLOR_Y_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_Y_MAX - COLOR_Y_MIN + 1) * sizeof(uint32_t), FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    hist[COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x)) - COLOR_Y_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_Y_MAX - COLOR_Y_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        continue;
                    }
                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    int r = COLOR_RGB565_TO_R8(pixel);
                    int g = COLOR_RGB565_TO_G8(pixel);
                    int b = COLOR_RGB565_TO_B8(pixel);
                    uint8_t y, u, v;
                    y = (uint8_t) (((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    u = (uint8_t) (((b << 14) - (r * 5529) - (g * 10855)) >> 15);  // -0.168736*r + -0.331264*g + 0.5*b
                    v = (uint8_t) (((r << 14) - (g * 13682) - (b * 2664)) >> 15);  // 0.5*r + -0.418688*g + -0.081312*b
                    IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, x, imlib_yuv_to_rgb(fast_floorf(s * hist[y]), u, v));
                }
            }

            if (hist) fb_free(hist);
            break;
        }
        case PIXFORMAT_RGB888: {
            int a = img->w * img->h;
            float s = (COLOR_Y_MAX - COLOR_Y_MIN) / ((float) a);
            uint32_t *hist = fb_alloc0((COLOR_Y_MAX - COLOR_Y_MIN + 1) * sizeof(uint32_t), FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    pixel_rgb_t value = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                    hist[COLOR_RGB888_TO_Y(value.r, value.g, value.b) - COLOR_Y_MIN] += 1;
                }
            }

            for (int i = 0, sum = 0, ii = COLOR_Y_MAX - COLOR_Y_MIN + 1; i < ii; i++) {
                sum += hist[i];
                hist[i] = sum;
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) continue;
                    pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                    int r = COLOR_RGB888_TO_R8(pixel);
                    int g = COLOR_RGB888_TO_G8(pixel);
                    int b = COLOR_RGB888_TO_B8(pixel);
                    uint8_t y, u, v;
                    y = (uint8_t)(((r * 9770) + (g * 19182) + (b * 3736)) >> 15); // .299*r + .587*g + .114*b
                    u = (uint8_t)(((b << 14) - (r * 5529) - (g * 10855)) >> 15);  // -0.168736*r + -0.331264*g + 0.5*b
                    v = (uint8_t)(((r << 14) - (g * 13682) - (b * 2664)) >> 15);  // 0.5*r + -0.418688*g + -0.081312*b
                    IMAGE_PUT_RGB888_PIXEL_FAST(row_ptr, x, imlib_yuv_to_rgb888(fast_floorf(s * hist[y]), u,v));
                }
            }

            fb_free(hist);
            break;
        }
        default: {
            break;
        }
    }
}

// ksize == 0 -> 1x1 kernel
// ksize == 1 -> 3x3 kernel
// ...
// ksize == n -> ((n*2)+1)x((n*2)+1) kernel
//
// To speed up this filter, we can help in two ways:
// 1) For the 'center portion' of the image area, we don't need to check
//   the x+y values against the boundary conditions on each pixel.
// 2) In that same region we can take advantage of the filter property being
//   the sum of all of the pixels by subtracting the last left edge values
//   and adding the new right edge values instead of re-calculating the sum
//   of every pixel. This will allow very large filters to be used without
//   much change in performance.
//
#ifdef IMLIB_ENABLE_MEAN
void imlib_mean_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask) {
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.pixfmt = img->pixfmt;

    int32_t over32_n = 65536 / (((ksize * 2) + 1) * ((ksize * 2) + 1));

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                int pixel, acc = 0;
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y + j);
                            acc -= IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            acc += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x + ksize);
                        }
                    } else {
                        acc = 0;

                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                                                   IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }

                    pixel = (int) ((acc * over32_n) >> 16);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                int pixel, acc = 0;
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }
                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + j);
                            acc -= IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            acc += IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x + ksize);
                        }
                    } else {
                        acc = 0;
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                                                                       IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                acc += IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                                                      IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }

                    pixel = (int) ((acc * over32_n) >> 16);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB565: {
            int pixel, r, g, b, r_acc, g_acc, b_acc;
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                r_acc = g_acc = b_acc = 0;
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }
                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + j);
                            // subtract last left-most pixel from the sums
                            pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            r_acc -= COLOR_RGB565_TO_R5(pixel);
                            g_acc -= COLOR_RGB565_TO_G6(pixel);
                            b_acc -= COLOR_RGB565_TO_B5(pixel);
                            // add new right edge pixel to the sums
                            pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x + ksize);
                            r_acc += COLOR_RGB565_TO_R5(pixel);
                            g_acc += COLOR_RGB565_TO_G6(pixel);
                            b_acc += COLOR_RGB565_TO_B5(pixel);
                        }
                    } else {
                        // check bounds and do full sum calculations
                        r_acc = g_acc = b_acc = 0;
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                r_acc += COLOR_RGB565_TO_R5(pixel);
                                g_acc += COLOR_RGB565_TO_G6(pixel);
                                b_acc += COLOR_RGB565_TO_B5(pixel);
                            }
                        }
                    }
                    int pixel;
                    r = (int) ((r_acc * over32_n) >> 16);
                    g = (int) ((g_acc * over32_n) >> 16);
                    b = (int) ((b_acc * over32_n) >> 16);
                    pixel = COLOR_R5_G6_B5_TO_RGB565(r, g, b);

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) <
                             COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB888: {
            int r, g, b, r_acc, g_acc, b_acc;
            pixel_rgb_t pixel;
            buf.data = fb_alloc(IMAGE_RGB888_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                pixel_rgb_t *buf_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows));

                r_acc = g_acc = b_acc = 0;
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }
                    if (!mask && x > ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) {
                       for (int j= -ksize; j<=ksize; j++) {
                           pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y+j);
                           // subtract last left-most pixel from the sums
                           pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x-ksize-1);
                           r_acc -= COLOR_RGB888_TO_R8(pixel);
                           g_acc -= COLOR_RGB888_TO_G8(pixel);
                           b_acc -= COLOR_RGB888_TO_B8(pixel);
                           // add new right edge pixel to the sums
                           pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x+ksize);
                           r_acc += COLOR_RGB888_TO_R8(pixel);
                           g_acc += COLOR_RGB888_TO_G8(pixel);
                           b_acc += COLOR_RGB888_TO_B8(pixel);
                        }
                    } else { // check bounds and do full sum calculations
                        r_acc = g_acc = b_acc = 0;
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,
                            IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,
                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                r_acc += COLOR_RGB888_TO_R8(pixel);
                                g_acc += COLOR_RGB888_TO_G8(pixel);
                                b_acc += COLOR_RGB888_TO_B8(pixel);
                            }
                        }
                    }
                    pixel_rgb_t pixel;
                    r = (int)((r_acc * over32_n)>>16);
                    g = (int)((g_acc * over32_n)>>16);
                    b = (int)((b_acc * over32_n)>>16);
                    pixel = COLOR_R8_G8_B8_TO_RGB888(r, g, b);

                    if (threshold) {
                        pixel_rgb_t temp = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                        if (((COLOR_RGB888_TO_Y(pixel.r, pixel.g, pixel.b) - offset) < COLOR_RGB888_TO_Y(temp.r, temp.g, temp.b)) ^ invert) {
                            pixel.r = COLOR_R8_MAX;
                            pixel.g = COLOR_G8_MAX;
                            pixel.b = COLOR_B8_MAX;
                        } else {
                            pixel.r = COLOR_R8_MIN;
                            pixel.g = COLOR_G8_MIN;
                            pixel.b = COLOR_B8_MIN;
                        }
                    }

                    IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB888_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB888_LINE_LEN_BYTES(img));
            }

            fb_free(buf.data);
            break;
        }
        default: {
            break;
        }
    }
}
#endif // IMLIB_ENABLE_MEAN

#ifdef IMLIB_ENABLE_MEDIAN
static uint8_t hist_median(uint8_t *data, int len, const int cutoff) {
    int i;
#if defined(ARM_MATH_CM7) || defined(ARM_MATH_CM4)
    uint32_t oldsum = 0, sum32 = 0;

    for (i = 0; i < len; i += 4) {
        // work 4 at time with SIMD
        sum32 = __USADA8(*(uint32_t *) &data[i], 0, sum32);
        if (sum32 >= cutoff) {
            // within this group
            while (oldsum < cutoff && i < len) {
                oldsum += data[i++];
            }
            break;
        } // if we're at the last 4 values
        oldsum = sum32;
    } // for each group of 4 elements
#else // generic C version
    int sum = 0;
    for (i = 0; i < len && sum < cutoff; i++) {
        sum += data[i];
    }
#endif
    return i - 1;
} /* hist_median() */

void imlib_median_filter(image_t *img, const int ksize, float percentile, bool threshold, int offset, bool invert,
                         image_t *mask) {
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.pixfmt = img->pixfmt;

    const int n = ((ksize * 2) + 1) * ((ksize * 2) + 1);
    const int median_cutoff = fast_floorf(percentile * (float) n);

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            int sum = 0;

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }
                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y + j);
                            sum -= IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            sum += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x + ksize);
                        }
                    } else {
                        sum = 0;
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                sum += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                                                   IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }
                    int pixel = (sum >= median_cutoff);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            uint8_t *data = fb_alloc(64, FB_ALLOC_NO_HINT);
            uint8_t pixel;
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        // update histogram edges
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + j);
                            pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            data[pixel >> 2]--; // remove old pixels
                            pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x + ksize);
                            data[pixel >> 2]++; // add new pixels
                        } // for j
                    } else {
                        // slow way
                        memset(data, 0, 64);
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                                                                       IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                                                       IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                data[pixel >> 2]++;
                            }
                        }
                    }

                    pixel = hist_median(data, 64, median_cutoff); // find the median
                    pixel <<= 2; // scale it back up
                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            if (data) fb_free(data);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            uint8_t *r_data = fb_alloc(32, FB_ALLOC_NO_HINT);
            uint8_t *g_data = fb_alloc(64, FB_ALLOC_NO_HINT);
            uint8_t *b_data = fb_alloc(32, FB_ALLOC_NO_HINT);
            uint8_t r, g, b;
            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }


                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + j);
                            int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            r_data[COLOR_RGB565_TO_R5(pixel)]--; // remove left pixel
                            g_data[COLOR_RGB565_TO_G6(pixel)]--;
                            b_data[COLOR_RGB565_TO_B5(pixel)]--;
                            pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x + ksize);
                            r_data[COLOR_RGB565_TO_R5(pixel)]++; // add right pixel
                            g_data[COLOR_RGB565_TO_G6(pixel)]++;
                            b_data[COLOR_RGB565_TO_B5(pixel)]++;
                        }
                    } else {
                        // need to check bounds
                        memset(r_data, 0, 32); memset(g_data, 0, 64); memset(b_data, 0, 32);
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                                                        IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                r_data[COLOR_RGB565_TO_R5(pixel)]++;
                                g_data[COLOR_RGB565_TO_G6(pixel)]++;
                                b_data[COLOR_RGB565_TO_B5(pixel)]++;
                            }
                        }
                    }

                    r = hist_median(r_data, 32, median_cutoff);
                    g = hist_median(g_data, 64, median_cutoff);
                    b = hist_median(b_data, 32, median_cutoff);

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(r, g, b);

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) <
                             COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            if (b_data) fb_free(b_data);
            if (g_data) fb_free(g_data);
            if (r_data) fb_free(r_data);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB888: {
            buf.data = fb_alloc(IMAGE_RGB888_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            uint8_t *r_data = fb_alloc(256, FB_ALLOC_NO_HINT);
            uint8_t *g_data = fb_alloc(256, FB_ALLOC_NO_HINT);
            uint8_t *b_data = fb_alloc(256, FB_ALLOC_NO_HINT);
            uint8_t r, g, b;
            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                pixel_rgb_t *buf_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }


                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y + j);
                            pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            r_data[COLOR_RGB888_TO_R8(pixel)]--; // remove left pixel
                            g_data[COLOR_RGB888_TO_G8(pixel)]--;
                            b_data[COLOR_RGB888_TO_B8(pixel)]--;
                            pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x+ksize);
                            r_data[COLOR_RGB888_TO_R8(pixel)]++; // add right pixel
                            g_data[COLOR_RGB888_TO_G8(pixel)]++;
                            b_data[COLOR_RGB888_TO_B8(pixel)]++;
                        }
                    } else { // need to check bounds
                        memset(r_data, 0, 256); memset(g_data, 0, 256); memset(b_data, 0, 256);
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,
                                                                                        IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,
                                                                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                r_data[COLOR_RGB888_TO_R8(pixel)]++;
                                g_data[COLOR_RGB888_TO_G8(pixel)]++;
                                b_data[COLOR_RGB888_TO_B8(pixel)]++;
                            }
                        }
                    }

                    r = hist_median(r_data, 256, median_cutoff);
                    g = hist_median(g_data, 256, median_cutoff);
                    b = hist_median(b_data, 256, median_cutoff);

                    pixel_rgb_t pixel = COLOR_R8_G8_B8_TO_RGB888(r, g, b);

                    if (threshold) {
                        pixel_rgb_t temp = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                        if (((COLOR_RGB888_TO_Y(pixel.r, pixel.g, pixel.b) - offset) < COLOR_RGB888_TO_Y(temp.r, temp.g, temp.b)) ^ invert) {
                            pixel.r = COLOR_R8_MAX;
                            pixel.g = COLOR_G8_MAX;
                            pixel.b = COLOR_B8_MAX;
                        } else {
                            pixel.r = COLOR_R8_MIN;
                            pixel.g = COLOR_G8_MIN;
                            pixel.b = COLOR_B8_MIN;
                        }
                    }

                    IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB888_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB888_LINE_LEN_BYTES(img));
            }

            if (b_data) fb_free(b_data);
            if (g_data) fb_free(g_data);
            if (r_data) fb_free(r_data);
            if (buf.data) fb_free(buf.data);
            break;
        }
        default: {
            break;
        }
    }
}
#endif // IMLIB_ENABLE_MEDIAN

#ifdef IMLIB_ENABLE_MODE
static uint8_t find_mode(uint8_t *bins, int len) {
    int i, j;
    uint8_t mode = 0, mcount = 0;
    for (i = 0; i < len; i += 4) {
        if (*(uint32_t *) &bins[i] == 0) {
            continue; // skip empty bins quickly
        }
        for (j = i; j < i + 4; j++) {
            if (bins[j] > mcount) {
                mcount = bins[j];
                mode = j;
            }
        }
    }
    return mode;
} /* find_mode() */

void imlib_mode_filter(image_t *img, const int ksize, bool threshold, int offset, bool invert, image_t *mask) {
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.pixfmt = img->pixfmt;
    const uint8_t n2 = (((ksize * 2) + 1) * ((ksize * 2) + 1)) / 2;
    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            int bins = 0;

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }
                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y + j);
                            bins -= IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            bins += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x + ksize);
                        }
                    } else {
                        bins = 0;
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));
                            for (int k = -ksize; k <= ksize; k++) {
                                bins += IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            }
                        }
                    }

                    uint8_t pixel = (bins > n2);

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            uint8_t *bins = fb_alloc((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1), FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));
                uint8_t pixel = 0, mode = 0;
                int mcount = -1;
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }
                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t m, *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + j);
                            pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            m = --bins[pixel];
                            if (pixel == mode) {
                                if (m < n2) {
                                    mcount = 256; // need to search later
                                } else {
                                    mcount = m; // we're still the mode
                                }
                            }
                            pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x + ksize);
                            m = ++bins[pixel];
                            if (m > mcount) {
                                mcount = m;
                                mode = pixel;
                            }
                        }
                        if (mcount == 256) {
                            // need to find max
                            mode = find_mode(bins, 256);
                            mcount = bins[mode];
                        }
                    } else {
                        // slow way
                        mcount = -1;
                        memset(bins, 0, (COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1));
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                                                                       IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                                                       IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                bins[pixel]++;

                                if (bins[pixel] > mcount) {
                                    mcount = bins[pixel];
                                    mode = pixel;
                                }
                            }
                        }
                    }
                    pixel = mode;

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            if (bins) fb_free(bins);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            uint8_t *r_bins = fb_alloc((COLOR_R5_MAX - COLOR_R5_MIN + 1), FB_ALLOC_NO_HINT);
            uint8_t *g_bins = fb_alloc((COLOR_G6_MAX - COLOR_G6_MIN + 1), FB_ALLOC_NO_HINT);
            uint8_t *b_bins = fb_alloc((COLOR_B5_MAX - COLOR_B5_MIN + 1), FB_ALLOC_NO_HINT);
            int r_pixel, g_pixel, b_pixel;

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));
                int r_mcount = 0, g_mcount = 0, b_mcount = 0;
                int pixel, r_mode, g_mode, b_mode;
                r_mode = g_mode = b_mode = 0;
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + j);
                            pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x - ksize - 1);
                            r_pixel = COLOR_RGB565_TO_R5(pixel);
                            g_pixel = COLOR_RGB565_TO_G6(pixel);
                            b_pixel = COLOR_RGB565_TO_B5(pixel);
                            r_bins[r_pixel]--;
                            g_bins[g_pixel]--;
                            b_bins[b_pixel]--;
                            if (r_pixel == r_mode) {
                                if (r_bins[r_pixel] < n2) {
                                    r_mcount = 256; // need to search later
                                } else {
                                    r_mcount = r_bins[r_pixel]; // we're still the mode
                                }
                            }
                            if (g_pixel == g_mode) {
                                if (g_bins[g_pixel] < n2) {
                                    g_mcount = 256; // need to search later
                                } else {
                                    g_mcount = g_bins[g_pixel]; // we're still the mode
                                }
                            }
                            if (b_pixel == b_mode) {
                                if (b_bins[b_pixel] < n2) {
                                    b_mcount = 256; // need to search later
                                } else {
                                    b_mcount = b_bins[b_pixel]; // we're still the mode
                                }
                            }

                            pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x + ksize);
                            r_pixel = COLOR_RGB565_TO_R5(pixel);
                            g_pixel = COLOR_RGB565_TO_G6(pixel);
                            b_pixel = COLOR_RGB565_TO_B5(pixel);
                            r_bins[r_pixel]++;
                            g_bins[g_pixel]++;
                            b_bins[b_pixel]++;
                            if (r_bins[r_pixel] > r_mcount) {
                                r_mcount = r_bins[r_pixel];
                                r_mode = r_pixel;
                            }

                            if (g_bins[g_pixel] > g_mcount) {
                                g_mcount = g_bins[g_pixel];
                                g_mode = g_pixel;
                            }

                            if (b_bins[b_pixel] > b_mcount) {
                                b_mcount = b_bins[b_pixel];
                                b_mode = b_pixel;
                            }
                        } // for j
                        if (r_mcount == 256) {
                            // need to find max
                            r_mode = find_mode(r_bins, 32);
                            r_mcount = r_bins[r_mode];
                        }
                        if (g_mcount == 256) {
                            // need to find max
                            g_mode = find_mode(g_bins, 64);
                            g_mcount = g_bins[g_mode];
                        }
                        if (b_mcount == 256) {
                            // need to find max
                            b_mode = find_mode(b_bins, 32);
                            b_mcount = r_bins[b_mode];
                        }
                    } else {
                        // slower way
                        memset(r_bins, 0, (COLOR_R5_MAX - COLOR_R5_MIN + 1));
                        memset(g_bins, 0, (COLOR_G6_MAX - COLOR_G6_MIN + 1));
                        memset(b_bins, 0, (COLOR_B5_MAX - COLOR_B5_MIN + 1));
                        r_mcount = g_mcount = b_mcount = 0;
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                r_pixel = COLOR_RGB565_TO_R5(pixel);
                                g_pixel = COLOR_RGB565_TO_G6(pixel);
                                b_pixel = COLOR_RGB565_TO_B5(pixel);
                                r_bins[r_pixel]++;
                                g_bins[g_pixel]++;
                                b_bins[b_pixel]++;

                                if (r_bins[r_pixel] > r_mcount) {
                                    r_mcount = r_bins[r_pixel];
                                    r_mode = r_pixel;
                                }

                                if (g_bins[g_pixel] > g_mcount) {
                                    g_mcount = g_bins[g_pixel];
                                    g_mode = g_pixel;
                                }

                                if (b_bins[b_pixel] > b_mcount) {
                                    b_mcount = b_bins[b_pixel];
                                    b_mode = b_pixel;
                                }
                            } // for k
                        } // for j
                    } // slow/fast way
                    pixel = COLOR_R5_G6_B5_TO_RGB565(r_mode, g_mode, b_mode);
                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            if (r_bins) fb_free(r_bins);
            if (g_bins) fb_free(g_bins);
            if (b_bins) fb_free(b_bins);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB888: {
            buf.data = fb_alloc(IMAGE_RGB888_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            uint8_t *r_bins = fb_alloc(256, FB_ALLOC_NO_HINT);
            uint8_t *g_bins = fb_alloc(256, FB_ALLOC_NO_HINT);
            uint8_t *b_bins = fb_alloc(256, FB_ALLOC_NO_HINT);
            int r_pixel, g_pixel, b_pixel;

            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                pixel_rgb_t *buf_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows));
                int r_mcount=0, g_mcount=0, b_mcount=0;
                int r_mode, g_mode, b_mode;
                pixel_rgb_t pixel;
                r_mode = g_mode = b_mode = 0;
                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                if (!mask && x > ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                    for (int j = -ksize; j <= ksize; j++) {
                        pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y + j);
                        pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x - ksize -1);
                        r_pixel = COLOR_RGB888_TO_R8(pixel);
                        g_pixel = COLOR_RGB888_TO_G8(pixel);
                        b_pixel = COLOR_RGB888_TO_B8(pixel);
                        r_bins[r_pixel]--;
                        g_bins[g_pixel]--;
                        b_bins[b_pixel]--;
                        if (r_pixel == r_mode) {
                            if (r_bins[r_pixel] < n2)
                                r_mcount = 256; // need to search later
                            else
                                r_mcount = r_bins[r_pixel]; // we're still the mode
                        }
                        if (g_pixel == g_mode) {
                            if (g_bins[g_pixel] < n2)
                                g_mcount = 256; // need to search later
                            else
                                g_mcount = g_bins[g_pixel]; // we're still the mode
                        }
                        if (b_pixel == b_mode) {
                            if (b_bins[b_pixel] < n2)
                                b_mcount = 256; // need to search later
                            else
                                b_mcount = b_bins[b_pixel]; // we're still the mode
                        }

                        pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x + ksize);
                        r_pixel = COLOR_RGB888_TO_R8(pixel);
                        g_pixel = COLOR_RGB888_TO_G8(pixel);
                        b_pixel = COLOR_RGB888_TO_B8(pixel);
                        r_bins[r_pixel]++;
                        g_bins[g_pixel]++;
                        b_bins[b_pixel]++;
                        if (r_bins[r_pixel] > r_mcount) {
                            r_mcount = r_bins[r_pixel];
                            r_mode = r_pixel;
                        }

                        if (g_bins[g_pixel] > g_mcount) {
                            g_mcount = g_bins[g_pixel];
                            g_mode = g_pixel;
                        }

                        if (b_bins[b_pixel] > b_mcount) {
                            b_mcount = b_bins[b_pixel];
                            b_mode = b_pixel;
                        }
                    } // for j
                    if (r_mcount == 256) { // need to find max
                        r_mode = find_mode(r_bins, 256);
                        r_mcount = r_bins[r_mode];
                    }
                    if (g_mcount == 256) { // need to find max
                        g_mode = find_mode(g_bins, 256);
                        g_mcount = g_bins[g_mode];
                    }
                    if (b_mcount == 256) { // need to find max
                        b_mode = find_mode(b_bins, 256);
                        b_mcount = r_bins[b_mode];
                    }
                } else { // slower way
                    memset(r_bins, 0, 256);
                    memset(g_bins, 0, 256);
                    memset(b_bins, 0, 256);
                    r_mcount = g_mcount = b_mcount = 0;
                    for (int j = -ksize; j <= ksize; j++) {
                        pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,
                                                                                    IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,
                                                                IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            r_pixel = COLOR_RGB888_TO_R8(pixel);
                            g_pixel = COLOR_RGB888_TO_G8(pixel);
                            b_pixel = COLOR_RGB888_TO_B8(pixel);
                            r_bins[r_pixel]++;
                            g_bins[g_pixel]++;
                            b_bins[b_pixel]++;

                            if (r_bins[r_pixel] > r_mcount) {
                                r_mcount = r_bins[r_pixel];
                                r_mode = r_pixel;
                            }

                            if (g_bins[g_pixel] > g_mcount) {
                                g_mcount = g_bins[g_pixel];
                                g_mode = g_pixel;
                            }

                            if (b_bins[b_pixel] > b_mcount) {
                                b_mcount = b_bins[b_pixel];
                                b_mode = b_pixel;
                            }
                        } // for k
                    } // for j
                    } // slow/fast way
                    pixel = COLOR_R8_G8_B8_TO_RGB888(r_mode, g_mode, b_mode);

                    if (threshold) {
                        pixel_rgb_t temp = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                        if (((COLOR_RGB888_TO_Y(pixel.r, pixel.g, pixel.b) - offset) < COLOR_RGB888_TO_Y(temp.r, temp.g, temp.b)) ^ invert) {
                            pixel.r = COLOR_R8_MAX;
                            pixel.g = COLOR_G8_MAX;
                            pixel.b = COLOR_B8_MAX;
                        } else {
                            pixel.r = COLOR_R8_MIN;
                            pixel.g = COLOR_G8_MIN;
                            pixel.b = COLOR_B8_MIN;
                        }
                    }

                    IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB888_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB888_LINE_LEN_BYTES(img));
            }

            if (b_bins) fb_free(b_bins);      // b_bins
            if (g_bins) fb_free(g_bins);      // g_bins
            if (r_bins) fb_free(r_bins);      // r_bins
            if (buf.data) fb_free(buf.data);  // buf.data
            break;
        }
        default: {
            break;
        }
    }
}
#endif // IMLIB_ENABLE_MODE

#ifdef IMLIB_ENABLE_MIDPOINT
void imlib_midpoint_filter(image_t *img, const int ksize, float bias, bool threshold, int offset, bool invert, image_t *mask) {
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.pixfmt = img->pixfmt;
    uint8_t *u8BiasTable;
    float max_bias = bias, min_bias = 1.0f - bias;

    u8BiasTable = fb_alloc(256, FB_ALLOC_NO_HINT);
    for (int i = 0; i < 256; i++) {
        u8BiasTable[i] = (uint8_t) fast_floorf((float) i * bias);
    }

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int min = COLOR_BINARY_MAX, max = COLOR_BINARY_MIN;

                    if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y + j);
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x + k);
                                min &= pixel;
                                max |= pixel;
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                                                        IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                min &= pixel;
                                max |= pixel;
                            }
                        }
                    }

                    int pixel = fast_floorf((min * min_bias) + (max * max_bias));

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int min = COLOR_GRAYSCALE_MAX, max = COLOR_GRAYSCALE_MIN;
                    if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + j);
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x + k);
                                if (pixel < min) {
                                    min = pixel;
                                } else if (pixel > max) {
                                    max = pixel;
                                }
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                                                                       IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                                                           IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                if (pixel < min) {
                                    min = pixel;
                                } else if (pixel > max) {
                                    max = pixel;
                                }
                            }
                        }
                    }

                    int pixel = min + u8BiasTable[max - min];

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int r_min = COLOR_R5_MAX, r_max = COLOR_R5_MIN;
                    int g_min = COLOR_G6_MAX, g_max = COLOR_G6_MIN;
                    int b_min = COLOR_B5_MAX, b_max = COLOR_B5_MIN;
                    if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + j);
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x + k);
                                int r_pixel = COLOR_RGB565_TO_R5(pixel);
                                int g_pixel = COLOR_RGB565_TO_G6(pixel);
                                int b_pixel = COLOR_RGB565_TO_B5(pixel);
                                if (r_pixel < r_min) {
                                    r_min = r_pixel;
                                } else if (r_pixel > r_max) {
                                    r_max = r_pixel;
                                }
                                if (g_pixel < g_min) {
                                    g_min = g_pixel;
                                } else if (g_pixel > g_max) {
                                    g_max = g_pixel;
                                }
                                if (b_pixel < b_min) {
                                    b_min = b_pixel;
                                } else if (b_pixel > b_max) {
                                    b_max = b_pixel;
                                }
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                                                        IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                int r_pixel = COLOR_RGB565_TO_R5(pixel);
                                int g_pixel = COLOR_RGB565_TO_G6(pixel);
                                int b_pixel = COLOR_RGB565_TO_B5(pixel);
                                if (r_pixel < r_min) {
                                    r_min = r_pixel;
                                } else if (r_pixel > r_max) {
                                    r_max = r_pixel;
                                }
                                if (g_pixel < g_min) {
                                    g_min = g_pixel;
                                } else if (g_pixel > g_max) {
                                    g_max = g_pixel;
                                }
                                if (b_pixel < b_min) {
                                    b_min = b_pixel;
                                } else if (b_pixel > b_max) {
                                    b_max = b_pixel;
                                }
                            }
                        }
                    }

                    r_min += u8BiasTable[r_max - r_min];
                    g_min += u8BiasTable[g_max - g_min];
                    b_min += u8BiasTable[b_max - b_min];
                    int pixel = COLOR_R5_G6_B5_TO_RGB565(r_min, g_min, b_min);

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) <
                             COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB888: {
            buf.data = fb_alloc(IMAGE_RGB888_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                pixel_rgb_t *buf_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int r_min = COLOR_R8_MAX, r_max = COLOR_R8_MIN;
                    int g_min = COLOR_G8_MAX, g_max = COLOR_G8_MIN;
                    int b_min = COLOR_B8_MAX, b_max = COLOR_B8_MIN;
                    if (x >= ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,y+j);
                            for (int k = -ksize; k <= ksize; k++) {
                                pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,x+k);
                                int r_pixel = COLOR_RGB888_TO_R8(pixel);
                                int g_pixel = COLOR_RGB888_TO_G8(pixel);
                                int b_pixel = COLOR_RGB888_TO_B8(pixel);
                                if (r_pixel < r_min) r_min = r_pixel;
                                else if (r_pixel > r_max) r_max = r_pixel;
                                if (g_pixel < g_min) g_min = g_pixel;
                                else if (g_pixel > g_max) g_max = g_pixel;
                                if (b_pixel < b_min) b_min = b_pixel;
                                else if (b_pixel > b_max) b_max = b_pixel;
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,
                                IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,
                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                int r_pixel = COLOR_RGB888_TO_R8(pixel);
                                int g_pixel = COLOR_RGB888_TO_G8(pixel);
                                int b_pixel = COLOR_RGB888_TO_B8(pixel);
                                if (r_pixel < r_min) r_min = r_pixel;
                                else if (r_pixel > r_max) r_max = r_pixel;
                                if (g_pixel < g_min) g_min = g_pixel;
                                else if (g_pixel > g_max) g_max = g_pixel;
                                if (b_pixel < b_min) b_min = b_pixel;
                                else if (b_pixel > b_max) b_max = b_pixel;
                            }
                        }
                    }

                    r_min += u8BiasTable[r_max-r_min];
                    g_min += u8BiasTable[g_max-g_min];
                    b_min += u8BiasTable[b_max-b_min];
                    pixel_rgb_t pixel = COLOR_R8_G8_B8_TO_RGB888(r_min, g_min, b_min);

                    if (threshold) {
                        pixel_rgb_t temp = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                        if (((COLOR_RGB888_TO_Y(pixel.r, pixel.g, pixel.b) - offset) < COLOR_RGB888_TO_Y(temp.r, temp.g, temp.b)) ^ invert) {
                            pixel.r = COLOR_R8_MAX;
                            pixel.g = COLOR_G8_MAX;
                            pixel.b = COLOR_B8_MAX;
                        } else {
                            pixel.r = COLOR_R8_MIN;
                            pixel.g = COLOR_G8_MIN;
                            pixel.b = COLOR_B8_MIN;
                        }
                    }

                    IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB888_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB888_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        default: {
            break;
        }
    }
    if (u8BiasTable) fb_free(u8BiasTable);
}
#endif // IMLIB_ENABLE_MIDPOINT

// http://www.fmwconcepts.com/imagemagick/digital_image_filtering.pdf

void imlib_morph(image_t *img,
                 const int ksize,
                 const int *krn,
                 const float m,
                 const int b,
                 bool threshold,
                 int offset,
                 bool invert,
                 image_t *mask) {
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.pixfmt = img->pixfmt;
    const int32_t m_int = fast_roundf(65536 * m);
    const int32_t b_int = b << 16;
    invert = invert ? 1 : 0; // ensure binary

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            for (int y = 0; y < img->h; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0; x < img->w; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        int p = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, p);
                        continue; // Short circuit.
                    }

                    int32_t acc = 0, ptr = 0;

                    if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y + j);
                            for (int k = -ksize; k <= ksize; k++) {
                                acc += krn[ptr++] * IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x + k);
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            int y_j = IM_MIN(IM_MAX(y + j, 0), (img->h - 1));
                            uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y_j);

                            for (int k = -ksize; k <= ksize; k++) {
                                int x_k = IM_MIN(IM_MAX(x + k, 0), (img->w - 1));
                                acc += krn[ptr++] * IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr, x_k);
                            }
                        }
                    }

                    int32_t tmp = (acc * m_int) + b_int;
                    int pixel = __USAT_ASR(tmp, 1, 16);

                    if (threshold) {
                        pixel -= offset;
                        pixel = pixel < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                        pixel = pixel ^ invert;
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0); y < img->h; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            #if defined(ARM_MATH_DSP)
            int32_t krn_4, krn_2_0, krn_5_3, krn_8_6, krn_7_1, offset_int, invert_ge, invert_lt;
            if (ksize == 1) {
                krn_4 = krn[4];
                krn_2_0 = __PKHBT(krn[0], krn[2], 16);
                krn_5_3 = __PKHBT(krn[3], krn[5], 16);
                krn_8_6 = __PKHBT(krn[6], krn[8], 16);
                krn_7_1 = __PKHBT(krn[1], krn[7], 16);
                offset_int = __PKHBT(offset, offset, 16);
                invert_ge = invert ? 0x00FF00FF : 0xFF00FF00;
                invert_lt = invert ? 0xFF00FF00 : 0x00FF00FF;
            }
            #endif

            for (int y = 0; y < img->h; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                if (0) {
                #if defined(ARM_MATH_DSP)
                } else if ((ksize == 1) && (!mask)) {
                    uint8_t *row_ptr_m1, *row_ptr_p1;

                    if (y == 0) {
                        row_ptr_m1 = row_ptr;
                        row_ptr_p1 = row_ptr + ((img->h >= 2) ? img->w : 0);
                    } else if (y >= (img->h - 1)) {
                        row_ptr_m1 = row_ptr - img->w;
                        row_ptr_p1 = row_ptr;
                    } else {
                        // get 2 neighboring rows
                        row_ptr_m1 = row_ptr - img->w;
                        row_ptr_p1 = row_ptr + img->w;
                    }

                    // If the image is an odd width this will go for the last loop and we drop the last column.
                    for (int x = 0; x < img->w; x += 2) {
                        uint32_t row_0, row_1, row_2;

                        if (x == 0) {
                            if (img->w >= 3) {
                                row_0 = *((uint16_t *) row_ptr_m1) | (*(row_ptr_m1 + 2) << 16);
                                row_1 = *((uint16_t *) row_ptr) | (*(row_ptr + 2) << 16);
                                row_2 = *((uint16_t *) row_ptr_p1) | (*(row_ptr_p1 + 2) << 16);
                            } else if (img->w >= 2) {
                                row_0 = *((uint16_t *) row_ptr_m1);
                                row_0 = __REV(row_0) | row_0;
                                row_1 = *((uint16_t *) row_ptr);
                                row_1 = __REV(row_1) | row_1;
                                row_2 = *((uint16_t *) row_ptr_p1);
                                row_2 = __REV(row_2) | row_2;
                            } else {
                                row_0 = *row_ptr_m1 * 0x010101;
                                row_1 = *row_ptr * 0x010101;
                                row_2 = *row_ptr_p1 * 0x010101;
                            }
                            row_0 = (row_0 << 8) | (row_0 & 0xff);
                            row_1 = (row_1 << 8) | (row_1 & 0xff);
                            row_2 = (row_2 << 8) | (row_2 & 0xff);
                        } else if (x == (img->w - 2)) {
                            row_0 = *((uint32_t *) (row_ptr_m1 + x - 2));
                            row_0 = (row_0 >> 8) | ((row_0 << 8) & 0xff000000);
                            row_1 = *((uint32_t *) (row_ptr + x - 2));
                            row_1 = (row_1 >> 8) | ((row_1 << 8) & 0xff000000);
                            row_2 = *((uint32_t *) (row_ptr_p1 + x - 2));
                            row_2 = (row_2 >> 8) | ((row_2 << 8) & 0xff000000);
                        } else if (x >= (img->w - 1)) {
                            row_0 = *((uint16_t *) (row_ptr_m1 + x - 1));
                            row_0 = ((__UXTB_RORn(row_0, 8) * 0x0101) << 16) | row_0;
                            row_1 = *((uint16_t *) (row_ptr + x - 1));
                            row_1 = ((__UXTB_RORn(row_1, 8) * 0x0101) << 16) | row_1;
                            row_2 = *((uint16_t *) (row_ptr_p1 + x - 1));
                            row_2 = ((__UXTB_RORn(row_2, 8) * 0x0101) << 16) | row_2;
                        } else {
                            // get 3 neighboring rows
                            row_0 = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_1 = *((uint32_t *) (row_ptr + x - 1));
                            row_2 = *((uint32_t *) (row_ptr_p1 + x - 1));
                        }

                        int32_t p0_4 = __UXTB_RORn(row_1, 8);
                        int32_t p0_7_1 = __PKHBT(__UXTB_RORn(row_0, 8), __UXTB_RORn(row_2, 8), 16);
                        int32_t pixel0 = krn_4 * p0_4;
                        pixel0 = __SMLAD(__UXTB16(row_0), krn_2_0, pixel0);
                        pixel0 = __SMLAD(__UXTB16(row_1), krn_5_3, pixel0);
                        pixel0 = __SMLAD(__UXTB16(row_2), krn_8_6, pixel0);
                        pixel0 = __SMLAD(p0_7_1, krn_7_1, pixel0);
                        pixel0 = (pixel0 * m_int) + b_int;
                        pixel0 = __USAT_ASR(pixel0, 8, 16);

                        int32_t p1_4 = __UXTB_RORn(row_1, 16);
                        int32_t p1_7_1 = __PKHBT(__UXTB_RORn(row_0, 16), __UXTB_RORn(row_2, 16), 16);
                        int32_t pixel1 = krn_4 * p1_4;
                        pixel1 = __SMLAD(__UXTB16_RORn(row_0, 8), krn_2_0, pixel1);
                        pixel1 = __SMLAD(__UXTB16_RORn(row_1, 8), krn_5_3, pixel1);
                        pixel1 = __SMLAD(__UXTB16_RORn(row_2, 8), krn_8_6, pixel1);
                        pixel1 = __SMLAD(p1_7_1, krn_7_1, pixel1);
                        pixel1 = (pixel1 * m_int) + b_int;
                        pixel1 = __USAT_ASR(pixel1, 8, 16);

                        // Re-pack to make thresholding faster.
                        int32_t p1_p0 = __PKHBT(pixel0, pixel1, 16);

                        if (threshold) {
                            p1_p0 = __SSUB16(__SSUB16(p1_p0, offset_int), __PKHBT(p0_4, p1_4, 16));
                            p1_p0 = __SEL(invert_ge, invert_lt);
                        }

                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, p1_p0);

                        if (x != (img->w - 1)) {
                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x + 1, p1_p0 >> 16);
                        }
                    }
                #endif
                } else {
                    for (int x = 0; x < img->w; x++) {
                        if (mask && (!image_get_mask_pixel(mask, x, y))) {
                            int p = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                            IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, p);
                            continue; // Short circuit.
                        }

                        int32_t acc = 0, ptr = 0;

                        if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                            for (int j = -ksize; j <= ksize; j++) {
                                uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + j);
                                for (int k = -ksize; k <= ksize; k++) {
                                    acc += krn[ptr++] * IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x + k);
                                }
                            }
                        } else {
                            for (int j = -ksize; j <= ksize; j++) {
                                int y_j = IM_MIN(IM_MAX(y + j, 0), (img->h - 1));
                                uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y_j);

                                for (int k = -ksize; k <= ksize; k++) {
                                    int x_k = IM_MIN(IM_MAX(x + k, 0), (img->w - 1));
                                    acc += krn[ptr++] * IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x_k);
                                }
                            }
                        }

                        int32_t tmp = (acc * m_int) + b_int;
                        int pixel = __USAT_ASR(tmp, 8, 16);

                        if (threshold) {
                            pixel -= offset;
                            pixel = pixel < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                            pixel = (pixel ^ invert) * COLOR_GRAYSCALE_BINARY_MAX;
                        }

                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                    }
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0); y < img->h; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            #if defined(ARM_MATH_DSP)
            int32_t krn_5, krn_1_0, krn_4_3, krn_7_6, krn_8_2, offset_int, invert_ge, invert_lt;
            if (ksize == 1) {
                krn_5 = krn[5];
                krn_1_0 = __PKHBT(krn[0], krn[1], 16);
                krn_4_3 = __PKHBT(krn[3], krn[4], 16);
                krn_7_6 = __PKHBT(krn[6], krn[7], 16);
                krn_8_2 = __PKHBT(krn[2], krn[8], 16);
                offset_int = __PKHBT(offset, offset, 16);
                invert_ge = invert ? 0xFFFFFFFF : 0x00000000;
                invert_lt = invert ? 0x00000000 : 0xFFFFFFFF;
            }
            #endif

            for (int y = 0; y < img->h; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                if (0) {
                #if defined(ARM_MATH_DSP)
                } else if (0 && (ksize == 1) && (!mask)) {
                    uint16_t *row_ptr_m1, *row_ptr_p1;

                    if (y == 0) {
                        row_ptr_m1 = row_ptr;
                        row_ptr_p1 = row_ptr + ((img->h >= 2) ? img->w : 0);
                    } else if (y >= (img->h - 1)) {
                        row_ptr_m1 = row_ptr - img->w;
                        row_ptr_p1 = row_ptr;
                    } else {
                        // get 2 neighboring rows
                        row_ptr_m1 = row_ptr - img->w;
                        row_ptr_p1 = row_ptr + img->w;
                    }

                    // If the image is an odd width this will go for the last loop and we drop the last column.
                    for (int x = 0; x < img->w; x += 2) {
                        uint32_t row_0[2], row_1[2], row_2[2];

                        if (x == 0) {
                            row_0[0] = *row_ptr_m1 * 0x10001;
                            row_1[0] = *row_ptr * 0x10001;
                            row_2[0] = *row_ptr_p1 * 0x10001;
                            if (img->w >= 3) {
                                row_0[1] = *((uint32_t *) (row_ptr_m1 + 1));
                                row_1[1] = *((uint32_t *) (row_ptr + 1));
                                row_2[1] = *((uint32_t *) (row_ptr_p1 + 1));
                            } else if (img->w >= 2) {
                                row_0[1] = row_ptr_m1[1] * 0x10001;
                                row_1[1] = row_ptr[1] * 0x10001;
                                row_2[1] = row_ptr_p1[1] * 0x10001;
                            } else {
                                row_0[1] = row_0[0];
                                row_1[1] = row_1[0];
                                row_2[1] = row_2[0];
                            }
                        } else if (x == (img->w - 2)) {
                            row_0[0] = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_0[1] = row_ptr_m1[x + 1] * 0x10001;
                            row_1[0] = *((uint32_t *) (row_ptr + x - 1));
                            row_1[1] = row_ptr[x + 1] * 0x10001;
                            row_2[0] = *((uint32_t *) (row_ptr_p1 + x - 1));
                            row_2[1] = row_ptr_p1[x + 1] * 0x10001;
                        } else if (x >= (img->w - 1)) {
                            row_0[0] = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_0[1] = __PKHTB(row_0[0], row_0[0], 16);
                            row_1[0] = *((uint32_t *) (row_ptr + x - 1));
                            row_1[1] = __PKHTB(row_1[0], row_1[0], 16);
                            row_2[0] = *((uint32_t *) (row_ptr_p1 + x - 1));
                            row_2[1] = __PKHTB(row_2[0], row_2[0], 16);
                        } else {
                            // get 3 neighboring rows
                            row_0[0] = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_0[1] = *((uint32_t *) (row_ptr_m1 + x + 1));
                            row_1[0] = *((uint32_t *) (row_ptr + x - 1));
                            row_1[1] = *((uint32_t *) (row_ptr + x + 1));
                            row_2[0] = *((uint32_t *) (row_ptr_p1 + x - 1));
                            row_2[1] = *((uint32_t *) (row_ptr_p1 + x + 1));
                        }

                        int32_t p0_8_2 = __PKHBT(row_0[1], row_2[1], 16);

                        int32_t p0_r_acc = ((row_1[1] >> 11) & 0x1F) * krn_5;
                        p0_r_acc = __SMLAD((row_0[0] >> 11) & 0x1F001F, krn_1_0, p0_r_acc);
                        p0_r_acc = __SMLAD((row_1[0] >> 11) & 0x1F001F, krn_4_3, p0_r_acc);
                        p0_r_acc = __SMLAD((row_2[0] >> 11) & 0x1F001F, krn_7_6, p0_r_acc);
                        p0_r_acc = __SMLAD((p0_8_2 >> 11) & 0x1F001F, krn_8_2, p0_r_acc);
                        p0_r_acc = (p0_r_acc * m_int) + b_int;
                        p0_r_acc = __USAT_ASR(p0_r_acc, 5, 16);

                        int32_t p0_g_acc = ((row_1[1] >> 5) & 0x3F) * krn_5;
                        p0_g_acc = __SMLAD((row_0[0] >> 5) & 0x3F003F, krn_1_0, p0_g_acc);
                        p0_g_acc = __SMLAD((row_1[0] >> 5) & 0x3F003F, krn_4_3, p0_g_acc);
                        p0_g_acc = __SMLAD((row_2[0] >> 5) & 0x3F003F, krn_7_6, p0_g_acc);
                        p0_g_acc = __SMLAD((p0_8_2 >> 5) & 0x3F003F, krn_8_2, p0_g_acc);
                        p0_g_acc = (p0_g_acc * m_int) + b_int;
                        p0_g_acc = __USAT_ASR(p0_g_acc, 6, 16);

                        int32_t p0_b_acc = (row_1[1] & 0x1F) * krn_5;
                        p0_b_acc = __SMLAD(row_0[0] & 0x1F001F, krn_1_0, p0_b_acc);
                        p0_b_acc = __SMLAD(row_1[0] & 0x1F001F, krn_4_3, p0_b_acc);
                        p0_b_acc = __SMLAD(row_2[0] & 0x1F001F, krn_7_6, p0_b_acc);
                        p0_b_acc = __SMLAD(p0_8_2 & 0x1F001F, krn_8_2, p0_b_acc);
                        p0_b_acc = (p0_b_acc * m_int) + b_int;
                        p0_b_acc = __USAT_ASR(p0_b_acc, 5, 16);

                        int pixel0 = COLOR_R5_G6_B5_TO_RGB565(p0_r_acc, p0_g_acc, p0_b_acc);

                        int32_t p1_8_2 = __PKHTB(row_2[1], row_0[1], 16);
                        int32_t p1_1_0 = (row_0[1] << 16) | (row_0[0] >> 16);
                        int32_t p1_4_3 = (row_1[1] << 16) | (row_1[0] >> 16);
                        int32_t p1_7_6 = (row_2[1] << 16) | (row_2[0] >> 16);

                        int32_t p1_r_acc = (row_1[1] >> 27) * krn_5;
                        p1_r_acc = __SMLAD((p1_1_0 >> 11) & 0x1F001F, krn_1_0, p1_r_acc);
                        p1_r_acc = __SMLAD((p1_4_3 >> 11) & 0x1F001F, krn_4_3, p1_r_acc);
                        p1_r_acc = __SMLAD((p1_7_6 >> 11) & 0x1F001F, krn_7_6, p1_r_acc);
                        p1_r_acc = __SMLAD((p1_8_2 >> 11) & 0x1F001F, krn_8_2, p1_r_acc);
                        p1_r_acc = (p1_r_acc * m_int) + b_int;
                        p1_r_acc = __USAT_ASR(p1_r_acc, 5, 16);

                        int32_t p1_g_acc = ((row_1[1] >> 21) & 0x3F) * krn_5;
                        p1_g_acc = __SMLAD((p1_1_0 >> 5) & 0x3F003F, krn_1_0, p1_g_acc);
                        p1_g_acc = __SMLAD((p1_4_3 >> 5) & 0x3F003F, krn_4_3, p1_g_acc);
                        p1_g_acc = __SMLAD((p1_7_6 >> 5) & 0x3F003F, krn_7_6, p1_g_acc);
                        p1_g_acc = __SMLAD((p1_8_2 >> 5) & 0x3F003F, krn_8_2, p1_g_acc);
                        p1_g_acc = (p1_g_acc * m_int) + b_int;
                        p1_g_acc = __USAT_ASR(p1_g_acc, 6, 16);

                        int32_t p1_b_acc = ((row_1[1] >> 16) & 0x1F) * krn_5;
                        p1_b_acc = __SMLAD(p1_1_0 & 0x1F001F, krn_1_0, p1_b_acc);
                        p1_b_acc = __SMLAD(p1_4_3 & 0x1F001F, krn_4_3, p1_b_acc);
                        p1_b_acc = __SMLAD(p1_7_6 & 0x1F001F, krn_7_6, p1_b_acc);
                        p1_b_acc = __SMLAD(p1_8_2 & 0x1F001F, krn_8_2, p1_b_acc);
                        p1_b_acc = (p1_b_acc * m_int) + b_int;
                        p1_b_acc = __USAT_ASR(p1_b_acc, 5, 16);

                        int pixel1 = COLOR_R5_G6_B5_TO_RGB565(p1_r_acc, p1_g_acc, p1_b_acc);

                        // Re-pack to make thresholding faster.
                        int32_t p1_p0 = __PKHBT(pixel0, pixel1, 16);

                        if (threshold) {
                            int32_t r_p = __PKHBT(p0_r_acc, p1_r_acc, 16);
                            int32_t g_p = __PKHBT(p0_g_acc, p1_g_acc, 16);
                            int32_t b_p = __PKHBT(p0_b_acc, p1_b_acc, 16);
                            int32_t r_l = (p1_4_3 >> 11) & 0x1F001F;
                            int32_t g_l = (p1_4_3 >> 5) & 0x3F003F;
                            int32_t b_l = p1_4_3 & 0x1F001F;
                            // Note, since the above values are rgb565 versus rgb888 we adjust
                            // the yuv transform below to account for the scale difference.
                            // r5 to r8 scale = (r << 3) | (r >> 2) = 8.25 ~= 255/31
                            // g6 to g8 scale = (g << 2) | (g >> 4) = 4.0625 ~= 255/63
                            // b5 to b8 scale = (b << 3) | (b >> 2) = 8.25 ~= 255/31
                            // r -> 38 * 8.25 = 313.5 -> 313
                            // g -> 75 * 4.0625 = 304.6875 -> 305
                            // b -> 15 * 8.25 = 123.75 -> 124
                            int y_p = __UXTB16(((r_p * 313) + (g_p * 305) + (b_p * 124)) >> 7);
                            int y_l = __UXTB16(((r_l * 313) + (g_l * 305) + (b_l * 124)) >> 7);
                            p1_p0 = __SSUB16(__SSUB16(y_p, offset_int), y_l);
                            p1_p0 = __SEL(invert_ge, invert_lt);
                        }

                        if (x == (img->w - 1)) {
                            // just put bottom
                            IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, p1_p0);
                        } else {
                            // put both
                            *((uint32_t *) (buf_row_ptr + x)) = p1_p0;
                        }
                    }
                #endif
                } else {
                    for (int x = 0; x < img->w; x++) {
                        if (mask && (!image_get_mask_pixel(mask, x, y))) {
                            int p = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                            IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, p);
                            continue; // Short circuit.
                        }

                        int32_t r_acc = 0, g_acc = 0, b_acc = 0, ptr = 0;

                        if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                            for (int j = -ksize; j <= ksize; j++) {
                                uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + j);
                                for (int k = -ksize; k <= ksize; k++) {
                                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x + k);
                                    r_acc += krn[ptr] * COLOR_RGB565_TO_R5(pixel);
                                    g_acc += krn[ptr] * COLOR_RGB565_TO_G6(pixel);
                                    b_acc += krn[ptr++] * COLOR_RGB565_TO_B5(pixel);
                                }
                            }
                        } else {
                            for (int j = -ksize; j <= ksize; j++) {
                                int y_j = IM_MIN(IM_MAX(y + j, 0), (img->h - 1));
                                uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y_j);

                                for (int k = -ksize; k <= ksize; k++) {
                                    int x_k = IM_MIN(IM_MAX(x + k, 0), (img->w - 1));
                                    int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x_k);
                                    r_acc += krn[ptr] * COLOR_RGB565_TO_R5(pixel);
                                    g_acc += krn[ptr] * COLOR_RGB565_TO_G6(pixel);
                                    b_acc += krn[ptr++] * COLOR_RGB565_TO_B5(pixel);
                                }
                            }
                        }

                        int32_t r_tmp = (r_acc * m_int) + b_int;
                        int r_pixel = __USAT_ASR(r_tmp, 5, 16);

                        int32_t g_tmp = (g_acc * m_int) + b_int;
                        int g_pixel = __USAT_ASR(g_tmp, 6, 16);

                        int32_t b_tmp = (b_acc * m_int) + b_int;
                        int b_pixel = __USAT_ASR(b_tmp, 5, 16);

                        int pixel = COLOR_R5_G6_B5_TO_RGB565(r_pixel, g_pixel, b_pixel);

                        if (threshold) {
                            pixel = COLOR_RGB565_TO_Y(pixel) - offset;
                            pixel = pixel < COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                            pixel = (pixel ^ invert) * COLOR_RGB565_BINARY_MAX;
                        }

                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                    }
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0); y < img->h; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB888: {
            buf.data = fb_alloc(IMAGE_RGB888_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);

            #if defined(ARM_MATH_DSP)
            int32_t krn_5, krn_1_0, krn_4_3, krn_7_6, krn_8_2, offset_int, invert_ge, invert_lt;
            if (ksize == 1) {
                krn_5 = krn[5];
                krn_1_0 = __PKHBT(krn[0], krn[1], 16);
                krn_4_3 = __PKHBT(krn[3], krn[4], 16);
                krn_7_6 = __PKHBT(krn[6], krn[7], 16);
                krn_8_2 = __PKHBT(krn[2], krn[8], 16);
                offset_int = __PKHBT(offset, offset, 16);
                invert_ge = invert ? 0xFFFFFFFF : 0x00000000;
                invert_lt = invert ? 0x00000000 : 0xFFFFFFFF;
            }
            #endif

            for (int y = 0; y < img->h; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                pixel_rgb_t *buf_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows));

                if (0) {
                #if defined(ARM_MATH_DSP)
                } else if (0 && (ksize == 1) && (!mask)) {
                    pixel_rgb_t *row_ptr_m1, *row_ptr_p1;

                    if (y == 0) {
                        row_ptr_m1 = row_ptr;
                        row_ptr_p1 = row_ptr + ((img->h >= 2) ? img->w : 0);
                    } else if (y >= (img->h - 1)) {
                        row_ptr_m1 = row_ptr - img->w;
                        row_ptr_p1 = row_ptr;
                    } else {
                        // get 2 neighboring rows
                        row_ptr_m1 = row_ptr - img->w;
                        row_ptr_p1 = row_ptr + img->w;
                    }

                    // If the image is an odd width this will go for the last loop and we drop the last column.
                    for (int x = 0; x < img->w; x += 2) {
                        uint32_t row_0[2], row_1[2], row_2[2];

                        if (x == 0) {
                            row_0[0] = *row_ptr_m1 * 0x10001;
                            row_1[0] = *row_ptr * 0x10001;
                            row_2[0] = *row_ptr_p1 * 0x10001;
                            if (img->w >= 3) {
                                row_0[1] = *((uint32_t *) (row_ptr_m1 + 1));
                                row_1[1] = *((uint32_t *) (row_ptr + 1));
                                row_2[1] = *((uint32_t *) (row_ptr_p1 + 1));
                            } else if (img->w >= 2) {
                                row_0[1] = row_ptr_m1[1] * 0x10001;
                                row_1[1] = row_ptr[1] * 0x10001;
                                row_2[1] = row_ptr_p1[1] * 0x10001;
                            } else {
                                row_0[1] = row_0[0];
                                row_1[1] = row_1[0];
                                row_2[1] = row_2[0];
                            }
                        } else if (x == (img->w - 2)) {
                            row_0[0] = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_0[1] = row_ptr_m1[x + 1] * 0x10001;
                            row_1[0] = *((uint32_t *) (row_ptr + x - 1));
                            row_1[1] = row_ptr[x + 1] * 0x10001;
                            row_2[0] = *((uint32_t *) (row_ptr_p1 + x - 1));
                            row_2[1] = row_ptr_p1[x + 1] * 0x10001;
                        } else if (x >= (img->w - 1)) {
                            row_0[0] = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_0[1] = __PKHTB(row_0[0], row_0[0], 16);
                            row_1[0] = *((uint32_t *) (row_ptr + x - 1));
                            row_1[1] = __PKHTB(row_1[0], row_1[0], 16);
                            row_2[0] = *((uint32_t *) (row_ptr_p1 + x - 1));
                            row_2[1] = __PKHTB(row_2[0], row_2[0], 16);
                        } else {
                            // get 3 neighboring rows
                            row_0[0] = *((uint32_t *) (row_ptr_m1 + x - 1));
                            row_0[1] = *((uint32_t *) (row_ptr_m1 + x + 1));
                            row_1[0] = *((uint32_t *) (row_ptr + x - 1));
                            row_1[1] = *((uint32_t *) (row_ptr + x + 1));
                            row_2[0] = *((uint32_t *) (row_ptr_p1 + x - 1));
                            row_2[1] = *((uint32_t *) (row_ptr_p1 + x + 1));
                        }

                        int32_t p0_8_2 = __PKHBT(row_0[1], row_2[1], 16);

                        int32_t p0_r_acc = ((row_1[1] >> 11) & 0x1F) * krn_5;
                        p0_r_acc = __SMLAD((row_0[0] >> 11) & 0x1F001F, krn_1_0, p0_r_acc);
                        p0_r_acc = __SMLAD((row_1[0] >> 11) & 0x1F001F, krn_4_3, p0_r_acc);
                        p0_r_acc = __SMLAD((row_2[0] >> 11) & 0x1F001F, krn_7_6, p0_r_acc);
                        p0_r_acc = __SMLAD((p0_8_2 >> 11) & 0x1F001F, krn_8_2, p0_r_acc);
                        p0_r_acc = (p0_r_acc * m_int) + b_int;
                        p0_r_acc = __USAT_ASR(p0_r_acc, 5, 16);

                        int32_t p0_g_acc = ((row_1[1] >> 5) & 0x3F) * krn_5;
                        p0_g_acc = __SMLAD((row_0[0] >> 5) & 0x3F003F, krn_1_0, p0_g_acc);
                        p0_g_acc = __SMLAD((row_1[0] >> 5) & 0x3F003F, krn_4_3, p0_g_acc);
                        p0_g_acc = __SMLAD((row_2[0] >> 5) & 0x3F003F, krn_7_6, p0_g_acc);
                        p0_g_acc = __SMLAD((p0_8_2 >> 5) & 0x3F003F, krn_8_2, p0_g_acc);
                        p0_g_acc = (p0_g_acc * m_int) + b_int;
                        p0_g_acc = __USAT_ASR(p0_g_acc, 6, 16);

                        int32_t p0_b_acc = (row_1[1] & 0x1F) * krn_5;
                        p0_b_acc = __SMLAD(row_0[0] & 0x1F001F, krn_1_0, p0_b_acc);
                        p0_b_acc = __SMLAD(row_1[0] & 0x1F001F, krn_4_3, p0_b_acc);
                        p0_b_acc = __SMLAD(row_2[0] & 0x1F001F, krn_7_6, p0_b_acc);
                        p0_b_acc = __SMLAD(p0_8_2 & 0x1F001F, krn_8_2, p0_b_acc);
                        p0_b_acc = (p0_b_acc * m_int) + b_int;
                        p0_b_acc = __USAT_ASR(p0_b_acc, 5, 16);

                        int pixel0 = COLOR_R8_G8_B8_TO_RGB888(p0_r_acc, p0_g_acc, p0_b_acc);

                        int32_t p1_8_2 = __PKHTB(row_2[1], row_0[1], 16);
                        int32_t p1_1_0 = (row_0[1] << 16) | (row_0[0] >> 16);
                        int32_t p1_4_3 = (row_1[1] << 16) | (row_1[0] >> 16);
                        int32_t p1_7_6 = (row_2[1] << 16) | (row_2[0] >> 16);

                        int32_t p1_r_acc = (row_1[1] >> 27) * krn_5;
                        p1_r_acc = __SMLAD((p1_1_0 >> 11) & 0x1F001F, krn_1_0, p1_r_acc);
                        p1_r_acc = __SMLAD((p1_4_3 >> 11) & 0x1F001F, krn_4_3, p1_r_acc);
                        p1_r_acc = __SMLAD((p1_7_6 >> 11) & 0x1F001F, krn_7_6, p1_r_acc);
                        p1_r_acc = __SMLAD((p1_8_2 >> 11) & 0x1F001F, krn_8_2, p1_r_acc);
                        p1_r_acc = (p1_r_acc * m_int) + b_int;
                        p1_r_acc = __USAT_ASR(p1_r_acc, 5, 16);

                        int32_t p1_g_acc = ((row_1[1] >> 21) & 0x3F) * krn_5;
                        p1_g_acc = __SMLAD((p1_1_0 >> 5) & 0x3F003F, krn_1_0, p1_g_acc);
                        p1_g_acc = __SMLAD((p1_4_3 >> 5) & 0x3F003F, krn_4_3, p1_g_acc);
                        p1_g_acc = __SMLAD((p1_7_6 >> 5) & 0x3F003F, krn_7_6, p1_g_acc);
                        p1_g_acc = __SMLAD((p1_8_2 >> 5) & 0x3F003F, krn_8_2, p1_g_acc);
                        p1_g_acc = (p1_g_acc * m_int) + b_int;
                        p1_g_acc = __USAT_ASR(p1_g_acc, 6, 16);

                        int32_t p1_b_acc = ((row_1[1] >> 16) & 0x1F) * krn_5;
                        p1_b_acc = __SMLAD(p1_1_0 & 0x1F001F, krn_1_0, p1_b_acc);
                        p1_b_acc = __SMLAD(p1_4_3 & 0x1F001F, krn_4_3, p1_b_acc);
                        p1_b_acc = __SMLAD(p1_7_6 & 0x1F001F, krn_7_6, p1_b_acc);
                        p1_b_acc = __SMLAD(p1_8_2 & 0x1F001F, krn_8_2, p1_b_acc);
                        p1_b_acc = (p1_b_acc * m_int) + b_int;
                        p1_b_acc = __USAT_ASR(p1_b_acc, 5, 16);

                        int pixel1 = COLOR_R8_G8_B8_TO_RGB888(p1_r_acc, p1_g_acc, p1_b_acc);

                        // Re-pack to make thresholding faster.
                        int32_t p1_p0 = __PKHBT(pixel0, pixel1, 16);

                        if (threshold) {
                            int32_t r_p = __PKHBT(p0_r_acc, p1_r_acc, 16);
                            int32_t g_p = __PKHBT(p0_g_acc, p1_g_acc, 16);
                            int32_t b_p = __PKHBT(p0_b_acc, p1_b_acc, 16);
                            int32_t r_l = (p1_4_3 >> 11) & 0x1F001F;
                            int32_t g_l = (p1_4_3 >> 5) & 0x3F003F;
                            int32_t b_l = p1_4_3 & 0x1F001F;
                            // Note, since the above values are rgb565 versus rgb888 we adjust
                            // the yuv transform below to account for the scale difference.
                            // r5 to r8 scale = (r << 3) | (r >> 2) = 8.25 ~= 255/31
                            // g6 to g8 scale = (g << 2) | (g >> 4) = 4.0625 ~= 255/63
                            // b5 to b8 scale = (b << 3) | (b >> 2) = 8.25 ~= 255/31
                            // r -> 38 * 8.25 = 313.5 -> 313
                            // g -> 75 * 4.0625 = 304.6875 -> 305
                            // b -> 15 * 8.25 = 123.75 -> 124
                            int y_p = __UXTB16(((r_p * 313) + (g_p * 305) + (b_p * 124)) >> 7);
                            int y_l = __UXTB16(((r_l * 313) + (g_l * 305) + (b_l * 124)) >> 7);
                            p1_p0 = __SSUB16(__SSUB16(y_p, offset_int), y_l);
                            p1_p0 = __SEL(invert_ge, invert_lt);
                        }

                        if (x == (img->w - 1)) {
                            // just put bottom
                            IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, p1_p0);
                        } else {
                            // put both
                            *((uint32_t *) (buf_row_ptr + x)) = p1_p0;
                        }
                    }
                #endif
                } else {
                    for (int x = 0; x < img->w; x++) {
                        if (mask && (!image_get_mask_pixel(mask, x, y))) {
                            pixel_rgb_t p = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                            IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, p);
                            continue; // Short circuit.
                        }

                        int32_t r_acc = 0, g_acc = 0, b_acc = 0, ptr = 0;

                        if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                            for (int j = -ksize; j <= ksize; j++) {
                                pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y + j);
                                for (int k = -ksize; k <= ksize; k++) {
                                    pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x + k);
                                    r_acc += krn[ptr] * COLOR_RGB888_TO_R8(pixel);
                                    g_acc += krn[ptr] * COLOR_RGB888_TO_G8(pixel);
                                    b_acc += krn[ptr++] * COLOR_RGB888_TO_B8(pixel);
                                }
                            }
                        } else {
                            for (int j = -ksize; j <= ksize; j++) {
                                int y_j = IM_MIN(IM_MAX(y + j, 0), (img->h - 1));
                                pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y_j);

                                for (int k = -ksize; k <= ksize; k++) {
                                    int x_k = IM_MIN(IM_MAX(x + k, 0), (img->w - 1));
                                    pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr, x_k);
                                    r_acc += krn[ptr] * COLOR_RGB888_TO_R8(pixel);
                                    g_acc += krn[ptr] * COLOR_RGB888_TO_G8(pixel);
                                    b_acc += krn[ptr++] * COLOR_RGB888_TO_B8(pixel);
                                }
                            }
                        }

                        int32_t r_tmp = (r_acc * m_int) + b_int;
                        int r_pixel = __USAT_ASR(r_tmp, 8, 16);

                        int32_t g_tmp = (g_acc * m_int) + b_int;
                        int g_pixel = __USAT_ASR(g_tmp, 8, 16);

                        int32_t b_tmp = (b_acc * m_int) + b_int;
                        int b_pixel = __USAT_ASR(b_tmp, 8, 16);

                        pixel_rgb_t pixel = COLOR_R8_G8_B8_TO_RGB888(r_pixel, g_pixel, b_pixel);

                        if (threshold) {
                            int tmp_y = COLOR_RGB888_TO_Y(pixel.r, pixel.g, pixel.b) - offset;
                            pixel_rgb_t tmp_pixel = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                            tmp_y = tmp_y < COLOR_RGB888_TO_Y(tmp_pixel.r, tmp_pixel.g, tmp_pixel.r);
                            tmp_y = (tmp_y ^ invert) * COLOR_RGB888_BINARY_MAX;
                            pixel_rgb_t pixel_tmp = COLOR_YUV_TO_RGB888(tmp_y, 128, 128);
                            pixel = pixel_tmp;
                        }

                        IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, pixel);
                    }
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB888_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0); y < img->h; y++) {
                memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB888_LINE_LEN_BYTES(img));
            }

            if (buf.data) fb_free(buf.data);
            break;
        }
        default: {
            break;
        }
    }
}

#ifdef IMLIB_ENABLE_BILATERAL
static float gaussian(float x, float sigma) {
    return fast_expf((x * x) / (-2.0f * sigma * sigma)) / (fabsf(sigma) * 2.506628f); // sqrt(2 * PI)
}

static float distance(int x, int y) {
    return fast_sqrtf((x * x) + (y * y));
}

void imlib_bilateral_filter(image_t *img,
                            const int ksize,
                            float color_sigma,
                            float space_sigma,
                            bool threshold,
                            int offset,
                            bool invert,
                            image_t *mask) {
    int brows = ksize + 1;
    image_t buf;
    buf.w = img->w;
    buf.h = brows;
    buf.pixfmt = img->pixfmt;

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            buf.data = fb_alloc(IMAGE_BINARY_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            float *gi_lut_ptr = fb_alloc((COLOR_BINARY_MAX - COLOR_BINARY_MIN + 1) * sizeof(float) * 2, FB_ALLOC_NO_HINT);
            float *gi_lut = &gi_lut_ptr[1];
            float max_color = IM_DIV(1.0f, COLOR_BINARY_MAX - COLOR_BINARY_MIN);
            for (int i = COLOR_BINARY_MIN; i <= COLOR_BINARY_MAX; i++) {
                gi_lut[-i] = gi_lut[i] = gaussian(i * max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float), FB_ALLOC_NO_HINT);

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y);
                uint32_t *buf_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int this_pixel = IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x);
                    float i_acc = 0, w_acc = 0;
                    int ptr = 0;
                    for (int j = -ksize; j <= ksize; j++) {
                        uint32_t *k_row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img,
                                                                                 IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                        for (int k = -ksize; k <= ksize; k++) {
                            int pixel = IMAGE_GET_BINARY_PIXEL_FAST(k_row_ptr,
                                                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                            float w = gi_lut[(this_pixel - pixel)] * gs_lut[ptr++];
                            i_acc += pixel * w;
                            w_acc += w;
                        }
                    }

                    int pixel = fast_floorf(IM_MIN(IM_DIV(i_acc, w_acc), COLOR_BINARY_MAX));

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_BINARY_MAX;
                        } else {
                            pixel = COLOR_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_BINARY_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_BINARY_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_BINARY_LINE_LEN_BYTES(img));
            }

            if (gs_lut) fb_free(gs_lut);
            if (gi_lut_ptr) fb_free(gi_lut_ptr);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            buf.data = fb_alloc(IMAGE_GRAYSCALE_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            float *gi_lut_ptr = fb_alloc((COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN + 1) * sizeof(float) * 2, FB_ALLOC_NO_HINT);
            float *gi_lut = &gi_lut_ptr[256]; // point to the middle
            float max_color = IM_DIV(1.0f, COLOR_GRAYSCALE_MAX - COLOR_GRAYSCALE_MIN);
            for (int i = COLOR_GRAYSCALE_MIN; i <= COLOR_GRAYSCALE_MAX; i++) {
                gi_lut[-i] = gi_lut[i] = gaussian(i * max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float), FB_ALLOC_NO_HINT);

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y);
                uint8_t *buf_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int this_pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x);
                    float i_acc = 0, w_acc = 0;
                    int ptr = 0;
                    if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y + j);
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr, x + k);
                                float w = gi_lut[this_pixel - pixel] * gs_lut[ptr++];
                                i_acc += pixel * w;
                                w_acc += w;
                            }
                        }
                    } else {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint8_t *k_row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img,
                                                                                       IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_GRAYSCALE_PIXEL_FAST(k_row_ptr,
                                                                           IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                float w = gi_lut[(this_pixel - pixel)] * gs_lut[ptr++];
                                i_acc += pixel * w;
                                w_acc += w;
                            }
                        }
                    }

                    int pixel = fast_floorf(IM_MIN(IM_DIV(i_acc, w_acc), COLOR_GRAYSCALE_MAX));

                    if (threshold) {
                        if (((pixel - offset) < IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, x)) ^ invert) {
                            pixel = COLOR_GRAYSCALE_BINARY_MAX;
                        } else {
                            pixel = COLOR_GRAYSCALE_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_GRAYSCALE_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_GRAYSCALE_LINE_LEN_BYTES(img));
            }

            if (gs_lut) fb_free(gs_lut);
            if (gi_lut_ptr) fb_free(gi_lut_ptr);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB565: {
            buf.data = fb_alloc(IMAGE_RGB565_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            float *rb_gi_ptr = fb_alloc((COLOR_R5_MAX - COLOR_R5_MIN + 1) * sizeof(float) * 2, FB_ALLOC_NO_HINT);
            float *g_gi_ptr = fb_alloc((COLOR_G6_MAX - COLOR_G6_MIN + 1) * sizeof(float) * 2, FB_ALLOC_NO_HINT);
            float *rb_gi_lut = &rb_gi_ptr[32]; // center
            float *g_gi_lut = &g_gi_ptr[64];

            float r_max_color = IM_DIV(1.0f, COLOR_R5_MAX - COLOR_R5_MIN);
            for (int i = COLOR_R5_MIN; i <= COLOR_R5_MAX; i++) {
                rb_gi_lut[-i] = rb_gi_lut[i] = gaussian(i * r_max_color, color_sigma);
            }

            float g_max_color = IM_DIV(1.0f, COLOR_G6_MAX - COLOR_G6_MIN);
            for (int i = COLOR_G6_MIN; i <= COLOR_G6_MAX; i++) {
                g_gi_lut[-i] = g_gi_lut[i] = gaussian(i * g_max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float), FB_ALLOC_NO_HINT);

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y);
                uint16_t *buf_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    int this_pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x);
                    int r_this_pixel = COLOR_RGB565_TO_R5(this_pixel);
                    int g_this_pixel = COLOR_RGB565_TO_G6(this_pixel);
                    int b_this_pixel = COLOR_RGB565_TO_B5(this_pixel);
                    float r_i_acc = 0, r_w_acc = 0;
                    float g_i_acc = 0, g_w_acc = 0;
                    float b_i_acc = 0, b_w_acc = 0;
                    int ptr = 0;
                    if (x >= ksize && x < img->w - ksize && y >= ksize && y < img->h - ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y + j);
                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr, x + k);
                                int r_pixel = COLOR_RGB565_TO_R5(pixel);
                                int g_pixel = COLOR_RGB565_TO_G6(pixel);
                                int b_pixel = COLOR_RGB565_TO_B5(pixel);
                                float gs = gs_lut[ptr++];
                                float r_w = rb_gi_lut[(r_this_pixel - r_pixel)] * gs;
                                float g_w = g_gi_lut[(g_this_pixel - g_pixel)] * gs;
                                float b_w = rb_gi_lut[(b_this_pixel - b_pixel)] * gs;
                                r_i_acc += r_pixel * r_w;
                                r_w_acc += r_w;
                                g_i_acc += g_pixel * g_w;
                                g_w_acc += g_w;
                                b_i_acc += b_pixel * b_w;
                                b_w_acc += b_w;
                            }
                        }
                    } else {
                        // check boundary conditions
                        for (int j = -ksize; j <= ksize; j++) {
                            uint16_t *k_row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img,
                                                                                     IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(k_row_ptr,
                                                                        IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                int r_pixel = COLOR_RGB565_TO_R5(pixel);
                                int g_pixel = COLOR_RGB565_TO_G6(pixel);
                                int b_pixel = COLOR_RGB565_TO_B5(pixel);
                                float gs = gs_lut[ptr++];
                                float r_w = rb_gi_lut[(r_this_pixel - r_pixel)] * gs;
                                float g_w = g_gi_lut[(g_this_pixel - g_pixel)] * gs;
                                float b_w = rb_gi_lut[(b_this_pixel - b_pixel)] * gs;
                                r_i_acc += r_pixel * r_w;
                                r_w_acc += r_w;
                                g_i_acc += g_pixel * g_w;
                                g_w_acc += g_w;
                                b_i_acc += b_pixel * b_w;
                                b_w_acc += b_w;
                            }
                        }
                    }

                    int pixel = COLOR_R5_G6_B5_TO_RGB565(fast_floorf(IM_MIN(IM_DIV(r_i_acc, r_w_acc), COLOR_R5_MAX)),
                                                         fast_floorf(IM_MIN(IM_DIV(g_i_acc, g_w_acc), COLOR_G6_MAX)),
                                                         fast_floorf(IM_MIN(IM_DIV(b_i_acc, b_w_acc), COLOR_B5_MAX)));

                    if (threshold) {
                        if (((COLOR_RGB565_TO_Y(pixel) - offset) <
                             COLOR_RGB565_TO_Y(IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, x))) ^ invert) {
                            pixel = COLOR_RGB565_BINARY_MAX;
                        } else {
                            pixel = COLOR_RGB565_BINARY_MIN;
                        }
                    }

                    IMAGE_PUT_RGB565_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) {
                    // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB565_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB565_LINE_LEN_BYTES(img));
            }

            if (gs_lut) fb_free(gs_lut);
            if (g_gi_ptr) fb_free(g_gi_ptr);
            if (rb_gi_ptr) fb_free(rb_gi_ptr);
            if (buf.data) fb_free(buf.data);
            break;
        }
        case PIXFORMAT_RGB888: {
            buf.data = fb_alloc(IMAGE_RGB888_LINE_LEN_BYTES(img) * brows, FB_ALLOC_NO_HINT);
            float *r_gi_ptr = fb_alloc((COLOR_R8_MAX - COLOR_R8_MIN + 1) * sizeof(float) *3, FB_ALLOC_NO_HINT);
            float *g_gi_ptr = fb_alloc((COLOR_G8_MAX - COLOR_G8_MIN + 1) * sizeof(float) *3, FB_ALLOC_NO_HINT);
            float *b_gi_ptr = fb_alloc((COLOR_B8_MAX - COLOR_B8_MIN + 1) * sizeof(float) *3, FB_ALLOC_NO_HINT);
            float *r_gi_lut = &r_gi_ptr[256]; // center
            float *g_gi_lut = &g_gi_ptr[256];
            float *b_gi_lut = &b_gi_ptr[256]; // center

            float r_max_color = IM_DIV(1.0f, COLOR_R8_MAX - COLOR_R8_MIN);
            for (int i = COLOR_R8_MIN; i <= COLOR_R8_MAX; i++) {
                r_gi_lut[-i] = r_gi_lut[i] = gaussian(i * r_max_color, color_sigma);
            }

            float g_max_color = IM_DIV(1.0f, COLOR_G8_MAX - COLOR_G8_MIN);
            for (int i = COLOR_G8_MIN; i <= COLOR_G8_MAX; i++) {
                g_gi_lut[-i] = g_gi_lut[i] = gaussian(i * g_max_color, color_sigma);
            }

            float b_max_color = IM_DIV(1.0f, COLOR_B8_MAX - COLOR_B8_MIN);
            for (int i = COLOR_B8_MIN; i <= COLOR_B8_MAX; i++) {
                b_gi_lut[-i] = b_gi_lut[i] = gaussian(i * b_max_color, color_sigma);
            }

            int n = (ksize * 2) + 1;
            float *gs_lut = fb_alloc(n * n * sizeof(float), FB_ALLOC_NO_HINT);

            float max_space = IM_DIV(1.0f, distance(ksize, ksize));
            for (int y = -ksize; y <= ksize; y++) {
                for (int x = -ksize; x <= ksize; x++) {
                    gs_lut[(n * (y + ksize)) + (x + ksize)] = gaussian(distance(x, y) * max_space, space_sigma);
                }
            }

            for (int y = 0, yy = img->h; y < yy; y++) {
                pixel_rgb_t *row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y);
                pixel_rgb_t *buf_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows));

                for (int x = 0, xx = img->w; x < xx; x++) {
                    if (mask && (!image_get_mask_pixel(mask, x, y))) {
                        IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x));
                        continue; // Short circuit.
                    }

                    pixel_rgb_t this_pixel = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                    int r_this_pixel = COLOR_RGB888_TO_R8(this_pixel);
                    int g_this_pixel = COLOR_RGB888_TO_G8(this_pixel);
                    int b_this_pixel = COLOR_RGB888_TO_B8(this_pixel);
                    float r_i_acc = 0, r_w_acc = 0;
                    float g_i_acc = 0, g_w_acc = 0;
                    float b_i_acc = 0, b_w_acc = 0;
                    int ptr = 0;
                    if (x >= ksize && x < img->w-ksize && y >= ksize && y < img->h-ksize) {
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,y+j);
                            for (int k = -ksize; k <= ksize; k++) {
                                pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,x+k);
                                int r_pixel = COLOR_RGB888_TO_R8(pixel);
                                int g_pixel = COLOR_RGB888_TO_G8(pixel);
                                int b_pixel = COLOR_RGB888_TO_B8(pixel);
                                float gs = gs_lut[ptr++];
                                float r_w = r_gi_lut[(r_this_pixel - r_pixel)] * gs;
                                float g_w = g_gi_lut[(g_this_pixel - g_pixel)] * gs;
                                float b_w = r_gi_lut[(b_this_pixel - b_pixel)] * gs;
                                r_i_acc += r_pixel * r_w;
                                r_w_acc += r_w;
                                g_i_acc += g_pixel * g_w;
                                g_w_acc += g_w;
                                b_i_acc += b_pixel * b_w;
                                b_w_acc += b_w;
                            }
                        }
                    } else { // check boundary conditions
                        for (int j = -ksize; j <= ksize; j++) {
                            pixel_rgb_t *k_row_ptr = IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img,
                                IM_MIN(IM_MAX(y + j, 0), (img->h - 1)));

                            for (int k = -ksize; k <= ksize; k++) {
                                pixel_rgb_t pixel = IMAGE_GET_RGB888_PIXEL_FAST(k_row_ptr,
                                    IM_MIN(IM_MAX(x + k, 0), (img->w - 1)));
                                int r_pixel = COLOR_RGB888_TO_R8(pixel);
                                int g_pixel = COLOR_RGB888_TO_G8(pixel);
                                int b_pixel = COLOR_RGB888_TO_B8(pixel);
                                float gs = gs_lut[ptr++];
                                float r_w = r_gi_lut[(r_this_pixel - r_pixel)] * gs;
                                float g_w = g_gi_lut[(g_this_pixel - g_pixel)] * gs;
                                float b_w = r_gi_lut[(b_this_pixel - b_pixel)] * gs;
                                r_i_acc += r_pixel * r_w;
                                r_w_acc += r_w;
                                g_i_acc += g_pixel * g_w;
                                g_w_acc += g_w;
                                b_i_acc += b_pixel * b_w;
                                b_w_acc += b_w;
                            }
                        }
                    }

                    pixel_rgb_t pixel = COLOR_R8_G8_B8_TO_RGB888(fast_floorf(IM_MIN(IM_DIV(r_i_acc, r_w_acc), COLOR_R8_MAX)),
                                                         fast_floorf(IM_MIN(IM_DIV(g_i_acc, g_w_acc), COLOR_G8_MAX)),
                                                         fast_floorf(IM_MIN(IM_DIV(b_i_acc, b_w_acc), COLOR_B8_MAX)));

                    if (threshold) {
                        pixel_rgb_t temp = IMAGE_GET_RGB888_PIXEL_FAST(row_ptr, x);
                        if (((COLOR_RGB888_TO_Y(pixel.r, pixel.g, pixel.b) - offset) < COLOR_RGB888_TO_Y(temp.r, temp.g, temp.b)) ^ invert) {
                            pixel.r = COLOR_R8_MAX;
                            pixel.g = COLOR_G8_MAX;
                            pixel.b = COLOR_B8_MAX;
                        } else {
                            pixel.r = COLOR_R8_MIN;
                            pixel.g = COLOR_G8_MIN;
                            pixel.b = COLOR_B8_MIN;
                        }
                    }

                    IMAGE_PUT_RGB888_PIXEL_FAST(buf_row_ptr, x, pixel);
                }

                if (y >= ksize) { // Transfer buffer lines...
                    memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, (y - ksize)),
                           IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, ((y - ksize) % brows)),
                           IMAGE_RGB888_LINE_LEN_BYTES(img));
                }
            }

            // Copy any remaining lines from the buffer image...
            for (int y = IM_MAX(img->h - ksize, 0), yy = img->h; y < yy; y++) {
                memcpy(IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(img, y),
                       IMAGE_COMPUTE_RGB888_PIXEL_ROW_PTR(&buf, (y % brows)),
                       IMAGE_RGB888_LINE_LEN_BYTES(img));
            }
            if (gs_lut) fb_free(gs_lut);
            if (b_gi_ptr) fb_free(b_gi_ptr);
            if (g_gi_ptr) fb_free(g_gi_ptr);
            if (r_gi_ptr) fb_free(r_gi_ptr);
            if (buf.data) fb_free(buf.data);
            break;
        }
        default: {
            break;
        }
    }
}
#endif // IMLIB_ENABLE_BILATERAL

#ifdef IMLIB_ENABLE_CARTOON
typedef struct imlib_cartoon_filter_mean_state {
    int r_acc, g_acc, b_acc, pixels;
} imlib_cartoon_filter_mean_state_t;

static void imlib_cartoon_filter_mean(image_t *img, int line, int l, int r, void *data) {
    imlib_cartoon_filter_mean_state_t *state = (imlib_cartoon_filter_mean_state_t *) data;

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                state->g_acc += IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, i);
                state->pixels += 1;
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                state->g_acc += IMAGE_GET_GRAYSCALE_PIXEL_FAST(row_ptr, i);
                state->pixels += 1;
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                int pixel = IMAGE_GET_RGB565_PIXEL_FAST(row_ptr, i);
                state->r_acc += COLOR_RGB565_TO_R5(pixel);
                state->g_acc += COLOR_RGB565_TO_G6(pixel);
                state->b_acc += COLOR_RGB565_TO_B5(pixel);
                state->pixels += 1;
            }
            break;
        }
        default: {
            break;
        }
    }
}

typedef struct imlib_cartoon_filter_fill_state {
    int mean;
} imlib_cartoon_filter_fill_state_t;

static void imlib_cartoon_filter_fill(image_t *img, int line, int l, int r, void *data) {
    imlib_cartoon_filter_fill_state_t *state = (imlib_cartoon_filter_fill_state_t *) data;

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                IMAGE_PUT_BINARY_PIXEL_FAST(row_ptr, i, state->mean);
            }
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            uint8_t *row_ptr = IMAGE_COMPUTE_GRAYSCALE_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                IMAGE_PUT_GRAYSCALE_PIXEL_FAST(row_ptr, i, state->mean);
            }
            break;
        }
        case PIXFORMAT_RGB565: {
            uint16_t *row_ptr = IMAGE_COMPUTE_RGB565_PIXEL_ROW_PTR(img, line);
            for (int i = l; i <= r; i++) {
                IMAGE_PUT_RGB565_PIXEL_FAST(row_ptr, i, state->mean);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void imlib_cartoon_filter(image_t *img, float seed_threshold, float floating_threshold, image_t *mask) {
    image_t mean_image, fill_image;

    mean_image.w = img->w;
    mean_image.h = img->h;
    mean_image.pixfmt = PIXFORMAT_BINARY;
    mean_image.pixels = fb_alloc0(image_size(&mean_image), FB_ALLOC_NO_HINT);

    fill_image.w = img->w;
    fill_image.h = img->h;
    fill_image.pixfmt = PIXFORMAT_BINARY;
    fill_image.pixels = fb_alloc0(image_size(&fill_image), FB_ALLOC_NO_HINT);

    if (mask) {
        for (int y = 0, yy = fill_image.h; y < yy; y++) {
            uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&fill_image, y);
            for (int x = 0, xx = fill_image.w; x < xx; x++) {
                if (image_get_mask_pixel(mask, x, y)) {
                    IMAGE_SET_BINARY_PIXEL_FAST(row_ptr, x);
                }
            }
        }
    }

    int color_seed_threshold = 0;
    int color_floating_threshold = 0;

    switch (img->pixfmt) {
        case PIXFORMAT_BINARY: {
            color_seed_threshold = fast_floorf(seed_threshold * COLOR_BINARY_MAX);
            color_floating_threshold = fast_floorf(floating_threshold * COLOR_BINARY_MAX);
            break;
        }
        case PIXFORMAT_GRAYSCALE: {
            color_seed_threshold = fast_floorf(seed_threshold * COLOR_GRAYSCALE_MAX);
            color_floating_threshold = fast_floorf(floating_threshold * COLOR_GRAYSCALE_MAX);
            break;
        }
        case PIXFORMAT_RGB565: {
            color_seed_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_floorf(seed_threshold * COLOR_R5_MAX),
                                                            fast_floorf(seed_threshold * COLOR_G6_MAX),
                                                            fast_floorf(seed_threshold * COLOR_B5_MAX));
            color_floating_threshold = COLOR_R5_G6_B5_TO_RGB565(fast_floorf(floating_threshold * COLOR_R5_MAX),
                                                                fast_floorf(floating_threshold * COLOR_G6_MAX),
                                                                fast_floorf(floating_threshold * COLOR_B5_MAX));
            break;
        }
        default: {
            break;
        }
    }

    for (int y = 0, yy = img->h; y < yy; y++) {
        uint32_t *row_ptr = IMAGE_COMPUTE_BINARY_PIXEL_ROW_PTR(&mean_image, y);
        for (int x = 0, xx = img->w; x < xx; x++) {
            if (!IMAGE_GET_BINARY_PIXEL_FAST(row_ptr, x)) {

                imlib_cartoon_filter_mean_state_t mean_state;
                memset(&mean_state, 0, sizeof(imlib_cartoon_filter_mean_state_t));
                imlib_flood_fill_int(&mean_image, img, x, y, color_seed_threshold, color_floating_threshold,
                                     imlib_cartoon_filter_mean, &mean_state);

                imlib_cartoon_filter_fill_state_t fill_state;
                memset(&fill_state, 0, sizeof(imlib_cartoon_filter_fill_state_t));

                switch (img->pixfmt) {
                    case PIXFORMAT_BINARY: {
                        fill_state.mean = mean_state.g_acc / mean_state.pixels;
                        break;
                    }
                    case PIXFORMAT_GRAYSCALE: {
                        fill_state.mean = mean_state.g_acc / mean_state.pixels;
                        break;
                    }
                    case PIXFORMAT_RGB565: {
                        fill_state.mean = COLOR_R5_G6_B5_TO_RGB565(mean_state.r_acc / mean_state.pixels,
                                                                   mean_state.g_acc / mean_state.pixels,
                                                                   mean_state.b_acc / mean_state.pixels);
                        break;
                    }
                    default: {
                        break;
                    }
                }

                imlib_flood_fill_int(&fill_image, img, x, y, color_seed_threshold, color_floating_threshold,
                                     imlib_cartoon_filter_fill, &fill_state);
            }
        }
    }

    if (fill_image.pixels) fb_free(fill_image.pixels);
    if (mean_image.pixels) fb_free(mean_image.pixels);
}
#endif // IMLIB_ENABLE_CARTOON
