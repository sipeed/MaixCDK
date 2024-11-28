/**
 * @author neucrack@sipeed, lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_tensor.hpp"
#include "maix_log.hpp"
#include "maix_err.hpp"
#include "maix_fs.hpp"
#include "maix_image_def.hpp"
#include "maix_image_color.hpp"
#include "maix_image_obj.hpp"
#include "maix_type.hpp"
#include <stdlib.h>

namespace maix::image
{
    /**
     * QRCodeDetector class
     * @maixpy maix.image.QRCodeDetector
     */
    class QRCodeDetector {
    private:
        void *_param;
    public:
        /**
         * QRCodeDetector constructor.
         * Use npu to accelerate the speed of QR code scanning, note that this object will occupy npu resources
         * @maixpy maix.image.QRCodeDetector.__init__
        */
        QRCodeDetector();

        ~QRCodeDetector();

        /**
         * @brief Finds all qrcodes in the image.
         * @param img The image to find qrcodes.
         * @param roi The region of interest, input in the format of (x, y, w, h), x and y are the coordinates of the upper left corner, w and h are the width and height of roi.
         * default is None, means whole image.
         * @param decoder_type Select the QR code decoding method. Choosing QRCODE_DECODER_TYPE_QUIRC allows for retrieving QR code version, ECC level, mask, data type, and other details,
         * though it may decode slower at lower resolutions. Opting for QRCODE_DECODER_TYPE_ZBAR enables faster decoding at lower resolutions but may slow down at higher resolutions,
         * providing only the QR code content and position information. default is QRCODE_DECODER_TYPE_ZBAR.
         * @return Returns the qrcodes of the image
         * @maixpy maix.image.QRCodeDetector.detect
        */
        std::vector<image::QRCode> detect(image::Image *img, std::vector<int> roi = std::vector<int>(), image::QRCodeDecoderType decoder_type = image::QRCodeDecoderType::QRCODE_DECODER_TYPE_ZBAR);
    };
} // namespace maix::image
