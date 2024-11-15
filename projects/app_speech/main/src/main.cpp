#include "maix_basic.hpp"
#include "maix_vision.hpp"
#include "main.h"
#include "maix_display.hpp"
#include "maix_touchscreen.hpp"
#include "maix_nn_speech.hpp"
#include <atomic>

using namespace maix;

static nn::Speech *speech;
static image::Image *img;
static display::Display *disp;
static i18n::Trans trans;
static thread::Thread *run_thread = nullptr;
static std::atomic<int> function_id(0);
static std::atomic<bool> is_run(false);
static std::atomic<bool> function_change(false);
static std::atomic<bool> run_thread_exit(false);

static std::vector<string> kw_tbl = {
    "xiao3 ai4 tong2 xue2",
    "tian1 mao1 jing1 ling2",
    "tian1 qi4 zen3 me yang4",
};

char* kw_str[3]={
    (char*)"小爱同学",
    (char*)"天猫精灵",
    (char*)"天气怎么样",
};

static std::vector<float> kw_gate = {0.1, 0.1, 0.1};

static void digit_callback(char* data, int len)
{
    char* digit_res = (char*) data;
    log::info("%s\n", digit_res);
    img->draw_rect(0, 50, disp->width(), disp->height()-110, image::COLOR_WHITE, -1);
    img->draw_string(140, 150, digit_res, image::COLOR_BLACK, 3);
    disp->show(*img);
}

static void draw_keyword(image::Image * img)
{
    img->draw_string(50, disp->height()*0.2, (trans.tr("Keyword") + " :").c_str(), image::COLOR_BLACK, 1.5);
    img->draw_string(50, disp->height()*0.35, "1. 小爱同学", image::COLOR_BLACK, 1.2);
    img->draw_string(50, disp->height()*0.50, "2. 天猫精灵", image::COLOR_BLACK, 1.2);
    img->draw_string(50, disp->height()*0.65, "3. 天气怎么样", image::COLOR_BLACK, 1.2);
}

static void kws_callback(std::vector<float> data, int len)
{
    float maxp = 0;
    int maxi = 0;
    for(int i=0; i<len; i++){
        log::info0("\tkw%d: %.3f;", i, data[i]);
        if(data[i] > maxp){
            maxp = data[i];
            maxi = i;
        }
    }
    log::info0("\n");

    if(maxp > 0) {
        img->draw_rect(0, 50, disp->width(), disp->height()-110, image::COLOR_WHITE, -1);
        img->draw_string(310, 170, kw_str[maxi], image::COLOR_BLACK, 1.7);
        draw_keyword(img);
        disp->show(*img);
    }
}

static void lvcsr_callback(std::pair<char*, char*> data, int len)
{
    log::info("PNYS: %s", data.second);
    log::info("HANS: %s", data.first);
    
    img->draw_rect(0, 50, disp->width(), disp->height()-110, image::COLOR_WHITE, -1);
    img->draw_string(10, 70, data.first, image::COLOR_BLACK, 1.5);
    img->draw_rect(0, disp->height()-60, disp->width(), 60, image::Color::from_rgb(17, 17, 17), -1);
    img->draw_string(0.05*disp->width(), disp->height()-45, trans.tr("Clear").c_str(), image::COLOR_WHITE, 1.5);
    img->draw_string(0.317*disp->width(), disp->height()-45, trans.tr("digit").c_str(), image::COLOR_WHITE, 1.5);
    img->draw_string(0.58*disp->width(), disp->height()-45, trans.tr("kws").c_str(), image::COLOR_WHITE, 1.5);
    img->draw_string(0.82*disp->width(), disp->height()-45, trans.tr("lvcsr").c_str(), image::COLOR_GREEN, 1.5);
    disp->show(*img);
}

void run_process(void *arg)
{
    run_thread_exit = false;
    while(!app::need_exit())
    {
        if (function_change == false) {
            is_run = true;
            speech->run(1);
            is_run = false;
        }
        time::sleep_ms(10);
    }
    run_thread_exit = true;
}

