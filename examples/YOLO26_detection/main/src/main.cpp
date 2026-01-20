/**
 * YOLO26 Detection Example
 */

#include "maix_nn_yolo26.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "maix_app.hpp"

using namespace maix;

int main(int argc, char *argv[])
{
    std::string model_path = "/root/models/yolo26n.mud";
    if (argc > 1)
        model_path = argv[1];

    // Initialize detector
    // @maixpy detector = nn.YOLO26(model="/root/models/yolo26n.mud", dual_buff=True)
    nn::YOLO26 detector(model_path, true);

    // Initialize camera with model input size
    // @maixpy cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
    camera::Camera cam(detector.input_width(), detector.input_height(), detector.input_format());
    
    // Initialize display
    // @maixpy disp = display.Display()
    display::Display disp;

    // FPS statistics
    int detect_count = 0;
    uint64_t total_detect_time = 0;

    // @maixpy while not app.need_exit():
    while (!app::need_exit())
    {
        // Read image
        // @maixpy img = cam.read()
        image::Image *img = cam.read();
        if (!img)
            continue;

        // Detect objects
        // @maixpy objs = detector.detect(img, conf_th=0.5, iou_th=0.45)
        uint64_t t0 = time::ticks_ms();
        std::vector<nn::Object> *objs = detector.detect(*img, 0.5, 0.45);
        uint64_t t1 = time::ticks_ms();

        // Calculate FPS
        uint64_t detect_time = t1 - t0;
        total_detect_time += detect_time;
        detect_count++;
        float detect_fps = (total_detect_time > 0) ? (float)detect_count * 1000.0f / total_detect_time : 0.0f;

        // Draw results
        // @maixpy for obj in objs:
        for (const auto &obj : *objs)
        {
            // @maixpy img.draw_rect(obj.x, obj.y, obj.w, obj.h, color=image.COLOR_RED)
            img->draw_rect(obj.x, obj.y, obj.w, obj.h, image::COLOR_RED, 2);

            // @maixpy msg = f'{detector.labels[obj.class_id]}: {obj.score:.2f}'
            // @maixpy img.draw_string(obj.x, obj.y, msg, color=image.COLOR_RED)
            char msg[64];
            snprintf(msg, sizeof(msg), "%s: %.2f", detector.labels[obj.class_id].c_str(), obj.score);
            img->draw_string(obj.x, std::max(0, obj.y - 15), msg, image::COLOR_RED);
        }

        // Draw FPS
        char fps_str[64];
        snprintf(fps_str, sizeof(fps_str), "Detect: %.1f FPS", detect_fps);
        img->draw_string(10, 10, fps_str, image::COLOR_GREEN, 1.5);

        // Show image
        // @maixpy disp.show(img)
        disp.show(*img);

        // Log
        log::info("检测耗时: %lu ms | 检测FPS: %.1f | 检测数: %zu",
                  detect_time, detect_fps, objs->size());

        // Clean up
        delete objs;
        delete img;
    }

    return 0;
}