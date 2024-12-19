#include "test_image.hpp"

#define TEST_ZBAR   (0)
#define TEST_ZXING  (0)

#if TEST_ZBAR
#include "zbar.hpp"

static zbar_qrcode_result_t zbar_scan_qrcode(image::Image *img)
{
    image::Image *gray_img = NULL;

    if (img->format() == image::FMT_GRAYSCALE) {
        gray_img = img;
    } else {
        if (image::FMT_YVU420SP == img->format()) {
            gray_img = new image::Image(img->width(), img->height(), image::FMT_GRAYSCALE);
            memcpy(gray_img->data(), img->data(), img->width() * img->height());
        } else {
            gray_img = img->to_format(image::FMT_GRAYSCALE);
        }
    }

    zbar_qrcode_result_t  result;
    zbar_scan_qrcode_in_gray((uint8_t *)gray_img->data(), gray_img->width(), gray_img->height(), &result);

    if (img->format() != image::FMT_GRAYSCALE) {
        delete gray_img;
    }

    return result;
}
#elif TEST_ZXING
#include "ReadBarcode.h"
#include <iostream>
static int zxing_scan_barcode(image::Image *img)
{
    image::Image *gray_img = NULL;

    if (img->format() == image::FMT_GRAYSCALE) {
        gray_img = img;
    } else {
        if (image::FMT_YVU420SP == img->format()) {
            gray_img = new image::Image(img->width(), img->height(), image::FMT_GRAYSCALE);
            memcpy(gray_img->data(), img->data(), img->width() * img->height());
        } else {
            gray_img = img->to_format(image::FMT_GRAYSCALE);
        }
    }

    int width = gray_img->width(), height = gray_img->height();
    unsigned char* data = (unsigned char*)gray_img->data();

    auto image = ZXing::ImageView(data, width, height, ZXing::ImageFormat::Lum);
    auto barcodes = ZXing::ReadBarcodes(image);

    log::info(" zxing find barcodes num:%d", barcodes.size());
    for (const auto& b : barcodes)
        std::cout << ZXing::ToString(b.format()) << ": " << b.text() << "\n";
    if (barcodes.empty()) {
        log::info("barcode is not found!");
    }
    if (img->format() != image::FMT_GRAYSCALE) {
        delete gray_img;
    }
    return 0;
}

#endif

int test_find_barcode(image::Image *img) {
    uint64_t t = time::ticks_ms();

#if TEST_ZBAR
    t = time::ticks_ms();
    auto zbar_scan_result = zbar_scan_qrcode(img);
    log::info("zbar scan use %lld ms, fps:%f", time::ticks_ms() - t, 1000.0 / (time::ticks_ms() - t));
    for (size_t i = 0; i < zbar_scan_result.data.size(); i ++) {
        auto corner = zbar_scan_result.corners[i];
        log::info("zbar scan result: %s", zbar_scan_result.data[i].c_str());
        log::info("zbar corners: (%d, %d) (%d, %d) (%d, %d) (%d, %d)",  corner[0],corner[1],
                                                                        corner[2],corner[3],
                                                                        corner[4],corner[5],
                                                                        corner[6],corner[7]);
        img->draw_line(corner[0], corner[1], corner[2], corner[3], maix::image::Color::from_rgb(0, 255, 0), 2);
        img->draw_line(corner[2], corner[3], corner[4], corner[5], maix::image::Color::from_rgb(0, 255, 0), 2);
        img->draw_line(corner[4], corner[5], corner[6], corner[7], maix::image::Color::from_rgb(0, 255, 0), 2);
        img->draw_line(corner[6], corner[7], corner[0], corner[1], maix::image::Color::from_rgb(0, 255, 0), 2);
    }
    if (zbar_scan_result.data.empty()) {
        log::info("barcode is not found!");
    }

#elif TEST_ZXING
    zxing_scan_barcode(img);
    log::info("find_barcodes scan use %lld ms, fps:%f", time::ticks_ms() - t, 1000.0 / (time::ticks_ms() - t));
#else
    t = time::ticks_ms();
    auto res = img->find_barcodes();
    log::info("find_barcodes scan use %lld ms, fps:%f", time::ticks_ms() - t, 1000.0 / (time::ticks_ms() - t));
    for (auto &i : res)
    {
        log::info("find_barcodes scan result: %s", i.payload().c_str());
        std::vector<std::vector<int>> corners = i.corners();
        for (int i = 0; i < 4; i ++) {
            img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], maix::image::Color::from_rgb(0, 255, 0), 2);
        }

        auto rect = i.rect();
        img->draw_rect(rect[0], rect[1], rect[2], rect[3], maix::image::Color::from_rgb(255, 0, 0));
    }

    if (res.empty()) {
        log::info("barcode is not found!");
    }
#endif
    return 0;
}