int _main(int argc, char* argv[])
{
    err::Err e = trans.load("locales");
    err::check_raise(e, "load translation yamls failed");

    disp = new display::Display();

    touchscreen::TouchScreen ts;
    int ts_x = 0, ts_y = 0;
    bool ts_pressed = false;
    bool is_pressed = false;

    img = new image::Image(disp->width(), disp->height(), image::Format::FMT_RGB888);
    if (!img)
    {
        log::error("create image failed");
        return err::ERR_NO_MEM;
    }

    image::Image *ret_img = image::load("./assets/ret.png", image::Format::FMT_RGB888);
    if (!ret_img)
    {
        log::error("load ret image failed");
        return err::ERR_NO_MEM;
    }

    image::load_font("sourcehansans", "/maixapp/share/font/SourceHanSansCN-Regular.otf", 20);
    image::set_default_font("sourcehansans");
    img->draw_rect(0, 0, disp->width(), disp->height(), image::COLOR_WHITE, -1);
    img->draw_rect(0, 0, disp->width(), 50, image::Color::from_rgb(17, 17, 17), -1);
    img->draw_rect(0, disp->height()-60, disp->width(), 60, image::Color::from_rgb(17, 17, 17), -1);

    img->draw_string(0.05*disp->width(), disp->height()-45, trans.tr("Clear").c_str(), image::COLOR_WHITE, 1.5);
    img->draw_string(0.317*disp->width(), disp->height()-45, trans.tr("digit").c_str(), image::COLOR_GREEN, 1.5);
    img->draw_string(0.58*disp->width(), disp->height()-45, trans.tr("kws").c_str(), image::COLOR_WHITE, 1.5);
    img->draw_string(0.82*disp->width(), disp->height()-45, trans.tr("lvcsr").c_str(), image::COLOR_WHITE, 1.5);

    if(strcmp(i18n::get_locale().c_str(), "zh") == 0) {
        img->draw_string(0.42*disp->width(), 15, trans.tr("Speech Recognition").c_str(), image::COLOR_WHITE);
    } else {
        img->draw_string(0.34*disp->width(), 15, trans.tr("Speech Recognition").c_str(), image::COLOR_WHITE);
    }

    img->draw_image(0, 9, *ret_img);
    disp->show(*img);

    // init asr lib
    speech = new nn::Speech("/root/models/am_3332_192_int8.mud");
    speech->init(nn::SpeechDevice::DEVICE_MIC, "hw:0,0");
    speech->digit(640, digit_callback);

    function_id = 1;
    run_thread = new thread::Thread(run_process, nullptr);
    run_thread->detach();

    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    while(!app::need_exit())
    {
        ts.read(ts_x, ts_y, ts_pressed);
        if (ts_pressed && !is_pressed) {
            is_pressed = true;
            if (ts_x < 40 + 60 && ts_y < 40) {
                app::set_exit_flag(true);
                break;
            } else if (ts_x > 0 && ts_x < 0.25*disp->width() && ts_y > 308) {
                log::info("clear\n");
                speech->clear();
                if (function_id == 2) {
                    img->draw_rect(0, 50, disp->width(), disp->height()-110, image::COLOR_WHITE, -1);
                    draw_keyword(img);
                    disp->show(*img);
                }
            } else if (ts_x > 138 && ts_x < 0.5*disp->width() && ts_y > 308 && function_id != 1) {
                function_id = 1;
                function_change = true;
            } else if (ts_x > 276 && ts_x < 0.75*disp->width() && ts_y > 308 && function_id != 2) {
                function_id = 2;
                function_change = true;
            } else if (ts_x > 0.75*disp->width() && ts_y > 308 && function_id != 3) {
                function_id = 3;
                function_change = true;
            }
        } else if (!ts_pressed && is_pressed) {
            is_pressed = false;
        }

        if (function_change && is_run == false) {
            img->draw_rect(0, disp->height()-60, disp->width(), 60, image::Color::from_rgb(17, 17, 17), -1);
            img->draw_string(0.05*disp->width(), disp->height()-45, trans.tr("Clear").c_str(), image::COLOR_WHITE, 1.5);
            img->draw_string(0.317*disp->width(), disp->height()-45, trans.tr("digit").c_str(), image::COLOR_WHITE, 1.5);
            img->draw_string(0.58*disp->width(), disp->height()-45, trans.tr("kws").c_str(), image::COLOR_WHITE, 1.5);
            img->draw_string(0.82*disp->width(), disp->height()-45, trans.tr("lvcsr").c_str(), image::COLOR_WHITE, 1.5);
            img->draw_rect(0, 50, disp->width(), disp->height()-110, image::COLOR_WHITE, -1);

            switch (function_id)
            {
                case 1:
                    log::info("digit\n");
                    speech->clear();
                    speech->digit(65535, digit_callback);
                    speech->dec_deinit(nn::SpeechDecoder::DECODER_KWS);
                    speech->dec_deinit(nn::SpeechDecoder::DECODER_LVCSR);
                    img->draw_string(0.317*disp->width(), disp->height()-45, trans.tr("digit").c_str(), image::COLOR_GREEN, 1.5);
                    function_change = false;
                    break;
                case 2: 
                    log::info("kws\n");
                    speech->clear();
                    speech->dec_deinit(nn::SpeechDecoder::DECODER_DIG);
                    speech->dec_deinit(nn::SpeechDecoder::DECODER_LVCSR);
                    speech->kws(kw_tbl, kw_gate, kws_callback);
                    draw_keyword(img);
                    img->draw_string(0.58*disp->width(), disp->height()-45, trans.tr("kws").c_str(), image::COLOR_GREEN, 1.5);
                    function_change = false;
                    break;
                case 3:
                    log::info("lvcsr\n");
                    speech->clear();
                    speech->dec_deinit(nn::SpeechDecoder::DECODER_DIG);
                    speech->dec_deinit(nn::SpeechDecoder::DECODER_KWS);
                    static const std::string lm_path = "/root/models/lmS/";
                    speech->lvcsr(lm_path + "lg_6m.sfst", lm_path + "lg_6m.sym", 
                                lm_path + "phones.bin", lm_path + "words_utf.bin", 
                                lvcsr_callback);
                    img->draw_string(0.82*disp->width(), disp->height()-45, trans.tr("lvcsr").c_str(), image::COLOR_GREEN, 1.5);
                    function_change = false;
                    break;
                default: break;
            }
            disp->show(*img);
        }
    }

    log::info("wait run thread exit");
    while(!run_thread_exit)
    {
        time::sleep_ms(20);
    }
    log::info("wait run thread exit done");

    if (run_thread != nullptr) {
        delete run_thread;
        run_thread = nullptr;
    }

    delete speech;
    delete img;
    delete disp;

    return err::ERR_NONE;
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