
#include "stdio.h"
#include "main.h"
#include "maix_util.hpp"
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_display.hpp"
#include "maix_camera.hpp"
#include "csignal"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <omv.hpp>

using namespace maix;

#define CAMERA_ENABLE                   1
#define DISPLAY_ENABLE                  1
#define INPUT_IMG_W                     640
#define INPUT_IMG_H                     480
#define PRINTF_IMG_EN                   0

#define TEST_MEANPOOL                   0       // OK
#define TEST_MIDPOINTPOOL               0       // OK
#define TEST_COMPRESS                   0       // OK
#define TEST_CLEAR                      0       // OK
#define TEST_MASK_RECTANGE              0       // OK
#define TEST_MASK_CIRCLE                0       // OK
#define TEST_MASK_ELLIPSE               0       // OK
#define TEST_BINARY                     0       // OK
#define TEST_INVERT                     0       // OK
#define TEST_B_AND                      0       // OK
#define TEST_B_NAND                     0       // OK
#define TEST_B_OR                       0       // OK
#define TEST_B_NOR                      0       // OK
#define TEST_B_XOR                      0       // OK
#define TEST_B_XNOR                     0       // OK
#define TEST_AWB                        0       // NOT OK
#define TEST_CCM                        0       // OK
#define TEST_GAMMA                      0       // OK
#define TEST_NEGATE                     0       // OK
#define TEST_REPLACE                    0       // OK
#define TEST_ADD                        0       // OK
#define TEST_SUB                        0       // OK
#define TEST_MUL                        0       // OK
#define TEST_DIV                        0       // OK
#define TEST_MIN                        0       // OK
#define TEST_MAX                        0       // OK
#define TEST_DIFFERENCE                 0       // OK
#define TEST_BLEND                      0       // OK
#define TEST_HISTEQ                     0       // OK
#define TEST_MEAN                       0       // OK
#define TEST_MEDIAN                     0       // OK
#define TEST_MODE                       0       // OK
#define TEST_MIDPOINT                   0       // OK   FIXME: valgrind check error：Invalid read of size 1
#define TEST_MORPH                      0       // OK
#define TEST_GAUSSIAN                   0       // OK
#define TEST_LAPLACIAN                  0       // OK
#define TEST_BILATERAL                  0       // OK
#define TEST_LINPOLAR                   0       // OK
#define TEST_LOGPOLAR                   0       // OK
#define TEST_LENS_COOR                  0       // NOT OK
#define TEST_ROTATION_COOR              0       // NOT OK
#define TEST_GET_HISTOGRAM              0       // OK
#define TEST_GET_STATISTICS             0       // OK
#define TEST_GET_REGRESSION             0       // OK
#define TEST_FLOOD_FILL                 0       // OK
#define TEST_ERODE                      0       // OK
#define TEST_DILATE                     0       // OK
#define TEST_OPEN                       0       // OK
#define TEST_CLOSE                      0       // OK
#define TEST_TOP_HAT                    0       // OK
#define TEST_BLACK_HAT                  0       // OK
#define TEST_FIND_BLOBS                 0       // OK
#define TEST_FIND_LINES                 0       // OK
#define TEST_FIND_LINE_SEGMENTS         0       // OK
#define TEST_FIND_CIRCLES               0       // OK
#define TEST_FIND_RECTS                 0       // OK
#define TEST_FIND_QRCODES               0       // OK
#define TEST_FIND_APRILTAGS             1       // OK
#define TEST_FIND_DATAMATRICES          0       // OK
#define TEST_FIND_BARCODES              0       // OK
#define TEST_FIND_DISPLACEMENT          0       // NOT OK
#define TEST_FIND_TEMPLATE              0       // OK
#define TEST_FIND_FEATURES              0       // NOT OK
#define TEST_FIND_LBP                   0       // NOT OK
#define TEST_FIND_KEYPOINTS             0       // NOT OK
#define TEST_FIND_EDGES                 0       // OK
#define TEST_FIND_HOG                   0       // NOT OK
#define TEST_STERO_DISPARITY            0       // NOT OK

void print_image(image::Image &img);
const char *test_640x480_png = "test.jpg";

int _main(int argc, char* argv[])
{
    uint64_t __attribute__((unused)) start_time;
    int a = 0;
    int b = 20;
    printf("%d", b / a);
    // 1. Create camera and display object
#if CAMERA_ENABLE
    camera::Camera cam = camera::Camera();
#else
    image::Image * __attribute__((unused)) test_cam_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
#endif

#if DISPLAY_ENABLE
    display::Display disp = display::Display();
#endif

    /* Test image::Image::mean_pool */
#if TEST_MEANPOOL
    {
#if 1
        {
            image::Image *img_copy;
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mean_pool(2, 2);
            log::info("mean pool gray image cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_mean_pool.png");

            start_time = time::time_us();
            img_copy = img->mean_pool(2, 2, true);
            log::info("mean pool gray image copy cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_mean_pool_copy.png");

            delete img_copy;
            delete img;
        }
#endif

#if 1
        {
            image::Image *img, *img_copy;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);

            img = src_img->copy();
            // img = src_img->resize(6, 4);
            // print_image(*img);
            start_time = time::time_us();
            img->mean_pool(2, 2);
            log::info("mean pool rgb888 image cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_mean_pool.png");
            // print_image(*img);
            delete img;

            img = src_img->copy();
            // img = src_img->resize(6, 4);
            // print_image(*img);
            start_time = time::time_us();
            img_copy = img->mean_pool(2, 2, true);
            log::info("mean pool rgb888 image copy cost %d us\r\n", (int)(time::time_us() - start_time));
            img_copy->save("out_rgb888_640x480_mean_pool_copy.png");
            // print_image(*img_copy);
            delete img;
            delete img_copy;
            delete src_img;
        }
#endif
    }
#endif

    /* Test image::Image::midpoint_pool */
#if TEST_MIDPOINTPOOL
    {
#if 1
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->midpoint_pool(2, 2, 0.5);
            log::info("midpoint pool gray image cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_midpoint_pool.png");
            delete img;
        }
#endif

#if 1
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->midpoint_pool(2, 2, 0.5);
            log::info("midpoint pool rgb888 image cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_midpoint_pool.png");
            delete img;
        }
#endif
    }
#endif

    /* Test compress */
#if TEST_COMPRESS
    {
        fs::File f = fs::File();
#if 1
        {
            image::Image *jpg;
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            jpg = img->compress();
            log::info("compress gray image cost %d us\r\n", (int)(time::time_us() - start_time));

            f.open("./out_gray_640x480_compress.jpg", "w+");
            f.write(jpg->data(), jpg->data_size());
            f.close();

            delete img;
            delete jpg;
        }
#endif

#if 1
        {
            image::Image *jpg;
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            jpg = img->compress();
            log::info("compress rgb888 image cost %d us\r\n", (int)(time::time_us() - start_time));

            f.open("./out_rgb888_640x480_compress.jpg", "w+");
            f.write(jpg->data(), jpg->data_size());
            f.close();

            delete img;
            delete jpg;
        }
#endif
    }
#endif

