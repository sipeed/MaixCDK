
#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "maix_nn_yolov8.hpp"
#include "maix_nn_face_landmarks.hpp"
#include "maix_nn_classifier.hpp"
#include "main.h"

using namespace maix;

int mode_fileter(int new_idx, size_t labels_num)
{
    static std::vector<int> data(10, -1);
    std::vector<int> count(labels_num, 0);
    static int p = 0;
    data[p++] = new_idx;
    for(size_t i=0; i<data.size(); ++i)
    {
        if(data[i] >= 0)
            ++count[data[i]];
    }
    int max_count = count[0];
    int max_idx = 0;
    for(size_t i=1; i<labels_num; ++i)
    {
        if (count[i] > max_count)
        {
            max_count = count[i];
            max_idx = i;
        }
    }
    return max_idx;
}


int _main(int argc, char *argv[])
{
    log::info("Program start");
    int ret = 0;
    maix::image::Format img_fmt = maix::image::FMT_RGB888;
    char tmp_chars[64] = {0};

    std::string help = "Usage: " + std::string(argv[0]) + " detect_model_path emotion_model_path <image_path>";

    if (argc < 2)
    {
        log::info(help.c_str());
        return -1;
    }

    const char *detect_model_path = argv[1];
    const char *emotion_model_path = argv[2];
    float conf_threshold = 0.5;
    float iou_threshold = 0.45;
    float crop_scale = 1.2;
    float emotion_threshold = 0.5;

    nn::YOLOv8 detector(detect_model_path, false);
    log::info("load yolov8 model %s success", detect_model_path);
    nn::FaceLandmarks landmarks_detector;
    nn::Classifier classifier(emotion_model_path, false);
    log::info("load emotion model %s success", emotion_model_path);

    // for draw result info
    int max_labels_length = 0;
    for (size_t j = 0; j < classifier.labels.size(); ++j)
    {
        image::Size size = image::string_size(classifier.labels[j]);
        if (size.width() > max_labels_length)
            max_labels_length = size.width();
    }

    if (argc >= 4)
    {
        const char *img_path = argv[3];
        log::info("load image now");
        maix::image::Image *img = maix::image::load(img_path, img_fmt);
        err::check_null_raise(img, "load image " + std::string(img_path) + " failed");
        log::info("load image %s success: %s", img_path, img->to_str().c_str());
        if (img->width() != detector.input_size().width() || img->height() != detector.input_size().height())
        {
            log::warn("image size not match model input size, will auto resize from %dx%d to %dx%d", img->width(), img->height(), detector.input_size().width(), detector.input_size().height());
        }
        int max_score_length = img->width() / 4;
        log::info("detect now");
        nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold);
        std::vector<std::vector<std::pair<int, float>> *> results;
        std::vector<int> idxes;
        maix::image::Image *img_std_first = nullptr;
        int i = 0;
        for (auto &r : *result)
        {
            maix::image::Image *img_std = landmarks_detector.crop_image(*img, r->x, r->y, r->w, r->h, r->points, classifier.input_width(), classifier.input_height(), crop_scale);
            if (img_std)
            {
                maix::image::Image *img_std_gray = img_std->to_format(image::Format::FMT_GRAYSCALE);
                if (i == 0)
                    img_std_first = img_std;
                else
                    delete img_std;
                std::vector<std::pair<int, float>> *res = classifier.classify(*img_std_gray);
                results.push_back(res);
                idxes.push_back(i);
                delete img_std_gray;
            }
            ++i;
        }
        i = 0;
        for (auto r : results)
        {
            if (i == 0)
            {
                img->draw_image(0, 0, *img_std_first);
                for (size_t j = 0; j < classifier.labels.size(); ++j)
                {
                    int idx = r->at(j).first;
                    float score = r->at(j).second;
                    img->draw_string(0, img_std_first->height() + idx * 16, classifier.labels[idx], image::COLOR_WHITE);
                    img->draw_rect(max_labels_length, img_std_first->height() + idx * 16, score * max_score_length, 8, score >= emotion_threshold ? image::COLOR_GREEN : image::COLOR_RED, -1);
                    snprintf(tmp_chars, sizeof(tmp_chars), "%.1f", score);
                    img->draw_string(max_labels_length + score * max_score_length + 2, img_std_first->height() + idx * 16, tmp_chars, image::COLOR_RED);
                }
                delete img_std_first;
            }
            auto color = r->at(0).second >= emotion_threshold ? image::COLOR_GREEN : image::COLOR_RED;
            nn::Object &obj = result->at(idxes[i]);
            img->draw_rect(obj.x, obj.y, obj.w, obj.h, color, 1);
            snprintf(tmp_chars, sizeof(tmp_chars), "%s: %.1f", classifier.labels[r->at(0).first].c_str(), r->at(0).second);
            img->draw_string(obj.x, obj.y, tmp_chars, color);
            ++i;
            delete r;
        }
        img->save("result.jpg");
        log::info("saved result image to result.jpg");
        delete img;
        delete result;
    }
    else
    {
        log::info("open camera now");
        maix::image::Size input_size = detector.input_size();
        camera::Camera cam = camera::Camera(input_size.width(), input_size.height(), detector.input_format());
        log::info("open camera success");
        display::Display disp = display::Display();
        int max_score_length = cam.width() / 4;
        while (!app::need_exit())
        {
            uint64_t t = time::ticks_ms();
            maix::image::Image *img = cam.read();
            err::check_null_raise(img, "read camera failed");
            uint64_t t2 = time::ticks_ms();
            nn::Objects *result = detector.detect(*img, conf_threshold, iou_threshold, image::FIT_CONTAIN, 0.5, 1);
            std::vector<std::vector<std::pair<int, float>> *> results;
            std::vector<int> idxes;
            maix::image::Image *img_std_first = nullptr;
            int i = 0;
            for (auto &r : *result)
            {
                maix::image::Image *img_std = landmarks_detector.crop_image(*img, r->x, r->y, r->w, r->h, r->points, classifier.input_width(), classifier.input_height(), crop_scale);
                if (img_std)
                {
                    maix::image::Image *img_std_gray = img_std->to_format(image::Format::FMT_GRAYSCALE);
                    if (i == 0)
                        img_std_first = img_std;
                    else
                        delete img_std;
                    std::vector<std::pair<int, float>> *res = classifier.classify(*img_std_gray);
                    results.push_back(res);
                    idxes.push_back(i);
                    delete img_std_gray;
                }
                ++i;
            }
            uint64_t t3 = time::ticks_ms();
            i = 0;
            for (auto r : results)
            {
                // draw fisrt face detailed info
                if (i == 0)
                {
                    img->draw_image(0, 0, *img_std_first);
                    for (size_t j = 0; j < classifier.labels.size(); ++j)
                    {
                        int idx = r->at(j).first;
                        float score = r->at(j).second;
                        img->draw_string(0, img_std_first->height() + idx * 16, classifier.labels[idx], image::COLOR_WHITE);
                        img->draw_rect(max_labels_length, img_std_first->height() + idx * 16, score * max_score_length, 8, score >= emotion_threshold ? image::COLOR_GREEN : image::COLOR_RED, -1);
                        snprintf(tmp_chars, sizeof(tmp_chars), "%.1f", score);
                        img->draw_string(max_labels_length + score * max_score_length + 2, img_std_first->height() + idx * 16, tmp_chars, image::COLOR_RED);
                    }
                    delete img_std_first;
                }
                // draw on all face
                auto color = r->at(0).second >= emotion_threshold ? image::COLOR_GREEN : image::COLOR_RED;
                nn::Object &obj = result->at(idxes[i]);
                img->draw_rect(obj.x, obj.y, obj.w, obj.h, color, 1);
                snprintf(tmp_chars, sizeof(tmp_chars), "%s: %.1f", classifier.labels[r->at(0).first].c_str(), r->at(0).second);
                img->draw_string(obj.x, obj.y, tmp_chars, color);
                ++i;
                delete r;
            }
            disp.show(*img);
            delete img;
            delete result;
            log::info("time: all %d ms, detect %d ms", time::ticks_ms() - t, t3 - t2);
        }
    }

    log::info("Program exit");

    return ret;
}

int main(int argc, char *argv[])
{
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig)
           { app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
