/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.11.27: Add framework, create this file.
 */

#include "maix_image.hpp"
#include "maix_image_obj.hpp"
#include "maix_image_extra.hpp"

using namespace maix;

namespace maix::image
{
    QRCodeDetector::QRCodeDetector() {

    }

    QRCodeDetector::~QRCodeDetector() {

    }

    std::vector<image::QRCode> QRCodeDetector::detect(image::Image *img, std::vector<int> roi, image::QRCodeDecoderType decoder_type)
    {
        (void)img;
        (void)roi;
        (void)decoder_type;
        return std::vector<image::QRCode>();
    }
}