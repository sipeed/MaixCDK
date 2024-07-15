#include "stdio.h"
#include "main.h"
#include "maix_basic.hpp"
#include "maix_util.hpp"
#include "maix_image.hpp"
#include "maix_time.hpp"
#include "maix_display.hpp"
#include "maix_rtsp.hpp"
#include "maix_camera.hpp"
#include "maix_nn_yolov5.hpp"
#include "csignal"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "queue"
#include "sophgo_middleware.hpp"
#include "rtsp_server.h"

void nv21_fill_rect(uint8_t *yuv, int width, int height, int x1, int y1, int rect_w, int rect_h, uint8_t y_value, uint8_t u_value, uint8_t v_value)
{
	for (int h = y1; h < y1 + rect_h; h++) {
		for (int w = x1; w < x1 + rect_w; w++) {
			int index = h * width + w;
			yuv[index] = y_value;
		}
	}

	for (int h = y1; h < y1 + rect_h; h+=2) {
		for (int w = x1; w < x1 + rect_w; w+=2) {
			int uv_index = width * height + (h / 2) * width + w;
			yuv[uv_index] = v_value;
			yuv[uv_index + 1] = u_value;
		}
	}
}

void nv21_draw_rectangle(uint8_t *yuv, int width, int height, int x, int y, int rect_width, int rect_height, uint8_t y_value, uint8_t u_value, uint8_t v_value, int thickness) {

	int tmp_x = 0, tmp_y = 0, tmp_rect_width = 0, tmp_rect_height = 0;
	x = (x % 2 == 0) ? x : x - 1;
	x = x < 0 ? 0 : x;
	y = (y % 2 == 0) ? y : y - 1;
	y = y < 0 ? 0 : y;
	rect_width = (rect_width % 2 == 0) ? rect_width : rect_width - 1;
	rect_width = (rect_width + x) < width ? rect_width : width - x;
	rect_width = rect_width < 0 ? 0 : rect_width;
	rect_height = (rect_height % 2 == 0) ? rect_height : rect_height - 1;
	rect_height = (rect_height + y) < height ? rect_height : height - y;
	rect_height = rect_height < 0 ? 0 : rect_height;
	thickness = (thickness % 2 == 0) ? thickness : thickness - 1;
	thickness = thickness < 0 ? 0 : thickness;

	tmp_x = x;
	tmp_y = y;
	tmp_rect_width = rect_width;
	tmp_rect_height = thickness;
	nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// upper

	tmp_x = x;
	tmp_y = (y + rect_height - thickness) >= 0 ? (y + rect_height - thickness) : 0;
	tmp_rect_width = rect_width;
	tmp_rect_height = thickness;
	nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// lower

	tmp_x = x;
	tmp_y = (y + thickness) < height ? (y + thickness) : height - thickness;
	tmp_rect_width = thickness;
	tmp_rect_height = rect_height - thickness * 2;
	nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// left


	tmp_x = (x + rect_width - thickness) < width ? (x + rect_width - thickness) : width - thickness;
	tmp_y = (y + thickness) < height ? (y + thickness) : height - thickness;
	tmp_rect_width = thickness;
	tmp_rect_height = rect_height - thickness * 2;
	nv21_fill_rect(yuv, width, height, tmp_x, tmp_y, tmp_rect_width, tmp_rect_height, y_value, u_value, v_value);		// right
}

#if 0
#define MAX_SAVE_TIME_MS (10 * 1000)
typedef struct {
    uint64_t timestamp;
    uint8_t *buff;
    size_t buff_size;
} video_buff_t;

static std::queue<video_buff_t> video_queue;

// Init video buffer
static void _video_buffer_init(void)
{

}

// Push video buffer
static void _video_buffer_push(uint8_t *buff, size_t buff_size)
{
    while (video_queue.size() > 0) {
        video_buff_t back = video_queue.back();
        if (back.timestamp + MAX_SAVE_TIME_MS > time::ticks_ms()) {
            if (back.buff != NULL) {
                free(back.buff);
                back.buff = NULL;
            }
            video_queue.pop();
        } else {
            break;
        }
    }

    video_buff_t video_buff;
    video_buff.timestamp = time::ticks_ms();
    video_buff.buff = (uint8_t *)malloc(buff_size);
    err::check_null_raise(video_buff.buff, "malloc video buff failed");
    memcpy(video_buff.buff, buff, buff_size);
    video_buff.buff_size = buff_size;
    video_queue.push(video_buff);printf("push the video is compileted, video queue size: %d\r\n", video_queue.size());
}

// Pop video buffer and save to file
static void _video_buffer_pop_and_save(char *path)
{
    FILE *f = fopen(path, "wb");
    err::check_null_raise(f, "open file failed");
    while (video_queue.size() > 0) {
        video_buff_t back = video_queue.back();
        if (back.buff != NULL) {
            size_t len = fwrite(back.buff, 1, back.buff_size, f);
            if (len != back.buff_size) {
                log::error("write file failed, need %d write %d\r\n", back.buff_size, len);
            }
            free(back.buff);
            back.buff = NULL;
        }
        video_queue.pop();
    };
}
#endif

using namespace maix;

