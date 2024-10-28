#include "test_image.hpp"



#define TEST_ZBAR   (0)
#define TEST_QUIRC  (0)

#if TEST_QUIRC
#include "quirc.hpp"
static std::string quirc_scan_qrcode(image::Image *img)
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

    quirc_qrcode_result_t  result;
    quirc_scan_qrcode_in_gray((uint8_t *)gray_img->data(), gray_img->width(), gray_img->height(), &result);

    if (img->format() != image::FMT_GRAYSCALE) {
        delete gray_img;
    }

    return result.data;
}
#endif

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
#endif

int test_find_qrcode(image::Image *img) {
    uint64_t t = time::ticks_ms();

#if TEST_ZBAR
    t = time::ticks_ms();
    auto zbar_scan_result = zbar_scan_qrcode(img);
    log::info("zbar scan use %lld ms, fps:%f", time::ticks_ms() - t, 1000.0 / (time::ticks_ms() - t));
    for (int i = 0; i < zbar_scan_result.data.size(); i ++) {
        log::info("zbar scan result: %s", zbar_scan_result.data[i].c_str());
        log::info("zbar corners: (%d, %d) (%d, %d) (%d, %d) (%d, %d)",  zbar_scan_result.corners[i][0],zbar_scan_result.corners[i][1],
                                                                        zbar_scan_result.corners[i][2],zbar_scan_result.corners[i][3],
                                                                        zbar_scan_result.corners[i][4],zbar_scan_result.corners[i][5],
                                                                        zbar_scan_result.corners[i][6],zbar_scan_result.corners[i][7]);
    }
#endif

#if TEST_QUIRC
    t = time::ticks_ms();
    auto quirc_scan_result = quirc_scan_qrcode(img);
    log::info("quirc scan use %lld ms, fps:%f", time::ticks_ms() - t, 1000.0 / (time::ticks_ms() - t));
    log::info("quirc scan result: %s", quirc_scan_result.c_str());
#endif

    t = time::ticks_ms();
    auto res = img->find_qrcodes();
    log::info("find_qrcodes scan use %lld ms, fps:%f", time::ticks_ms() - t, 1000.0 / (time::ticks_ms() - t));
    for (auto &i : res)
    {
        log::info("find_qrcodes scan result: %s", i.payload().c_str());
        std::vector<std::vector<int>> corners = i.corners();
        for (int i = 0; i < 4; i ++) {
            img->draw_line(corners[i][0], corners[i][1], corners[(i + 1) % 4][0], corners[(i + 1) % 4][1], maix::image::Color::from_rgb(0, 255, 0), 2);
        }

        auto rect = i.rect();
        img->draw_rect(rect[0], rect[1], rect[2], rect[3], maix::image::Color::from_rgb(255, 0, 0));
    }
    return 0;
}