#if TEST_CLEAR
    {
#if 1
        {
            image::Image *img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            start_time = time::time_us();
            img->clear();
            log::info("clear gray image cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_clear.png");
            delete img;

            img = src_img->copy();
            start_time = time::time_us();
            img->clear(&mask_img);
            log::info("clear gray image with mask cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_clear_mask.png");
            delete img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            start_time = time::time_us();
            img->clear();
            log::info("clear rgb888 image cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_clear.png");
            delete img;

            img = src_img->copy();
            start_time = time::time_us();
            img->clear(&mask_img);
            log::info("clear rgb888 image with mask cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_clear_mask.png");
            delete img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_MASK_RECTANGE
    {
        image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
        start_time = time::time_us();
        img->mask_rectange();
        log::info("img.mask_rectange() cost %d us\r\n", (int)(time::time_us() - start_time));
        img->save("out_gray_640x480_mask_rectange.png");
        delete img;
    }

    {
        image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
        start_time = time::time_us();
        img->mask_rectange();
        log::info("img.mask_rectange() cost %d us\r\n", (int)(time::time_us() - start_time));
        img->save("out_rgb888_640x480_mask_rectange.png");
        delete img;
    }
#endif

#if TEST_MASK_CIRCLE
    {
        image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
        start_time = time::time_us();
        img->mask_circle();
        log::info("img.mask_circle() cost %d us\r\n", (int)(time::time_us() - start_time));
        img->save("out_gray_640x480_mask_circle.png");
        delete img;
    }

    {
        image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
        start_time = time::time_us();
        img->mask_circle();
        log::info("img.mask_circle() cost %d us\r\n", (int)(time::time_us() - start_time));
        img->save("out_rgb888_640x480_mask_circle.png");
        delete img;
    }
#endif

#if TEST_MASK_ELLIPSE
    {
        image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
        start_time = time::time_us();
        img->mask_ellipse();
        log::info("img.mask_ellipse() cost %d us\r\n", (int)(time::time_us() - start_time));
        img->save("out_gray_640x480_mask_ellipse.png");
        delete img;
    }

    {
        image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
        start_time = time::time_us();
        img->mask_ellipse();
        log::info("img.mask_ellipse() cost %d us\r\n", (int)(time::time_us() - start_time));
        img->save("out_rgb888_640x480_mask_ellipse.png");
        delete img;
    }
#endif

#if TEST_BINARY
    {
#if 0
        {
            image::Image *img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            std::vector<std::vector<int>> thresholds = {{13, 24}};

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            start_time = time::time_us();
            img->binary(thresholds);
            log::info("gray img->binary(thresholds) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_binary.png");
            delete img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            start_time = time::time_us();
            img->binary(thresholds, false, false, &mask_img);
            log::info("gray img->binary(thresholds, false, false, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_binary_mask.png");
            delete img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            std::vector<std::vector<int>> thresholds = {{0, 100, 20, 80, 20, 80}};

            // img = src_img->copy();
            // cv::Mat bgrImage(img->height(), img->width(), CV_8UC((int)image::fmt_size[img->format()]), img->data());
            // cv::Mat labImage;
            // start_time = time::time_us();
            // cv::cvtColor(bgrImage, labImage, cv::COLOR_BGR2Lab);
            // log::info("rgb888 cv::cvtColor(bgrImage, labImage, cv::COLOR_BGR2Lab) cost %d us\r\n", (int)(time::time_us() - start_time));
            // // cv::threshold(mat, binaryImage, 128, 255, cv2.THRESH_BINARY);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            start_time = time::time_us();
            img->binary(thresholds);
            log::info("rgb888 img->binary(thresholds) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_binary.png");
            delete img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            start_time = time::time_us();
            img->binary(thresholds, false, false, &mask_img);
            log::info("rgb888 img->binary(thresholds, false, false, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_binary_mask.png");
            delete img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_INVERT
    {
#if 1
        {
            image::Image *img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);

            // img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            // print_image(*img);
            img = src_img->copy();
            start_time = time::time_us();
            img->invert();
            log::info("gray img->invert() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_invert.png");
            // print_image(*img);

            delete img;
            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);

            // img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            // print_image(*img);
            img = src_img->copy();
            start_time = time::time_us();
            img->invert();
            log::info("rgb888 img->invert() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_invert.png");
            // print_image(*img);

            delete img;
            delete src_img;
        }
#endif
    }
#endif

#if TEST_B_AND
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_and(other_img);
            log::info("gray img->b_and(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_and.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_and(other_img, &mask_img);
            log::info("gray img->b_and(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_and.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_and(other_img);
            log::info("rgb888 img->b_and(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_and.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_and(other_img, &mask_img);
            log::info("rgb888 img->b_and(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_and_mask.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_B_NAND
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nand(other_img);
            log::info("gray img->b_nand(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_nand.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nand(other_img, &mask_img);
            log::info("gray img->b_nand(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_nand.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nand(other_img);
            log::info("rgb888 img->b_nand(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_nand.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nand(other_img, &mask_img);
            log::info("rgb888 img->b_nand(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_nand_mask.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_B_OR
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_or(other_img);
            log::info("gray img->b_or(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_or.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_or(other_img, &mask_img);
            log::info("gray img->b_or(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_or.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_or(other_img);
            log::info("rgb888 img->b_or(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_or.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_or(other_img, &mask_img);
            log::info("rgb888 img->b_or(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_or_mask.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_B_NOR
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nor(other_img);
            log::info("gray img->b_nor(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_nor.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nor(other_img, &mask_img);
            log::info("gray img->b_nor(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_nor.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nor(other_img);
            log::info("rgb888 img->b_nor(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_nor.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_nor(other_img, &mask_img);
            log::info("rgb888 img->b_nor(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_nor_mask.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_B_XOR
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xor(other_img);
            log::info("gray img->b_xor(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_xor.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xor(other_img, &mask_img);
            log::info("gray img->b_xor(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_xor.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xor(other_img);
            log::info("rgb888 img->b_xor(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_xor.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xor(other_img, &mask_img);
            log::info("rgb888 img->b_xor(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_xor_mask.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_B_XNOR
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xnor(other_img);
            log::info("gray img->b_xnor(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_xnor.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xnor(other_img, &mask_img);
            log::info("gray img->b_xnor(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_b_xnor.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xnor(other_img);
            log::info("rgb888 img->b_xnor(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_xnor.png");
            print_image(*img);
            delete img;
            delete other_img;

            img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            other_img = src_img->resize(INPUT_IMG_W, INPUT_IMG_H);
            print_image(*img);

            start_time = time::time_us();
            img->b_xnor(other_img, &mask_img);
            log::info("rgb888 img->b_xnor(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_b_xnor_mask.png");
            print_image(*img);
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_AWB
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->awb();
            log::info("img.awb() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_awb.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->awb(true);
            log::info("img.mode(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_awb_max.png");
            delete img;
        }
    }
#endif

#if TEST_CCM
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            std::vector<float> color_matrix_3x3= {
                1, 0, 0.5,
                0, 1, 0.5,
                0, 0, 0
            };
            img->ccm(color_matrix_3x3);
            log::info("img.ccm(color_matrix_3x3) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_ccm_3x3.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            std::vector<float> color_matrix_4x3= {
                1, 0, 0.5,
                0, 1, 0.5,
                0, 0, 0,
                -50, 20, 20
            };
            img->ccm(color_matrix_4x3);
            log::info("img.ccm(color_matrix_4x3) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_ccm_4x3.png");
            delete img;
        }
    }
#endif

#if TEST_GAMMA
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->gamma(0.2);
            log::info("img.gamma(0.2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gamma_0.2.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->gamma(0.2, 2);
            log::info("img.gamma(0.2, 2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gamma0.2_contrast2.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->gamma(0.2, 2, 0.8);
            log::info("img.gamma(0.2, 2, 0.8) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gamma0.2_contrast2_brightness0.8.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->gamma(0.2);
            log::info("img.gamma(0.2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gamma_0.2.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->gamma(0.2, 2);
            log::info("img.gamma(0.2, 2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gamma0.2_contrast2.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->gamma(0.2, 2, 0.8);
            log::info("img.gamma(0.2, 2, 0.8) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gamma0.2_contrast2_brightness0.8.png");
            delete img;
        }
    }
#endif

#if TEST_NEGATE
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->negate();
            log::info("img.negate cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_negate.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->negate();
            log::info("img.negate() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_negate.png");
            delete img;
        }
    }
#endif

#if TEST_REPLACE
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image src_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            src_img.draw_rect(0, 0, src_img.width(), src_img.height(), 255, -1);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            delete img;

            // hmirror without other image
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->replace(NULL, true);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_replace_hmirror_without_other.png");
            delete img;

            // hmirror
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            src_img.replace(img, true);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_gray_640x480_replace_hmirror.png");
            delete img;

            // vflip
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            src_img.replace(img, false, true);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_gray_640x480_replace_vflip.png");
            delete img;

            // flip=false, hmirror=false, transpose=false
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            src_img.replace(img);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_gray_640x480_replace_rot0.png");
            delete img;

            // flip=true, hmirror=true, transpose=false
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            src_img.replace(img, true, true);
            log::info("img.replace(img, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_gray_640x480_replace_rot180.png");
            delete img;

            // flip=true, hmirror=false, transpose=true
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            src_img.replace(img, true, false, true, &mask_img);
            log::info("img.replace(img, true, false, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_gray_640x480_replace_rot90.png");
            delete img;

            // flip=false, hmirror=true, transpose=true
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image src_img2(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            src_img2.replace(img, false, true, true, &mask_img);
            log::info("img.replace(img, false, true, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img2.save("out_gray_640x480_replace_rot270.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image src_img(img->width(), img->height(), image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            src_img.draw_rect(0, 0, src_img.width(), src_img.height(), 255, -1);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            delete img;

            // hmirror without other image
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->replace(NULL, true);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_replace_hmirror_without_other.png");
            delete img;

            // hmirror
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            src_img.replace(img, true);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_rgb888_640x480_replace_hmirror.png");
            delete img;

            // vflip
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            src_img.replace(img, false, true);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_rgb888_640x480_replace_vflip.png");
            delete img;

            // flip=false, hmirror=false, transpose=false
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            src_img.replace(img);
            log::info("img.replace(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_rgb888_640x480_replace_rot0.png");
            delete img;

            // flip=true, hmirror=true, transpose=false
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            src_img.replace(img, true, true);
            log::info("img.replace(img, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_rgb888_640x480_replace_rot180.png");
            delete img;

            // flip=true, hmirror=false, transpose=true
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            src_img.replace(img, true, false, true, &mask_img);
            log::info("img.replace(img, true, false, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img.save("out_rgb888_640x480_replace_rot90.png");
            delete img;

            // flip=false, hmirror=true, transpose=true
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image src_img2(img->width(), img->height(), image::Format::FMT_RGB888);
            start_time = time::time_us();
            src_img2.replace(img, false, true, true, &mask_img);
            log::info("img.replace(img, false, true, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            src_img2.save("out_rgb888_640x480_replace_rot270.png");
            delete img;
        }
    }
#endif

#if TEST_ADD
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->add(other_img);
            log::info("gray img->add(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_add.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->add(other_img, &mask_img);
            log::info("gray img->add(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_add_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->add(other_img);
            log::info("rgb888 img->add(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_add.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->add(other_img, &mask_img);
            log::info("rgb888 img->add(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_add_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_SUB
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->sub(other_img);
            log::info("gray img->sub(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_sub.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->sub(other_img, true, &mask_img);
            log::info("gray img->sub(other_img, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_sub_true_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->sub(other_img);
            log::info("rgb888 img->sub(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_sub.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->sub(other_img, true, &mask_img);
            log::info("rgb888 img->sub(other_img, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_sub_true_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_MUL
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->mul(other_img);
            log::info("gray img->mul(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_mul.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->mul(other_img, true, &mask_img);
            log::info("gray img->mul(other_img, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_mul_true_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->mul(other_img);
            log::info("rgb888 img->mul(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_mul.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->mul(other_img, true, &mask_img);
            log::info("rgb888 img->mul(other_img, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_mul_true_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_DIV
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->div(other_img);
            log::info("gray img->div(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_div.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->div(other_img, false, true, &mask_img);
            log::info("gray img->div(other_img, false, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_div_false_true_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->div(other_img);
            log::info("rgb888 img->div(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_div.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->div(other_img, false, true, &mask_img);
            log::info("rgb888 img->div(other_img, false, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_div_false_true_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_MIN
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->min(other_img);
            log::info("gray img->min(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_min.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->min(other_img, &mask_img);
            log::info("gray img->min(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_min_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->min(other_img);
            log::info("rgb888 img->min(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_min.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->min(other_img, &mask_img);
            log::info("rgb888 img->min(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_min_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_MAX
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->max(other_img);
            log::info("gray img->max(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_max.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->max(other_img, &mask_img);
            log::info("gray img->max(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_max_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->max(other_img);
            log::info("rgb888 img->max(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_max.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->max(other_img, &mask_img);
            log::info("rgb888 img->max(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_max_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_DIFFERENCE
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->difference(other_img);
            log::info("gray img->difference(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_difference.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->difference(other_img, &mask_img);
            log::info("gray img->difference(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_difference_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->difference(other_img);
            log::info("rgb888 img->difference(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_difference.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->difference(other_img, &mask_img);
            log::info("rgb888 img->difference(other_img, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_difference_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_BLEND
    {
#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->blend(other_img);
            log::info("gray img->blend(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_blend.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->blend(other_img, 64, &mask_img);
            log::info("gray img->blend(other_img, 64, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_blend_64_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif

#if 1
        {
            image::Image *img, *other_img;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->blend(other_img);
            log::info("rgb888 img->blend(other_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_blend.png");
            delete img;
            delete other_img;

            img = src_img->copy();
            other_img = src_img->copy();
            start_time = time::time_us();
            img->blend(other_img, 64, &mask_img);
            log::info("rgb888 img->blend(other_img, 64, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_blend_64_mask.png");
            delete img;
            delete other_img;

            delete src_img;
        }
#endif
    }
#endif

#if TEST_HISTEQ
    {
        {
            image::Image *gray_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            gray_img->histeq();
            log::info("gray_img.histeq() cost %d us\r\n", (int)(time::time_us() - start_time));
            gray_img->save("out_gray_640x480_histeq.png");
            delete gray_img;

            gray_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            gray_img->histeq(true);
            log::info("gray_img.histeq(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            gray_img->save("out_gray_640x480_histeq_adaptive.png");
            delete gray_img;

            gray_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            gray_img->histeq(false, 10);
            log::info("gray_img.histeq(true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            gray_img->save("out_gray_640x480_histeq_clip_limit_10.png");
            delete gray_img;

            gray_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img = image::Image(gray_img->width(), gray_img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(0, 0, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            gray_img->histeq(false, 10, &mask_img);
            log::info("gray_img.histeq(true, 10, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            gray_img->save("out_gray_640x480_histeq_mask.png");
            delete gray_img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->histeq();
            log::info("img.histeq() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_histeq.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->histeq(true);
            log::info("img.histeq(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_histeq_adaptive.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->histeq(false, 10);
            log::info("img.histeq(true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_histeq_clip_limit_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(0, 0, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->histeq(false, 10, &mask_img);
            log::info("img.histeq(true, 10, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_histeq_mask.png");
            delete img;
        }
    }
#endif

#if TEST_MEAN
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mean(2);
            log::info("img.mean() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mean.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mean(2, true);
            log::info("img.mean(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mean_thr_true.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mean(2, true, 10);
            log::info("img.mean(true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mean_oft_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mean(2, true, -10);
            log::info("img.mean(true, -10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mean_oft_neg_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(0, 0, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->mean(2, true, 0, true, &mask_img);
            log::info("img.mean(true, 10, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mean_invert_and_mask.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mean(2);
            log::info("img.mean() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mean.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mean(2, true);
            log::info("img.mean(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mean_thr_true.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mean(2, true, 10);
            log::info("img.mean(true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mean_oft_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mean(2, true, -10);
            log::info("img.mean(true, -10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mean_oft_neg_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(0, 0, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->mean(2, true, 0, true, &mask_img);
            log::info("img.mean(true, 10, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mean_invert_and_mask.png");
            delete img;
        }
    }
#endif

#if TEST_MEDIAN
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->median(2);
            log::info("img.median(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->median(2, 0.1);
            log::info("img.median(2, 0.1) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median_1.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->median(2, 0.9);
            log::info("img.median(2, 0.9) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median_9.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->median(2, 0.5, true);
            log::info("img.median(2, 0.5, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median_thr_en.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->median(2, 0.5, true, 10);
            log::info("img.median(2, 0.5, true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median_oft_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->median(2, 0.5, true, -10);
            log::info("img.median(2, 0.5, true, -10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median_oft_neg_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->median(2, 0.5, true, 0, true, &mask_img);
            log::info("img.median(2, 0.5, true, 0, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_median_invert_and_mask.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->median(2);
            log::info("img.median(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->median(2, 0.1);
            log::info("img.median(2, 0.1) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median_1.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->median(2, 0.9);
            log::info("img.median(2, 0.9) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median_9.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->median(2, 0.5, true);
            log::info("img.median(2, 0.5, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median_thr_en.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->median(2, 0.5, true, 10);
            log::info("img.median(2, 0.5, true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median_oft_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->median(2, 0.5, true, -10);
            log::info("img.median(2, 0.5, true, -10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median_oft_neg_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->median(2, 0.5, true, 0, true, &mask_img);
            log::info("img.median(2, 0.5, true, 0, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_median_invert_and_mask.png");
            delete img;
        }
    }
#endif

#if TEST_MODE
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mode(2);
            log::info("img.mode(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mode.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mode(2, true);
            log::info("img.mode(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mode_thr_en.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mode(2, true, 10);
            log::info("img.mode(2, true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mode_oft_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->mode(2, true, -10);
            log::info("img.mode(2, true, -10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mode_oft_neg_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->mode(2, true, 0, true, &mask_img);
            log::info("img.mode(2, true, 0, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_64x64_mode_invert_and_mask.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mode(2);
            log::info("img.mode(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mode.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mode(2, true);
            log::info("img.mode(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mode_thr_en.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mode(2, true, 10);
            log::info("img.mode(2, true, 10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mode_oft_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->mode(2, true, -10);
            log::info("img.mode(2, true, -10) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mode_oft_neg_10.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->mode(2, true, 0, true, &mask_img);
            log::info("img.mode(2, true, 0, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_64x64_mode_invert_and_mask.png");
            delete img;
        }
    }
#endif

#if TEST_MIDPOINT
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->midpoint(2);
            log::info("img.midpoint(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_midpoint.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->midpoint(2, 0.2);
            log::info("img.midpoint(2, 0.2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_midpoint_bias0.2.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->midpoint(2, 0.2, true);
            log::info("img.midpoint(2, 0.2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_midpoint_bias0.2_thr.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->midpoint(2, 0.2, true, -20);
            log::info("img.midpoint(2, 0.2, true, -20) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_midpoint_bias0.2_thr_oft20.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->midpoint(2, 0.2, true, -20, true, &mask_img);
            log::info("img.midpoint(2, 0.2, true, -20, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_midpoint_bias0.2_thr_oft20_invert.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            start_time = time::time_us();
            img->midpoint(2);
            log::info("img.midpoint(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_midpoint.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->midpoint(2, 0.2);
            log::info("img.midpoint(2, 0.2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_midpoint_bias0.2.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->midpoint(2, 0.2, true);
            log::info("img.midpoint(2, 0.2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_midpoint_bias0.2_thr.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->midpoint(2, 0.2, true, -20);
            log::info("img.midpoint(2, 0.2, true, -20) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_midpoint_bias0.2_thr_oft20.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->midpoint(2, 0.2, true, -20, true, &mask_img);
            log::info("img.midpoint(2, 0.2, true, -20, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_midpoint_bias0.2_thr_oft20_invert.png");
            delete img;
        }
    }
#endif

#if TEST_MORPH
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            std::vector<int> kernel = { -1, -2, -1,
                                        -2, 12, -2,
                                        -1, -2, -1};

            start_time = time::time_us();
            img->morph(1, kernel);
            log::info("img.morph(1, kernel) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_morph_kernel3x3.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->morph(1, kernel, 200);
            log::info("img.morph(1, kernel, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_morph_kernel3x3_mul200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->morph(1, kernel, 200, 200);
            log::info("img.morph(1, kernel, 200, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_morph_kernel3x3_mul200_add20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->morph(1, kernel, 200, 200, true);
            log::info("img.morph(1, kernel, 200, 200, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_morph_kernel3x3_mul200_add200_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->morph(1, kernel, 200, 200, true, 20);
            log::info("img.morph(1, kernel, 200, 200, true, 20) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_morph_kernel3x3_mul200_add200_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->morph(1, kernel, 200, 200, true, 20, true, &mask_img);
            log::info("img.morph(1, kernel, 200, 200, true, 20, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_morph_kernel3x3_mul200_add200_thr_oft20_invert.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            std::vector<int> kernel = { -1, -2, -1,
                                        -2, 12, -2,
                                        -1, -2, -1};

            start_time = time::time_us();
            img->morph(1, kernel);
            log::info("img.morph(1, kernel) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_morph_kernel3x3.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->morph(1, kernel, 200);
            log::info("img.morph(1, kernel, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_morph_kernel3x3_mul200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->morph(1, kernel, 200, 200);
            log::info("img.morph(1, kernel, 200, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_morph_kernel3x3_mul200_add20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->morph(1, kernel, 200, 200, true);
            log::info("img.morph(1, kernel, 200, 200, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_morph_kernel3x3_mul200_add200_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->morph(1, kernel, 200, 200, true, 20);
            log::info("img.morph(1, kernel, 200, 200, true, 20) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_morph_kernel3x3_mul200_add200_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->morph(1, kernel, 200, 200, true, 20, true, &mask_img);
            log::info("img.morph(1, kernel, 200, 200, true, 20, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_morph_kernel3x3_mul200_add200_thr_oft20_invert.png");
            delete img;
        }
    }
#endif

#if TEST_GAUSSIAN
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->gaussian(2);
            log::info("img.gaussian(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->gaussian(2, true);
            log::info("img.gaussian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2_unsharp.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->gaussian(2, true, 0.005);
            log::info("img.gaussian(2, true, 0.005) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2_unsharp_mul200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->gaussian(2, true, 0.005, 200);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2_unsharp_mul200_add200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->gaussian(2, true, 0.005, 200, true);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2_unsharp_mul200_add200_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->gaussian(2, true, 0.005, 200, true, 20);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2_unsharp_mul200_add200_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->gaussian(2, true, 0.005, 200, true, 20, true, &mask_img);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_gaussian_size2_unsharp_mul200_add200_thr_oft20_invert.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->gaussian(1);
            log::info("img.gaussian(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->gaussian(2, true);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2_unsharp.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->gaussian(2, true, 0.005);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2_unsharp_mul200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->gaussian(2, true, 0.005, 200);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2_unsharp_mul200_add200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->gaussian(2, true, 0.005, 200, true);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2_unsharp_mul200_add200_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->gaussian(2, true, 0.005, 200, true, 20);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2_unsharp_mul200_add200_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->gaussian(2, true, 0.005, 200, true, 20, true, &mask_img);
            log::info("img.gaussian(2, 200) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_gaussian_size2_unsharp_mul200_add200_thr_oft20_invert.png");
            delete img;
        }
    }
#endif

#if TEST_LAPLACIAN
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->laplacian(2);
            log::info("img.laplacian(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->laplacian(2, true);
            log::info("img.laplacian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2_sharpen.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->laplacian(2, true, 0.005);
            log::info("img.laplacian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2_sharpen_mul200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->laplacian(2, true, 0.005, 200);
            log::info("img.laplacian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2_sharpen_mul200_add200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->laplacian(2, true, 0.005, 200, true);
            log::info("img.laplacian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2_sharpen_mul200_add200_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->laplacian(2, true, 0.005, 200, true, 20);
            log::info("img.laplacian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2_sharpen_mul200_add200_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->laplacian(2, true, 0.005, 200, true, 20, true, &mask_img);
            log::info("img.laplacian(2, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_laplacian_size2_sharpen_mul200_add200_thr_oft20_invert.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->laplacian(1);
            log::info("img.laplacian(1) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->laplacian(1, true);
            log::info("img.laplacian(1, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2_sharpen.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->laplacian(1, true, 200);
            log::info("img.laplacian(1, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2_sharpen_mul200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->laplacian(1, true, 200, 200);
            log::info("img.laplacian(1, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2_sharpen_mul200_add200.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->laplacian(1, true, 200, 200, true);
            log::info("img.laplacian(1, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2_sharpen_mul200_add200_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->laplacian(1, true, 200, 200, true, 20);
            log::info("img.laplacian(1, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2_sharpen_mul200_add200_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->laplacian(1, true, 200, 200, true, 20, true, &mask_img);
            log::info("img.laplacian(1, true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_laplacian_size2_sharpen_mul200_add200_thr_oft20_invert.png");
            delete img;
        }
    }
#endif

#if TEST_BILATERAL
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->bilateral(2);
            log::info("img.bilateral(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_bilateral_size2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->bilateral(1, 0.5);
            log::info("img.bilateral(1, 0.5) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_bilateral_size2_color0.5.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->bilateral(1, 0.5, 0.1);
            log::info("img.bilateral(1, 0.5, 0.1) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_bilateral_size2_color0.1_sigma0.1.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->bilateral(1, 0.1, 1, true);
            log::info("img.bilateral(1, 0.1, 1, true)) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_bilateral_size2_color0.1_sigma1_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->bilateral(1, 0.1, 1, true, 5);
            log::info("img.bilateral(1, 0.1, 1, true, 5) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_bilateral_size2_color0.1_sigma1_thr_oft20.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->bilateral(1, 0.1, 1, true, 5, true, &mask_img);
            log::info("img.bilateral(1, 0.1, 1, true, 5, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_bilateral_size2_color0.1_sigma1_thr_oft20_invert.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->bilateral(2);
            log::info("img.bilateral(2) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_bilateral_size2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->bilateral(1, 0.5);
            log::info("img.bilateral(1, 0.5) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_bilateral_size2_color0.5.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->bilateral(1, 0.5, 0.1);
            log::info("img.bilateral(1, 0.5, 0.1) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_bilateral_size2_color0.5_sigma0.1.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->bilateral(1, 0.1, 1, true);
            log::info("img.bilateral(1, 0.1, 1, true)) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_bilateral_size2_color0.1_sigma1_thr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->bilateral(1, 0.1, 1, true, 5);
            log::info("img.bilateral(1, 0.1, 1, true, 5) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_bilateral_size2_color0.1_sigma1_thr_oft5.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->bilateral(1, 0.1, 1, true, 5, true, &mask_img);
            log::info("img.bilateral(1, 0.1, 1, true, 5, true, &mask_img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_bilateral_size2_color0.1_sigma1_thr_oft5_invert.png");
            delete img;
        }
    }
#endif

#if TEST_LINPOLAR
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);

            start_time = time::time_us();
            img->linpolar();
            log::info("img.linpolar() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_linpolar.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->linpolar(true);
            log::info("img.linpolar(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_linpolar_reverse.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->linpolar();
            log::info("img.linpolar() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_linpolar.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            start_time = time::time_us();
            img->linpolar(true);
            log::info("img.linpolar(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_linpolar_reverse.png");
            delete img;
        }
    }
#endif

#if TEST_LOGPOLAR
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->logpolar();
            log::info("img.logpolar() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_logpolar.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->logpolar(true);
            log::info("img.logpolar(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_logpolar_reverse.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->logpolar();
            log::info("img.logpolar() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_logpolar.png");
            delete img;

            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            start_time = time::time_us();
            img->logpolar(true);
            log::info("img.logpolar(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_logpolar_reverse.png");
            delete img;
        }
    }
#endif

#if TEST_LENS_COOR
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->lens_corr();
            log::info("img.lens_corr() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_lens_corr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->lens_corr(2.0);
            log::info("img.lens_corr(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_lens_corr_strength2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->lens_corr(2.0, 0.5);
            log::info("img.lens_corr(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_lens_corr_strength2_zoom0.5.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            img->lens_corr(2.0, 0.5, 1, 1);
            log::info("img.lens_corr(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_lens_corr_strength2_zoom0.5_x1,y1.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            start_time = time::time_us();
            img->lens_corr();
            log::info("img.lens_corr() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_lens_corr.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->lens_corr(2.0);
            log::info("img.lens_corr(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_lens_corr_strength2.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->lens_corr(2.0, 0.5);
            log::info("img.lens_corr(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_lens_corr_strength2_zoom0.5.png");
            delete img;

            start_time = time::time_us();
            img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            img->lens_corr(2.0, 0.5, 1, 1);
            log::info("img.lens_corr(true) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_lens_corr_strength2_zoom0.5_x1,y1.png");
            delete img;
        }
    }
#endif

#if TEST_ROTATION_COOR
    {
        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            double x_rot = 0;
            double y_rot = 0;
            double z_rot = 0;
            double x_trans = 0;
            double y_trans = 0;
            double zoom = 1;
            double fov = 60;
            std::vector<float> corners = {};

            start_time = time::time_us();
            img->rotation_corr();
            log::info("img.rotation_corr() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_rotation_corr.png");
            delete img;

            start_time = time::time_us();
            img->rotation_corr(x_rot, y_rot, z_rot, x_trans, y_trans, zoom, fov, corners);
            log::info("img.rotation_corr(x_rot, y_rot, z_rot, x_trans, y_trans) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_rotation_corr_input.png");
            delete img;
        }

        {
            image::Image *img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(img->width(), img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);
            double x_rot = 0;
            double y_rot = 0;
            double z_rot = 0;
            double x_trans = 0;
            double y_trans = 0;
            double zoom = 1;
            double fov = 60;
            std::vector<float> corners = {};

            start_time = time::time_us();
            img->rotation_corr();
            log::info("img.rotation_corr() cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_rotation_corr.png");
            delete img;

            start_time = time::time_us();
            img->rotation_corr(x_rot, y_rot, z_rot, x_trans, y_trans, zoom, fov, corners);
            log::info("img.rotation_corr(x_rot, y_rot, z_rot, x_trans, y_trans) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_rotation_corr_input.png");
            delete img;
        }
    }
#endif

#if TEST_GET_HISTOGRAM
    {
#if 0
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{100, 200}};
                bool invert = false;
                int bins = 256;
                int l_bins = 256;
                int a_bins = 256;
                int b_bins = 256;
                image::Image difference = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
                difference.draw_rect(0, 0, difference.width(), difference.height(), 255, -1);

                std::map<std::string, std::vector<float>> hist;
                start_time = time::time_us();
                hist = img->get_histogram(thresholds, invert, roi, bins, l_bins, a_bins, b_bins, NULL);
                log::info("gray get histogram cost %d us\r\n", (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                {
                    int start_x = 0, start_y = 0, hist_max_w = 512, hist_max_h = 200;
                    int rect_w = hist_max_w / bins;
                    std::vector<float> data = hist["L"];
                    for (int i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(255, 0, 0), -1);
                    }
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{0, 100}};
                bool invert = false;
                int bins = 256;
                int l_bins = 100;
                int a_bins = 256;
                int b_bins = 256;
                image::Image difference = image::Image(img->width(), img->height(), image::Format::FMT_GRAYSCALE);
                difference.draw_rect(0, 0, difference.width(), difference.height(), 255, -1);

                std::map<std::string, std::vector<float>> hist;
                start_time = time::time_us();
                hist = img->get_histogram(thresholds, invert, roi, bins, l_bins, a_bins, b_bins, NULL);
                log::info("rgb888 get histogram cost %d us\r\n", (int)(time::time_us() - start_time));

                // Process results
                {
                    int start_x = 0, start_y = 0, rect_w = 2, hist_max_h = 200;
                    std::vector<float> data = hist["L"];
                    for (size_t i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(255, 0, 0), -1);
                    }
                }

                {
                    int start_x = 0, start_y = 100, rect_w = 2, hist_max_h = 200;
                    std::vector<float> data = hist["A"];
                    for (size_t i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(0, 255, 0), -1);
                    }
                }

                {
                    int start_x = 0, start_y = 200, rect_w = 2, hist_max_h = 200;
                    std::vector<float> data = hist["B"];
                    for (size_t i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(0, 0, 255), -1);
                    }
                }


                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_GET_STATISTICS
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{0, 100}};
                bool invert = false;
                int bins = 256;
                int l_bins = 256;
                int a_bins = 256;
                int b_bins = 256;

                image::Statistics statistics;
                start_time = time::time_us();
                statistics = img->get_statistics(thresholds, invert, roi, bins, l_bins, a_bins, b_bins, NULL);
                log::info("gray get statistics cost %d us\r\n", (int)(time::time_us() - start_time));

                std::map<std::string, std::vector<float>> hist;
                hist = img->get_histogram(thresholds, invert, roi, bins, l_bins, a_bins, b_bins, NULL);
                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                {
                    int start_x = 0, start_y = 0, hist_max_w = 512, hist_max_h = 200;
                    int rect_w = hist_max_w / bins;
                    std::vector<float> data = hist["L"];
                    for (int i = 0; i < (int)data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(255, 0, 0), -1);
                    }
                }
                log::info("L: mean %d, median %d, mode %d, std_dev %d, min %d, max %d, lq %d, uq %d\r\n",
                            statistics.l_mean(), statistics.l_median(), statistics.l_mode(), statistics.l_std_dev(),
                            statistics.l_min(), statistics.l_max(), statistics.l_lq(), statistics.l_uq());
                log::info("A: mean %d, median %d, mode %d, std_dev %d, min %d, max %d, lq %d, uq %d\r\n",
                            statistics.a_mean(), statistics.a_median(), statistics.a_mode(), statistics.a_std_dev(),
                            statistics.a_min(), statistics.a_max(), statistics.a_lq(), statistics.a_uq());
                log::info("B: mean %d, median %d, mode %d, std_dev %d, min %d, max %d, lq %d, uq %d\r\n",
                            statistics.b_mean(), statistics.b_median(), statistics.b_mode(), statistics.b_std_dev(),
                            statistics.b_min(), statistics.b_max(), statistics.b_lq(), statistics.b_uq());
                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{0, 100}};
                bool invert = false;
                int bins = 256;
                int l_bins = 100;
                int a_bins = 256;
                int b_bins = 256;
                image::Statistics statistics;
                start_time = time::time_us();
                statistics = img->get_statistics(thresholds, invert, roi, bins, l_bins, a_bins, b_bins, NULL);
                log::info("rgb888 get statistics cost %d us\r\n", (int)(time::time_us() - start_time));

                std::map<std::string, std::vector<float>> hist;
                hist = img->get_histogram(thresholds, invert, roi, bins, l_bins, a_bins, b_bins, NULL);
                // Process results
                {
                    int start_x = 0, start_y = 0, rect_w = 2, hist_max_h = 200;
                    std::vector<float> data = hist["L"];
                    for (size_t i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(255, 0, 0), -1);
                    }
                }

                {
                    int start_x = 0, start_y = 100, rect_w = 2, hist_max_h = 200;
                    std::vector<float> data = hist["A"];
                    for (size_t i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(0, 255, 0), -1);
                    }
                }

                {
                    int start_x = 0, start_y = 200, rect_w = 2, hist_max_h = 200;
                    std::vector<float> data = hist["B"];
                    for (size_t i = 0; i < data.size(); i ++) {
                        int height = data[i] * hist_max_h;
                        img->draw_rect(start_x + i * rect_w, start_y, rect_w, height, image::Color::from_rgb(0, 0, 255), -1);
                    }
                }

                log::info("L: mean %d, median %d, mode %d, std_dev %d, min %d, max %d, lq %d, uq %d\r\n",
                            statistics.l_mean(), statistics.l_median(), statistics.l_mode(), statistics.l_std_dev(),
                            statistics.l_min(), statistics.l_max(), statistics.l_lq(), statistics.l_uq());
                log::info("A: mean %d, median %d, mode %d, std_dev %d, min %d, max %d, lq %d, uq %d\r\n",
                            statistics.a_mean(), statistics.a_median(), statistics.a_mode(), statistics.a_std_dev(),
                            statistics.a_min(), statistics.a_max(), statistics.a_lq(), statistics.a_uq());
                log::info("B: mean %d, median %d, mode %d, std_dev %d, min %d, max %d, lq %d, uq %d\r\n",
                            statistics.b_mean(), statistics.b_median(), statistics.b_mode(), statistics.b_std_dev(),
                            statistics.b_min(), statistics.b_max(), statistics.b_lq(), statistics.b_uq());

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_GET_REGRESSION
#if 0
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                img->binary({{20, 80}}, true);
                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{0, 80, -70, -10, 0, 30}}; // GREEN
                bool invert = false;
                int x_stride = 1;
                int y_stride = 1;
                int area_threshold = 100;
                int pixels_threshold = 100;
                int robust = false;
                std::vector<image::Line> lines;
                start_time = time::time_us();
                lines = img->get_regression(thresholds, invert, roi, x_stride, y_stride, area_threshold, pixels_threshold, robust);
                log::info("gray get regression %d cost %d us\r\n", lines.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &l : lines) {
                    img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(l.x2() + 5, l.y2() + 5, "len: " + std::to_string(l.length()), image::Color::from_rgb(200, 0, 0));

                    int theta = l.theta();
                    int rho = l.rho();
                    int X = std::cos(theta * M_PI / 180) * rho;
                    int Y = std::sin(theta * M_PI / 180) * rho;
                    img->draw_line(0, 0, X, Y, image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(X + 5, Y + 5, std::to_string(theta) + "," + std::to_string(rho), image::Color::from_rgb(200, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                std::vector<std::vector<int>> thresholds = {{0, 80, 30, 100, -120, -60}}; // BLUE
                img->binary(thresholds, true);

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};

                bool invert = false;
                int x_stride = 2;
                int y_stride = 1;
                int area_threshold = 100;
                int pixels_threshold = 100;
                int robust = false;
                std::vector<image::Line> lines;
                start_time = time::time_us();
                lines = img->get_regression(thresholds, invert, roi, x_stride, y_stride, area_threshold, pixels_threshold, robust);
                log::info("rgb888 get regression %d cost %d us\r\n", lines.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &l : lines) {
                    img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(l.x2() + 5, l.y2() + 5, "len: " + std::to_string(l.length()), image::Color::from_rgb(200, 0, 0));

                    int theta = l.theta();
                    int rho = l.rho();
                    int X = std::cos(theta * M_PI / 180) * rho;
                    int Y = std::sin(theta * M_PI / 180) * rho;
                    img->draw_line(0, 0, X, Y, image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(X + 5, Y + 5, std::to_string(theta) + "," + std::to_string(rho), image::Color::from_rgb(200, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
#endif

#if TEST_FLOOD_FILL
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            start_time = time::time_us();
            img->flood_fill(100, 100);
            log::info("img.flood_fill(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_flood_fill.png");
            delete img;

            img = src_img->copy();
            start_time = time::time_us();
            img->flood_fill(100, 100, 0.10, 0.10, image::COLOR_WHITE);
            log::info("img.flood_fill(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_flood_fill_seed_floating.png");
            delete img;

            img = src_img->copy();
            start_time = time::time_us();
            img->flood_fill(100, 100, 0.10, 0.10, image::COLOR_WHITE, true, true, &mask_img);
            log::info("img.flood_fill(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_flood_fill_seed_floating_invert.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            start_time = time::time_us();
            img->flood_fill(100, 100);
            log::info("img.flood_fill(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_flood_fill.png");
            delete img;

            img = src_img->copy();
            start_time = time::time_us();
            img->flood_fill(100, 100, 0.10, 0.10, image::COLOR_WHITE);
            log::info("img.flood_fill(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_flood_fill_seed_floating.png");
            delete img;

            img = src_img->copy();
            start_time = time::time_us();
            img->flood_fill(100, 100, 0, 0, image::COLOR_WHITE, false, false, &mask_img);
            log::info("img.flood_fill(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_flood_fill_seed_floating_invert.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_ERODE
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{20, 80}});
            start_time = time::time_us();
            img->erode(2, -1, &mask_img);
            log::info("gray img.erode(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_erode_mask.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->erode(2);
            log::info("rgb888 img.erode(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_erode.png");
            delete img;

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->erode(2, -1, &mask_img);
            log::info("rgb888 img.erode(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_erode_mask.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_DILATE
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{20, 80}});
            start_time = time::time_us();
            img->dilate(2, 0, &mask_img);
            log::info("gray img.dilate(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_dilate_mask.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->dilate(2, 0);
            log::info("rgb888 img.dilate(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_dilate.png");
            delete img;

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->dilate(2, 0, &mask_img);
            log::info("rgb888 img.dilate(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_dilate_mask.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_OPEN
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{20, 80}});
            start_time = time::time_us();
            img->open(2, 0, &mask_img);
            log::info("gray img.open(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_open.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->open(2, 0);
            log::info("rgb888 img.open(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_open.png");
            delete img;

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->open(2, 0, &mask_img);
            log::info("rgb888 img.open(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_open_mask.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_CLOSE
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{20, 80}});
            start_time = time::time_us();
            img->close(2, 0, &mask_img);
            log::info("gray img.close(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_close.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->close(2, 0);
            log::info("rgb888 img.close(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_close.png");
            delete img;

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->close(2, 0, &mask_img);
            log::info("rgb888 img.close(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_close_mask.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_TOP_HAT
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{20, 80}});
            start_time = time::time_us();
            img->top_hat(2, 0, &mask_img);
            log::info("gray img.top_hat(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_top_hat.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->top_hat(2, 0);
            log::info("rgb888 img.top_hat(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_top_hat.png");
            delete img;

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->top_hat(2, 0, &mask_img);
            log::info("rgb888 img.top_hat(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_top_hat_mask.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_BLACK_HAT
    {
        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_GRAYSCALE);
            image::Image mask_img(INPUT_IMG_W, INPUT_IMG_H, image::Format::FMT_GRAYSCALE);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{20, 80}});
            start_time = time::time_us();
            img->black_hat(2, 0, &mask_img);
            log::info("gray img.black_hat(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_gray_640x480_black_hat.png");
            delete img;

            delete src_img;
        }

        {
            image::Image *img = NULL;
            image::Image *src_img = image::load(test_640x480_png, image::Format::FMT_RGB888);
            image::Image mask_img(src_img->width(), src_img->height(), image::Format::FMT_RGB888);
            mask_img.draw_rect(mask_img.width() / 4, mask_img.height() / 4, mask_img.width() / 2, mask_img.height() / 2, 255, -1);

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->black_hat(2, 0);
            log::info("rgb888 img.black_hat(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_black_hat.png");
            delete img;

            //
            img = src_img->copy();
            img->binary({{10, 80}});
            start_time = time::time_us();
            img->black_hat(2, 0, &mask_img);
            log::info("rgb888 img.black_hat(img) cost %d us\r\n", (int)(time::time_us() - start_time));
            img->save("out_rgb888_640x480_black_hat_mask.png");
            delete img;

            delete src_img;
        }
    }
#endif

#if TEST_FIND_BLOBS
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{0, 20}};
                bool invert = false;
                int x_stride = 1;
                int y_stride = 1;
                int area_threshold = 100;
                int pixels_threshold = 100;
                int merge = true;
                int margin = 0;
                int x_hist_bins_max = 2;
                int y_hist_bins_max = 2;
                std::vector<image::Blob> blobs;
                start_time = time::time_us();
                blobs = img->find_blobs(thresholds, invert, roi, x_stride, y_stride, area_threshold, pixels_threshold, merge, margin, x_hist_bins_max, y_hist_bins_max);
                log::info("find %d blobs cost %d us\r\n", blobs.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : blobs) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0));
                    }
                    img->draw_string(corners[0][0] + 5, corners[0][1] + 5, "corners area: " + std::to_string(a.area()), image::Color::from_rgb(255, 0, 0));

                    // mini_corners
                    std::vector<std::vector<int>> mini_corners = a.mini_corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(mini_corners[i][0], mini_corners[i][1], mini_corners[(i + 1) % 4][0], mini_corners[(i + 1) % 4][1], image::Color::from_rgb(0, 255, 0));
                    }
                    img->draw_string(mini_corners[0][0] + 5, mini_corners[0][1] + 5, "mini_corners", image::Color::from_rgb(0, 255, 0));

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // ...
                    img->draw_string(a.x() + a.w() + 5, a.y(), "(" + std::to_string(a.x()) + "," + std::to_string(a.y()) + ")", image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.cx(), a.cy(), "(" + std::to_string(a.cx()) + "," + std::to_string(a.cy()) + ")", image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() / 2, a.y(), std::to_string(a.w()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x(), a.y() + a.h() / 2, std::to_string(a.h()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 15, std::to_string(a.rotation_deg()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 30, "code:" + std::to_string(a.code()) + ", count:" + std::to_string(a.count()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 45, "perimeter:" + std::to_string(a.perimeter()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 60, "roundness:" + std::to_string(a.roundness()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 75, "elongation:" + std::to_string(a.elongation()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 90, "area:" + std::to_string(a.area()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 105, "density:" + std::to_string(a.density()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 120, "extent:" + std::to_string(a.extent()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 135, "compactness:" + std::to_string(a.compactness()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 150, "solidity:" + std::to_string(a.solidity()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 165, "convexity:" + std::to_string(a.convexity()), image::Color::from_rgb(0, 255, 0), 1);

                    // major axis line
                    std::vector<int> major_axis_line = a.major_axis_line();
                    img->draw_line(major_axis_line[0], major_axis_line[1], major_axis_line[2], major_axis_line[3], image::Color::from_rgb(255, 0, 0), 1);

                    // minor axis line
                    std::vector<int> minor_axis_line = a.minor_axis_line();
                    img->draw_line(minor_axis_line[0], minor_axis_line[1], minor_axis_line[2], minor_axis_line[3], image::Color::from_rgb(0, 0, 255), 1);

                    // enclosing circle
                    std::vector<int> enclosing_circle = a.enclosing_circle();
                    img->draw_circle(enclosing_circle[0], enclosing_circle[1], enclosing_circle[2], image::Color::from_rgb(255, 0, 0), 1);

                    // enclosing ellipse
                    std::vector<int> enclosed_ellipse = a.enclosed_ellipse();
                    img->draw_ellipse(enclosed_ellipse[0], enclosed_ellipse[1], enclosed_ellipse[2], enclosed_ellipse[3], enclosed_ellipse[4], 0, 360, image::Color::from_rgb(0, 0, 255), 1);

                    // hist
                    std::vector<int> x_hist_bins = a.x_hist_bins();
                    std::vector<int> y_hist_bins = a.y_hist_bins();
                    // for (int i = 0; i < x_hist_bins.size(); i ++) {
                    //     printf("[%d] (%d, %d)\r\n", i, x_hist_bins[i], y_hist_bins[i]);
                    // }
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<std::vector<int>> thresholds = {{0, 80, -70, -10, 0, 30}}; // GREEN
                bool invert = false;
                int x_stride = 1;
                int y_stride = 1;
                int area_threshold = 100;
                int pixels_threshold = 100;
                int merge = true;
                int margin = 0;
                int x_hist_bins_max = 2;
                int y_hist_bins_max = 2;
                std::vector<image::Blob> blobs;
                start_time = time::time_us();
                blobs = img->find_blobs(thresholds, invert, roi, x_stride, y_stride, area_threshold, pixels_threshold, merge, margin, x_hist_bins_max, y_hist_bins_max);
                log::info("find %d blobs cost %d us\r\n", blobs.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &a : blobs) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0));
                    }
                    img->draw_string(corners[0][0] + 5, corners[0][1] + 5, "corners area: " + std::to_string(a.area()), image::Color::from_rgb(255, 0, 0));

                    // mini_corners
                    std::vector<std::vector<int>> mini_corners = a.mini_corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(mini_corners[i][0], mini_corners[i][1], mini_corners[(i + 1) % 4][0], mini_corners[(i + 1) % 4][1], image::Color::from_rgb(0, 255, 0));
                    }
                    img->draw_string(mini_corners[0][0] + 5, mini_corners[0][1] + 5, "mini_corners", image::Color::from_rgb(0, 255, 0));

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // ...
                    img->draw_string(a.x() + a.w() + 5, a.y(), "(" + std::to_string(a.x()) + "," + std::to_string(a.y()) + ")", image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.cx(), a.cy(), "(" + std::to_string(a.cx()) + "," + std::to_string(a.cy()) + ")", image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() / 2, a.y(), std::to_string(a.w()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x(), a.y() + a.h() / 2, std::to_string(a.h()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 15, std::to_string(a.rotation_deg()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 30, "code:" + std::to_string(a.code()) + ", count:" + std::to_string(a.count()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 45, "perimeter:" + std::to_string(a.perimeter()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 60, "roundness:" + std::to_string(a.roundness()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 75, "elongation:" + std::to_string(a.elongation()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 90, "area:" + std::to_string(a.area()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 105, "density:" + std::to_string(a.density()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 120, "extent:" + std::to_string(a.extent()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 135, "compactness:" + std::to_string(a.compactness()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 150, "solidity:" + std::to_string(a.solidity()), image::Color::from_rgb(0, 255, 0), 1);
                    img->draw_string(a.x() + a.w() + 5, a.y() + 165, "convexity:" + std::to_string(a.convexity()), image::Color::from_rgb(0, 255, 0), 1);

                    // major axis line
                    std::vector<int> major_axis_line = a.major_axis_line();
                    img->draw_line(major_axis_line[0], major_axis_line[1], major_axis_line[2], major_axis_line[3], image::Color::from_rgb(255, 0, 0), 1);

                    // minor axis line
                    std::vector<int> minor_axis_line = a.minor_axis_line();
                    img->draw_line(minor_axis_line[0], minor_axis_line[1], minor_axis_line[2], minor_axis_line[3], image::Color::from_rgb(0, 0, 255), 1);

                    // enclosing circle
                    std::vector<int> enclosing_circle = a.enclosing_circle();
                    img->draw_circle(enclosing_circle[0], enclosing_circle[1], enclosing_circle[2], image::Color::from_rgb(255, 0, 0), 1);

                    // enclosing ellipse
                    std::vector<int> enclosed_ellipse = a.enclosed_ellipse();
                    img->draw_ellipse(enclosed_ellipse[0], enclosed_ellipse[1], enclosed_ellipse[2], enclosed_ellipse[3], enclosed_ellipse[4], 0, 360, image::Color::from_rgb(0, 0, 255), 1);

                    // hist
                    std::vector<int> x_hist_bins = a.x_hist_bins();
                    std::vector<int> y_hist_bins = a.y_hist_bins();
                    // for (int i = 0; i < x_hist_bins.size(); i ++) {
                    //     printf("[%d] (%d, %d)\r\n", i, x_hist_bins[i], y_hist_bins[i]);
                    // }
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_LINES
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int x_stride = 1;
                int y_stride = 1;
                int threshold = 2000;
                int theta_margin = 30;
                int rho_margin = 30;
                std::vector<image::Line> lines;
                start_time = time::time_us();
                lines = img->find_lines(roi, x_stride, y_stride, threshold, theta_margin, rho_margin);
                log::info("find %d lines cost %d us\r\n", lines.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &l : lines) {
                    img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(l.x2() + 5, l.y2() + 5, "len: " + std::to_string(l.length()), image::Color::from_rgb(200, 0, 0));

                    int theta = l.theta();
                    int rho = l.rho();
                    int X = std::cos(theta * M_PI / 180) * rho;
                    int Y = std::sin(theta * M_PI / 180) * rho;
                    img->draw_line(0, 0, X, Y, image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(X + 5, Y + 5, std::to_string(theta) + "," + std::to_string(rho), image::Color::from_rgb(200, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int x_stride = 1;
                int y_stride = 1;
                int threshold = 2000;
                int theta_margin = 30;
                int rho_margin = 30;
                std::vector<image::Line> lines;
                start_time = time::time_us();
                lines = img->find_lines(roi, x_stride, y_stride, threshold, theta_margin, rho_margin);
                log::info("find %d lines cost %d us\r\n", lines.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &l : lines) {
                    img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(l.x2() + 5, l.y2() + 5, "len: " + std::to_string(l.length()), image::Color::from_rgb(200, 0, 0));

                    int theta = l.theta();
                    int rho = l.rho();
                    int X = std::cos(theta * M_PI / 180) * rho;
                    int Y = std::sin(theta * M_PI / 180) * rho;
                    img->draw_line(0, 0, X, Y, image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(X + 5, Y + 5, std::to_string(theta) + "," + std::to_string(rho), image::Color::from_rgb(200, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_LINE_SEGMENTS
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int merge_distance = 1;
                int max_theta_difference = 20;
                std::vector<image::Line> lines;
                start_time = time::time_us();
                lines = img->find_line_segments(roi, merge_distance, max_theta_difference);
                log::info("find %d lines cost %d us\r\n", lines.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &l : lines) {
                    img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(l.x2() + 5, l.y2() + 5, "len: " + std::to_string(l.length()), image::Color::from_rgb(200, 0, 0));

                    int theta = l.theta();
                    int rho = l.rho();
                    int X = std::cos(theta * M_PI / 180) * rho;
                    int Y = std::sin(theta * M_PI / 180) * rho;
                    img->draw_line(0, 0, X, Y, image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(X + 5, Y + 5, std::to_string(theta) + "," + std::to_string(rho), image::Color::from_rgb(200, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int merge_distance = 20;
                int max_theta_difference = 20;
                std::vector<image::Line> lines;
                start_time = time::time_us();
                lines = img->find_line_segments(roi, merge_distance, max_theta_difference);
                log::info("find %d lines cost %d us\r\n", lines.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &l : lines) {
                    img->draw_line(l.x1(), l.y1(), l.x2(), l.y2(), image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(l.x2() + 5, l.y2() + 5, "len: " + std::to_string(l.length()), image::Color::from_rgb(200, 0, 0));

                    int theta = l.theta();
                    int rho = l.rho();
                    int X = std::cos(theta * M_PI / 180) * rho;
                    int Y = std::sin(theta * M_PI / 180) * rho;
                    img->draw_line(0, 0, X, Y, image::Color::from_rgb(200, 0, 0), 2);
                    img->draw_string(X + 5, Y + 5, std::to_string(theta) + "," + std::to_string(rho), image::Color::from_rgb(200, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_CIRCLES
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int x_stride = 1;
                int y_stride = 1;
                int threshold = 3000;
                int x_margin = 10;
                int y_margin = 10;
                int r_margin = 10;
                int r_min = 20;
                int r_max = 50;
                int r_step = 2;
                std::vector<image::Circle> circles;
                start_time = time::time_us();
                circles = img->find_circles(roi, x_stride, y_stride, threshold, x_margin, y_margin, r_margin, r_min, r_max, r_step);
                log::info("find %d circles cost %d us\r\n", circles.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : circles) {
                    img->draw_circle(a.x(), a.y(), a.r(), image::Color::from_rgb(255, 0, 0), 2);
                    img->draw_string(a.x() + a.r() + 5, a.y() + a.r() + 5, "r: " + std::to_string(a.r()) + "magnitude: " + std::to_string(a.magnitude()), image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int x_stride = 1;
                int y_stride = 1;
                int threshold = 3000;
                int x_margin = 10;
                int y_margin = 10;
                int r_margin = 10;
                int r_min = 20;
                int r_max = 50;
                int r_step = 2;
                std::vector<image::Circle> circles;
                start_time = time::time_us();
                circles = img->find_circles(roi, x_stride, y_stride, threshold, x_margin, y_margin, r_margin, r_min, r_max, r_step);
                log::info("find %d circles cost %d us\r\n", circles.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &a : circles) {
                    img->draw_circle(a.x(), a.y(), a.r(), image::Color::from_rgb(255, 0, 0), 2);
                    img->draw_string(a.x() + a.r() + 5, a.y() + a.r() + 5, "r: " + std::to_string(a.r()) + "magnitude: " + std::to_string(a.magnitude()), image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_RECTS
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int threshold = 1000;
                std::vector<image::Rect> rects;
                start_time = time::time_us();
                rects = img->find_rects(roi, threshold);
                log::info("find %d rects cost %d us\r\n", rects.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : rects) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0));
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 20, "magnitude: " + std::to_string(a.magnitude()), image::Color::from_rgb(0, 0, 255));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int threshold = 1000;
                std::vector<image::Rect> rects;
                start_time = time::time_us();
                rects = img->find_rects(roi, threshold);
                log::info("find %d rects cost %d us\r\n", rects.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &a : rects) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0));
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 20, "magnitude: " + std::to_string(a.magnitude()), image::Color::from_rgb(0, 0, 255));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_QRCODES
    {
#if 0
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
                image::Image *img = src_img_temp;
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<image::QRCode> qrcodes;
                start_time = time::time_us();
                qrcodes = img->find_qrcodes(roi);
                log::info("find %d qrcodes cost %d us\r\n", qrcodes.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : qrcodes) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0));
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // payload
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "payload: " + a.payload(), image::Color::from_rgb(0, 0, 255));

                    // version
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 35, "version: " + std::to_string(a.version()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 50, "ecc_level: " + std::to_string(a.ecc_level()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 65, "mask: " + std::to_string(a.mask()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 80, "data_type: " + std::to_string(a.data_type()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 95, "eci: " + std::to_string(a.eci()), image::Color::from_rgb(0, 0, 255));
                    if (a.is_numeric()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is numeric", image::Color::from_rgb(0, 0, 255));
                    } else if (a.is_alphanumeric()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is alphanumeric", image::Color::from_rgb(0, 0, 255));
                    } else if (a.is_binary()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is binary", image::Color::from_rgb(0, 0, 255));
                    } else if (a.is_kanji()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is kanji", image::Color::from_rgb(0, 0, 255));
                    } else {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is unknown", image::Color::from_rgb(0, 0, 255));
                    }
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<image::QRCode> qrcodes;
                start_time = time::time_us();
                qrcodes = img->find_qrcodes(roi);
                log::info("find %d qrcodes cost %d us\r\n", qrcodes.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &a : qrcodes) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0));
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255));
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // payload
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "payload: " + a.payload(), image::Color::from_rgb(0, 0, 255));

                    // version
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 35, "version: " + std::to_string(a.version()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 50, "ecc_level: " + std::to_string(a.ecc_level()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 65, "mask: " + std::to_string(a.mask()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 80, "data_type: " + std::to_string(a.data_type()), image::Color::from_rgb(0, 0, 255));
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 95, "eci: " + std::to_string(a.eci()), image::Color::from_rgb(0, 0, 255));
                    if (a.is_numeric()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is numeric", image::Color::from_rgb(0, 0, 255));
                    } else if (a.is_alphanumeric()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is alphanumeric", image::Color::from_rgb(0, 0, 255));
                    } else if (a.is_binary()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is binary", image::Color::from_rgb(0, 0, 255));
                    } else if (a.is_kanji()) {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is kanji", image::Color::from_rgb(0, 0, 255));
                    } else {
                        img->draw_string(a.x() + a.w() + 5, rect[1] + 110, "is unknown", image::Color::from_rgb(0, 0, 255));
                    }
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_APRILTAGS
    {
#if 0
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                image::ApriltagFamilies families = image::TAG36H11;
                float fx = -1;
                float fy = -1;
                int cx = img->width() / 2;
                int cy = img->height() / 2;
                std::vector<image::AprilTag> apriltags;
                start_time = time::time_us();
                apriltags = img->find_apriltags(roi, families, fx, fy, cx, cy);
                log::info("find %d apriltags cost %d us\r\n", apriltags.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : apriltags) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0), 2);
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255), 2);
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // apriltag
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "id: " + std::to_string(a.id()), image::Color::from_rgb(255, 0, 0));

                    // family
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 35, "family: " + std::to_string(a.family()), image::Color::from_rgb(255, 0, 0));

                    // center coordinate
                    img->draw_string(a.cx(), a.cy(), "(" + std::to_string(a.cx()) + "," + std::to_string(a.cy()) + ")", image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(a.cx(), a.cy() + 15, "rot: " + std::to_string(a.rotation()), image::Color::from_rgb(255, 0, 0));

                    // hamming
                    img->draw_string(a.cx(), a.cy() + 30, "hamming: " + std::to_string(a.hamming()), image::Color::from_rgb(255, 0, 0));

                    // goodness
                    img->draw_string(a.cx(), a.cy() + 45, "goodness: " + std::to_string(a.goodness()), image::Color::from_rgb(255, 0, 0));

                    // translation
                    img->draw_string(a.cx(), a.cy() + 60, "translation: (" + std::to_string(a.x_translation()) + "," + std::to_string(a.y_translation()) + "," + std::to_string(a.z_translation()) + ")", image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(a.cx(), a.cy() + 75, "rotation: (" + std::to_string(a.x_rotation()) + "," + std::to_string(a.y_rotation()) + "," + std::to_string(a.z_rotation()) + ")", image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                maix::image::ApriltagFamilies families = image::TAG36H11;
                float fx = -1;
                float fy = -1;
                std::vector<image::AprilTag> apriltags;
                start_time = time::time_us();
                maix::image::Image *resize_img = img->resize(160, 120);
                int cx = resize_img->width() / 2;
                int cy = resize_img->height() / 2;
                float x_scale = (float)img->width()  / 160;
                float y_scale = (float)img->height() / 120;
                float w_scale = x_scale;
                std::vector<int> roi = {0, 0, resize_img->width(), resize_img->height()};
                apriltags = resize_img->find_apriltags(roi, families, fx, fy, cx, cy);
                delete resize_img;
                log::info("find %d apriltags cost %d us\r\n", apriltags.size(), (int)(time::time_us() - start_time));

                static uint64_t last_us = 0;
                log::info("loop cost %d us\r\n", (int)(time::time_us() - last_us));
                last_us = time::time_us();

                // Process results
                for (auto &a : apriltags) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        corners[i][0] = corners[i][0] * x_scale;
                        corners[i][1] = corners[i][1] * y_scale;
                    }
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0), 2);
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    rect[0] = rect[0] * x_scale;
                    rect[1] = rect[1] * y_scale;
                    rect[2] = rect[2] * x_scale;
                    rect[3] = rect[3] * y_scale;
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255), 2);
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    int x = a.x() * x_scale;
                    int w = a.w() * w_scale;
                    int cx = a.cx() * x_scale;
                    int cy = a.cy() * y_scale;

                    // apriltag
                    img->draw_string(x + w + 5, rect[1] + 20, "id: " + std::to_string(a.id()), image::Color::from_rgb(255, 0, 0));

                    // family
                    img->draw_string(x + w + 5, rect[1] + 35, "family: " + std::to_string(a.family()), image::Color::from_rgb(255, 0, 0));

                    // center coordinate
                    img->draw_string(cx, cy, "(" + std::to_string(cx) + "," + std::to_string(cy) + ")", image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(cx, cy + 15, "rot: " + std::to_string(a.rotation()), image::Color::from_rgb(255, 0, 0));

                    // hamming
                    img->draw_string(cx, cy + 30, "hamming: " + std::to_string(a.hamming()), image::Color::from_rgb(255, 0, 0));

                    // goodness
                    img->draw_string(cx, cy + 45, "goodness: " + std::to_string(a.goodness()), image::Color::from_rgb(255, 0, 0));

                    // translation
                    img->draw_string(cx, cy + 60, "translation: (" + std::to_string(a.x_translation()) + "," + std::to_string(a.y_translation()) + "," + std::to_string(a.z_translation()) + ")", image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(cx, cy + 75, "rotation: (" + std::to_string(a.x_rotation()) + "," + std::to_string(a.y_rotation()) + "," + std::to_string(a.z_rotation()) + ")", image::Color::from_rgb(255, 0, 0));
                }

                // // Draw ROI
                // img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                // img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_DATAMATRICES
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int effort = 200;
                std::vector<image::DataMatrix> datamatrices;
                start_time = time::time_us();
                datamatrices = img->find_datamatrices(roi, effort);
                log::info("find %d datamatrices cost %d us\r\n", datamatrices.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : datamatrices) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0), 2);
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255), 2);
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // payload
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "payload: " + a.payload(), image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(a.x(), a.y() + 15, "rot: " + std::to_string(a.rotation()), image::Color::from_rgb(255, 0, 0));

                    // rows and columns
                    img->draw_string(a.x(), a.y() + 30, "rows: " + std::to_string(a.rows()) + ", columns: " + std::to_string(a.columns()), image::Color::from_rgb(255, 0, 0));

                    // capacity
                    img->draw_string(a.x(), a.y() + 45, "capacity: " + std::to_string(a.capacity()), image::Color::from_rgb(255, 0, 0));

                    // padding
                    img->draw_string(a.x(), a.y() + 60, "padding: " + std::to_string(a.padding()), image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int effort = 200;
                std::vector<image::DataMatrix> datamatrices;
                start_time = time::time_us();
                datamatrices = img->find_datamatrices(roi, effort);
                log::info("find %d datamatrices cost %d us\r\n", datamatrices.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &a : datamatrices) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0), 2);
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255), 2);
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // payload
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "payload: " + a.payload(), image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(a.x(), a.y() + 15, "rot: " + std::to_string(a.rotation()), image::Color::from_rgb(255, 0, 0));

                    // rows and columns
                    img->draw_string(a.x(), a.y() + 30, "rows: " + std::to_string(a.rows()) + ", columns: " + std::to_string(a.columns()), image::Color::from_rgb(255, 0, 0));

                    // capacity
                    img->draw_string(a.x(), a.y() + 45, "capacity: " + std::to_string(a.capacity()), image::Color::from_rgb(255, 0, 0));

                    // padding
                    img->draw_string(a.x(), a.y() + 60, "padding: " + std::to_string(a.padding()), image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_BARCODES
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<image::BarCode> barcodes;
                start_time = time::time_us();
                barcodes = img->find_barcodes(roi);
                log::info("find %d barcodes cost %d us\r\n", barcodes.size(), (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                for (auto &a : barcodes) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0), 2);
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255), 2);
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // payload
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "payload: " + a.payload(), image::Color::from_rgb(255, 0, 0));

                    // type
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 35, "type: " + std::to_string(a.type()), image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(a.x(), a.y() + 15, "rot: " + std::to_string(a.rotation()), image::Color::from_rgb(255, 0, 0));

                    // quality
                    img->draw_string(a.x(), a.y() + 30, "quality: " + std::to_string(a.quality()), image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<image::BarCode> barcodes;
                start_time = time::time_us();
                barcodes = img->find_barcodes(roi);
                log::info("find %d barcodes cost %d us\r\n", barcodes.size(), (int)(time::time_us() - start_time));

                // Process results
                for (auto &a : barcodes) {
                    // corners
                    std::vector<std::vector<int>> corners = a.corners();
                    for (int i = 0; i < 4; i ++) {
                        img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], image::Color::from_rgb(255, 0, 0), 2);
                    }

                    // rect
                    std::vector<int> rect = a.rect();
                    img->draw_rect(rect[0], rect[1], rect[2], rect[3], image::Color::from_rgb(0, 0, 255), 2);
                    img->draw_string(rect[0] + 5, rect[1] + 5, "rect", image::Color::from_rgb(0, 0, 255));

                    // payload
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 20, "payload: " + a.payload(), image::Color::from_rgb(255, 0, 0));

                    // type
                    img->draw_string(a.x() + a.w() + 5, rect[1] + 35, "type: " + std::to_string(a.type()), image::Color::from_rgb(255, 0, 0));

                    // rotation
                    img->draw_string(a.x(), a.y() + 15, "rot: " + std::to_string(a.rotation()), image::Color::from_rgb(255, 0, 0));

                    // quality
                    img->draw_string(a.x(), a.y() + 30, "quality: " + std::to_string(a.quality()), image::Color::from_rgb(255, 0, 0));
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_DISPLACEMENT
    {
        image::Image *template_image = NULL;
        std::vector<int> template_roi = {};
#if 1
        {
            log::info("Enter ctrl+c to snap an template image.\r\n");
            app::set_exit_flag(false);
            while(1)
            {
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (img) {
                    template_roi = {10, 10, img->width() - 20, img->height() - 20};

                    if (app::need_exit()) {
                        template_image = img->copy();
                        log::info("Snap an template image.\r\n");
                        delete img;
                        break;
                    }

                    // Draw ROI
                    img->draw_string(template_roi[0] + 5, template_roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                    img->draw_rect(template_roi[0], template_roi[1], template_roi[2], template_roi[3], image::Color::from_rgb(0, 255, 0));

                    // Show image
    #if DISPLAY_ENABLE
                    disp.show(*img);
#endif

                    // Free image data, important!
                    delete img;
                }
            }
        }
#endif

#if 1
        {
            image::Image *new_template_img = template_image->to_format(image::Format::FMT_GRAYSCALE);

            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                // std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                std::vector<int> roi = template_roi;
                bool logpolar = true;
                image::Displacement displacement;
                start_time = time::time_us();
                displacement = img->find_displacement(*new_template_img, roi, template_roi, logpolar);
                log::info("find %d displacement cost %d us\r\n", 1, (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                img->draw_string(50, 50, "x: " + std::to_string(displacement.x_translation()), image::Color::from_rgb(255, 0, 0));
                img->draw_string(50, 65, "y: " + std::to_string(displacement.y_translation()), image::Color::from_rgb(255, 0, 0));
                img->draw_string(50, 80, "rotation: " + std::to_string(displacement.rotation()), image::Color::from_rgb(255, 0, 0));
                img->draw_string(50, 95, "scale: " + std::to_string(displacement.scale()), image::Color::from_rgb(255, 0, 0));
                img->draw_string(50, 110, "response: " + std::to_string(displacement.response()), image::Color::from_rgb(255, 0, 0));

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_TEMPLATE
    {
        image::Image *template_image = NULL;
        std::vector<int> template_roi = {};
#if 1
        {
            log::info("Enter ctrl+c to snap an template image.\r\n");
            app::set_exit_flag(false);
            while(1) {
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (img) {
                    template_roi = {img->width() / 4, img->height() / 4, img->width() / 4, img->height() / 4};

                    if (app::need_exit()) {
                        // template_image = img->copy();
                        template_image = img->crop(template_roi[0], template_roi[1], template_roi[2], template_roi[3]);
                        log::info("Snap an template image.\r\n");
                        delete img;
                        break;
                    }

                    // Draw ROI
                    img->draw_string(template_roi[0] + 5, template_roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                    img->draw_rect(template_roi[0], template_roi[1], template_roi[2], template_roi[3], image::Color::from_rgb(0, 255, 0));

                    // Show image
    #if DISPLAY_ENABLE
                    disp.show(*img);
#endif

                    // Free image data, important!
                    delete img;
                }
            }
        }
#endif

#if 1
        {
            image::Image *new_template_img = template_image->to_format(image::Format::FMT_GRAYSCALE);

            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                float threshold = 0.7;
                int step = 2;
                image::TemplateMatch search = image::SEARCH_DS;
                // image::TemplateMatch search = image::SEARCH_EX;
                std::vector<int> template_rect;
                start_time = time::time_us();
                template_rect = img->find_template(*new_template_img, threshold, roi, step, search);
                log::info("find %d template_roi cost %d us\r\n", template_roi.size() != 0 ? 1 : 0, (int)(time::time_us() - start_time));
                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                if (template_rect.size() >= 4) {
                    img->draw_rect(template_rect[0], template_rect[1], template_rect[2], template_rect[3], image::Color::from_rgb(0, 0, 255), 2);
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Draw template image
                img->draw_image(0, 200, *new_template_img);

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }

            delete new_template_img;
        }
#endif

#if 1
        {
            image::Image *new_template_img = template_image;

            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                float threshold = 0.5;
                int step = 4;
                // image::TemplateMatch search = image::SEARCH_DS;
                image::TemplateMatch search = image::SEARCH_EX;
                std::vector<int> template_rect;
                start_time = time::time_us();
                template_rect = img->find_template(*new_template_img, threshold, roi, step, search);
                log::info("find %d template_roi cost %d us\r\n", template_roi.size() != 0 ? 1 : 0, (int)(time::time_us() - start_time));

                // Process results
                if (template_rect.size() >= 4) {
                    img->draw_rect(template_rect[0], template_rect[1], template_rect[2], template_rect[3], image::Color::from_rgb(0, 0, 255), 2);
                }

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Draw template image
                img->draw_image(0, 200, *new_template_img);

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
        if (template_image)
            delete template_image;
    }
#endif

#if TEST_FIND_FEATURES
    {

    }
#endif

#if TEST_FIND_LBP
    {
#if 1
        {
            app::set_exit_flag(false);
            int need_first_keypoint = 0;
            image::LBPKeyPoint first_keypoint;
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                char key = get_key();
                if (key != 0) {
                    log::info("key %c pressed\r\n", key);
                }

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 2, img->height() / 2, img->width() / 4, img->height() / 4};
                std::vector<int> first_keypoint_roi = {img->width() / 8, img->height() / 8, img->width() / 4, img->height() / 4};
                int distance = 0;
                if (need_first_keypoint && key == 's') {
                    start_time = time::time_us();
                    first_keypoint = img->find_lbp(first_keypoint_roi);
                    log::info("find first lbp cost %d us\r\n", (int)(time::time_us() - start_time));
                    need_first_keypoint = 0;
                } else if (!need_first_keypoint) {
                    start_time = time::time_us();
                    image::LBPKeyPoint keypoint = img->find_lbp(roi);
                    log::info("find other lbp cost %d us\r\n", (int)(time::time_us() - start_time));
                    // distance = img->match_lbp_descriptor(first_keypoint, keypoint);
                }

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                if (need_first_keypoint) {
                    img->draw_string(0, 0, "enter s to calculate first keypoint", image::Color::from_rgb(255, 0, 0));
                }
                img->draw_string(50, 50, "distance: " + std::to_string(distance), image::Color::from_rgb(255, 0, 0));

                // Draw ROI
                img->draw_string(first_keypoint_roi[0] + 5, first_keypoint_roi[1] + 5, "First ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(first_keypoint_roi[0], first_keypoint_roi[1], first_keypoint_roi[2], first_keypoint_roi[3], image::Color::from_rgb(0, 255, 0));

                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_KEYPOINTS
    {
#if 1
        {
            app::set_exit_flag(false);
            int need_first_keypoint = 0;
            image::LBPKeyPoint first_keypoint;
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                char key = get_key();
                if (key != 0) {
                    log::info("key %c pressed\r\n", key);
                }

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 2, img->height() / 2, img->width() / 4, img->height() / 4};
                std::vector<int> first_keypoint_roi = {img->width() / 8, img->height() / 8, img->width() / 4, img->height() / 4};
                int distance = 0;
                if (need_first_keypoint && key == 's') {
                    start_time = time::time_us();
                    first_keypoint = img->find_lbp(first_keypoint_roi);
                    log::info("find first lbp cost %d us\r\n", (int)(time::time_us() - start_time));
                    need_first_keypoint = 0;
                } else if (!need_first_keypoint) {
                    start_time = time::time_us();
                    image::LBPKeyPoint keypoint = img->find_lbp(roi);
                    log::info("find other lbp cost %d us\r\n", (int)(time::time_us() - start_time));
                    // distance = img->match_lbp_descriptor(first_keypoint, keypoint);
                }

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                if (need_first_keypoint) {
                    img->draw_string(0, 0, "enter s to calculate first keypoint", image::Color::from_rgb(255, 0, 0));
                }
                img->draw_string(50, 50, "distance: " + std::to_string(distance), image::Color::from_rgb(255, 0, 0));

                // Draw ROI
                img->draw_string(first_keypoint_roi[0] + 5, first_keypoint_roi[1] + 5, "First ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(first_keypoint_roi[0], first_keypoint_roi[1], first_keypoint_roi[2], first_keypoint_roi[3], image::Color::from_rgb(0, 255, 0));

                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_EDGES
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                image::EdgeDetector edge_type = image::EDGE_CANNY;
                std::vector<int> threshold = {100, 200};
                start_time = time::time_us();
                img = img->find_edges(edge_type, roi, threshold);
                log::info("find edges cost %d us\r\n", (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                // None

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                image::EdgeDetector edge_type = image::EDGE_CANNY;
                std::vector<int> threshold = {100, 200};
                start_time = time::time_us();
                img->find_edges(edge_type, roi, threshold);
                log::info("find edges cost %d us\r\n", (int)(time::time_us() - start_time));

                // Process results
                // None

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_FIND_HOG
    {
#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                image::Image *new_gray_img = img->to_format(image::Format::FMT_GRAYSCALE);
                delete img;
                img = new_gray_img;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int size = 8;
                start_time = time::time_us();
                img = img->find_hog(roi, size);
                log::info("find hog cost %d us\r\n", (int)(time::time_us() - start_time));

                image::Image *new_rgb_img = img->to_format(image::Format::FMT_RGB888);
                delete img;
                img = new_rgb_img;

                // Process results
                // None

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif

#if 1
        {
            app::set_exit_flag(false);
            while(!app::need_exit())
            {
                // Read image from the camera
#if CAMERA_ENABLE
                image::Image *img = cam.read();
#else
                image::Image *img = test_cam_img->resize(INPUT_IMG_W, INPUT_IMG_H);
#endif
                if (!img)
                    continue;

                // Process find
                std::vector<int> roi = {img->width() / 4, img->height() / 4, img->width() / 2, img->height() / 2};
                int size = 8;
                start_time = time::time_us();
                img = img->find_hog(roi, size);
                log::info("find edges cost %d us\r\n", (int)(time::time_us() - start_time));

                // Process results
                // None

                // Draw ROI
                img->draw_string(roi[0] + 5, roi[1] + 5, "ROI", image::Color::from_rgb(0, 255, 0));
                img->draw_rect(roi[0], roi[1], roi[2], roi[3], image::Color::from_rgb(0, 255, 0));

                // Show image
#if DISPLAY_ENABLE
                disp.show(*img);
#endif
                // Free image data, important!
                delete img;
            }
        }
#endif
    }
#endif

#if TEST_STERO_DISPARITY
    {

    }
#endif

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

void print_image(image::Image &img)
{
#if PRINTF_IMG_EN
    printf("{\r\n");
    switch (img.format())
    {
    case image::Format::FMT_GRAYSCALE:
        for (int h = 0; h < img.height(); h++)
        {
            for (int w = 0; w < img.width(); w++)
            {
                std::vector<int> pixel = img.get_pixel(w, h);
                if (pixel.size() > 0)
                    printf("%3d ", pixel[0]);
            }
            printf("\n");
        }
        break;
    case image::Format::FMT_BGR888:
    case image::Format::FMT_RGB888:
        for (int h = 0; h < img.height(); h++)
        {
            for (int w = 0; w < img.width(); w++)
            {
                std::vector<int> pixel = img.get_pixel(w, h, true);
                if (pixel.size() > 2)
                    printf("(%3d, %3d, %3d) ", pixel[0], pixel[1], pixel[2]);
            }
            printf("\n");
        }
        break;
    case image::Format::FMT_BGR565:
    case image::Format::FMT_RGB565:
        for (int h = 0; h < img.height(); h++)
        {
            for (int w = 0; w < img.width(); w++)
            {
                std::vector<int> pixel = img.get_pixel(w, h, true);
                if (pixel.size() > 2)
                    printf("(%3d, %3d, %3d) ", pixel[0], pixel[1], pixel[2]);
            }
            printf("\n");
        }
        break;
    default :
        printf("Not support format: %d\n", img.format());
        break;
    }
    printf("}\r\n");
#endif
}