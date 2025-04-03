/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.6.7: Add yolov8 support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>
#include "maix_nn_yolo11.hpp"

namespace maix::nn
{
    /**
     * YOLOv8 class
     * @maixpy maix.nn.YOLOv8
     */
    class YOLOv8 : public YOLO11
    {
    public:
        /**
         * Constructor of YOLOv8 class
         * @param model model path, default empty, you can load model later by load function.
         * @param[in] dual_buff prepare dual input output buffer to accelarate forward, that is, when NPU is forwarding we not wait and prepare the next input buff.
         *                      If you want to ensure every time forward output the input's result, set this arg to false please.
         *                      Default true to ensure speed.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.YOLOv8.__init__
         * @maixcdk maix.nn.YOLOv8.YOLOv8
         */
        YOLOv8(const string &model = "", bool dual_buff = true)
        : YOLO11(model, dual_buff)
        {
            type_str = "yolov8"
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.YOLOv8.load
         */
        err::Err load(const string &model)
        {
            return YOLO11::load(model);
        }

        /**
         * Detect objects from image
         * @param img Image want to detect, if image's size not match model input's, will auto resize with fit method.
         * @param conf_th Confidence threshold, default 0.5.
         * @param iou_th IoU threshold, default 0.45.
         * @param fit Resize method, default image.Fit.FIT_CONTAIN.
         * @param keypoint_th keypoint threshold, default 0.5, only for yolov8-pose model.
         * @param sort sort result according to object size, default 0 means not sort, 1 means bigger in front, -1 means smaller in front.
         * @throw If image format not match model input format, will throw err::Exception.
         * @return Object list. In C++, you should delete it after use.
         *         If model is yolov8-pose, object's points have value, and if points' value < 0 means that point is invalid(conf < keypoint_th).
         * @maixpy maix.nn.YOLOv8.detect
         */
        nn::Objects *detect(image::Image &img, float conf_th = 0.5, float iou_th = 0.45, maix::image::Fit fit = maix::image::FIT_CONTAIN, float keypoint_th = 0.5, int sort = 0)
        {
            return YOLO11::detect(img, conf_th, iou_th, fit, keypoint_th, sort);
        }

        /**
         * Get model input size
         * @return model input size
         * @maixpy maix.nn.YOLOv8.input_size
         */
        image::Size input_size()
        {
            return YOLO11::input_size();
        }

        /**
         * Get model input width
         * @return model input size of width
         * @maixpy maix.nn.YOLOv8.input_width
         */
        int input_width()
        {
            return YOLO11::input_width();
        }

        /**
         * Get model input height
         * @return model input size of height
         * @maixpy maix.nn.YOLOv8.input_height
         */
        int input_height()
        {
            return YOLO11::input_height();
        }

        /**
         * Get input image format
         * @return input image format, image::Format type.
         * @maixpy maix.nn.YOLOv8.input_format
         */
        image::Format input_format()
        {
            return YOLO11::input_format();
        }

        /**
         * Draw pose keypoints on image
         * @param img image object, maix.image.Image type.
         * @param points keypoits, int list type, [x, y, x, y ...]
         * @param radius radius of points.
         * @param color color of points.
         * @param colors assign colors for points, list type, element is image.Color object.
         * @param body true, if points' length is 17*2 and body is ture, will draw lines as human body, if set to false won't draw lines, default true.
         * @param close connect all points to close a polygon, default false.
         * @maixpy maix.nn.YOLOv8.draw_pose
         */
        void draw_pose(image::Image &img, std::vector<int> points, int radius = 4, image::Color color = image::COLOR_RED, const std::vector<image::Color> &colors = std::vector<image::Color>(), bool body = true, bool close = false)
        {
            YOLO11::draw_pose(img, points, radius, color, colors, body,close);
        }

        /**
         * Draw segmentation on image
         * @param img image object, maix.image.Image type.
         * @param seg_mask segmentation mask image by detect method, a grayscale image
         * @param threshold only mask's value > threshold will be draw on image, value from 0 to 255.
         * @maixpy maix.nn.YOLOv8.draw_seg_mask
         */
        void draw_seg_mask(image::Image &img, int x, int y, image::Image &seg_mask, int threshold = 127)
        {
            YOLO11::draw_seg_mask(img, x, y, seg_mask, threshold);
        }

    public:
        /**
         * Labels list
         * @maixpy maix.nn.YOLOv8.labels
         */
        //std::vector<string> labels;

        /**
         * Label file path
         * @maixpy maix.nn.YOLOv8.label_path
         */
        //std::string label_path;

        /**
         * Get mean value, list type
         * @maixpy maix.nn.YOLOv8.mean
         */
        //std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.YOLOv8.scale
         */
        //std::vector<float> scale;
    };

} // namespace maix::nn