int _main(int argc, char* argv[])
{
    uint64_t t = time::ticks_ms();
    uint64_t loop_ms = time::ticks_ms();
    int cnt = 0;
    std::string model_type = "unknown";
    std::string help = "Usage: " + std::string(argv[0]) + " mud_model_path <image_path>";
    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }
    const char *model_path = argv[1];

    camera::Camera cam = camera::Camera(1920, 1080, image::Format::FMT_YVU420SP);
    display::Display disp = display::Display();
    rtsp::Rtsp rtsp = rtsp::Rtsp();
    // rtsp.bind_camera(&cam);

    log::info("%s\r\n", rtsp.get_url().c_str());
    rtsp.start();

    nn::YOLOv5 detector;
    err::Err e = detector.load(model_path);
    err::check_raise(e, "load model failed");
    log::info("load yolov5 model %s success\r\n", model_path);
    maix::image::Size input_size = detector.input_size();
    log::info("model size is %dx%d, format is %d\r\n", input_size.width(), input_size.height(), detector.input_format());

    int cam2_width = input_size.width();
    int cam2_height = input_size.height();
    if (cam2_width * cam.height() / cam.width() > cam2_height) {
        cam2_height = cam2_width * cam.height() / cam.width();
    } else {
        cam2_width = cam2_height * cam.width() / cam.height();
    }
    cam2_width = ((cam2_width / 64) + 1) * 64;
    cam2_height = cam2_width * cam.height() / cam.width();
    camera::Camera *cam2 = cam.add_channel(cam2_width, cam2_height, detector.input_format());
    err::check_null_raise(cam2, "add cam channel failed");
    log::info("sub camera width:%d height:%d\r\n", cam2->width(), cam2->height());

    loop_ms = time::ticks_ms();
    int rgn_en = 0, rgn_x = 0, rgn_y = 0, rgn_w = 0, rgn_h = 0;
    while(!app::need_exit()) {
        // {
            int vi_ch = 0, enc_ch = 1;
            void *data;
            int data_size, width, height, format;
            rtsp.update_timestamp();
            uint64_t timestamp = rtsp.get_timestamp();

            mmf_h265_stream_t stream;
            if (!mmf_enc_h265_pop(enc_ch, &stream)) {
                int stream_size = 0;
                for (int i = 0; i < stream.count; i ++) {
                    log::info("[%d] stream.data:%p stream.len:%d\n", i, stream.data[i], stream.data_size[i]);
                    stream_size += stream.data_size[i];
                }

                if (stream.count > 1) {
                    uint8_t *stream_buffer = (uint8_t *)malloc(stream_size);
                    if (stream_buffer) {
                        int copy_length = 0;
                        for (int i = 0; i < stream.count; i ++) {
                            memcpy(stream_buffer + copy_length, stream.data[i], stream.data_size[i]);
                            copy_length += stream.data_size[i];
                        }
                        rtsp_send_h265_data(timestamp, stream_buffer, copy_length);
                        free(stream_buffer);
                    } else {
                        log::warn("malloc failed!\r\n");
                    }
                } else if (stream.count == 1) {
                    rtsp_send_h265_data(timestamp, (uint8_t *)stream.data[0], stream.data_size[0]);
                }

                if (mmf_enc_h265_free(enc_ch)) {
                    log::warn("mmf_enc_h265_free failed\n");
                    continue;
                }
            }

            if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
                continue;
            }

            if (rgn_en) {
                nv21_draw_rectangle((uint8_t *)data, width, height, rgn_x, rgn_y, rgn_w, rgn_h, 150, 53, 1, 2);
            }

            t = time::ticks_ms();
            image::Image *img = cam2->read();
            err::check_null_raise(img, "read camera failed");

            if (mmf_enc_h265_push(enc_ch, (uint8_t *)data, width, height, format)) {
                log::warn("mmf_enc_h265_push failed\n");
                continue;
            }

            mmf_vi_frame_free(vi_ch);
        // }

        // {
            int crop_oft_x = (img->width() - input_size.width()) / 2;
            int crop_oft_y = (img->height() - input_size.height()) / 2;
            image::Image *crop_img = img->crop(crop_oft_x, crop_oft_y, input_size.width(), input_size.height());
            err::check_null_raise(crop_img, "crop image failed");

            std::vector<nn::Object> *result = detector.detect(*crop_img);
            for (auto &r : *result)
            {
                if (detector.labels[r.class_id] == "person") {
                    log::info("result: %s", r.to_str().c_str());
                    img->draw_rect(r.x + crop_oft_x, r.y + crop_oft_y, r.w, r.h, maix::image::Color::from_rgb(255, 0, 0));
                    img->draw_string(r.x + crop_oft_x, r.y + crop_oft_y, detector.labels[r.class_id], maix::image::Color::from_rgb(255, 0, 0));
                    // t = time::ticks_ms();
                    float w_scale = (float)cam.width() / img->width();
                    float h_scale = (float)cam.height() / img->height();
                    rgn_x = (r.x + crop_oft_x) * w_scale;
                    rgn_y = (r.y + crop_oft_y) * h_scale;
                    rgn_w = r.w * w_scale;
                    rgn_h = r.h * h_scale;
                    rgn_en = 1;
                    break;
                } else {
                    rgn_en = 0;
                }
            }

            disp.show(*img);
            delete result;
            delete crop_img;
            delete img;
        // }
        log::info("time: %d ms", time::ticks_ms() - loop_ms);
        loop_ms = time::ticks_ms();

    }

    delete cam2;

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
