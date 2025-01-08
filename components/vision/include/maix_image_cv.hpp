#pragma once

#include "maix_image.hpp"
#include "opencv2/opencv.hpp"

namespace maix::image
{

    /**
     * Convert image::Image object to opencv Mat object
     * @param[in] img image::Image object
     * @param[out] mat cv::Mat object, output image, will be allocated by this function.
     * @param[in] ensure_bgr auto convert to BGR888 or BGRA8888 if img format is not BGR or BGRA, if set to false, will not auto convert and directly use img's data, default false.
     *                       If copy is false, ensure_bgr always be false.
     * @param[in] copy Whether alloc new image and copy data or not, if ensure_bgr and img is not bgr or bgra format, always copy,
     *        if not copy, array object will directly use img's data buffer, will faster but change array will affect img's data, default true.
     * @maixcdk maix.image.image2cv
     */
    err::Err image2cv(image::Image &img, cv::Mat &mat, bool ensure_bgr = false, bool copy = true);


    /**
     * OpenCV Mat object to Image object
     * @param mat cv::Mat image object.
     * @param bgr if set bgr, the return image will be marked as BGR888 or BGRA8888 format, grayscale will ignore this arg.
     * @param copy if true, will alloc new buffer and copy data, else will directly use array's data buffer, default true.
     *        Use this arg carefully, when set to false, ther array MUST keep alive until we don't use the return img of this func, or will cause program crash.
     * @return Image object
     * @maixcdk maix.image.cv2image
     */
    image::Image *cv2image(cv::Mat &mat, bool bgr = true, bool copy = true);

}
