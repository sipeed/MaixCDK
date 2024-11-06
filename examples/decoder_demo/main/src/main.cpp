#include "maix_vision.hpp"
#include "maix_audio.hpp"
#include "main.h"
#include "sophgo_middleware.hpp"
using namespace maix;
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <list>
void helper(void)
{
    printf( "========================\r\n"
            "Intput param:\r\n"
            "0 <filepath> <seek_s> : decode the video and display. example: ./decoder_demo 0 test.h264\r\n"
            "========================\r\n");
}

int _main(int argc, char* argv[])
{
    if (argc < 2) {
        helper();
        return -1;
    }

    int cmd = atoi(argv[1]);
    switch (cmd)
    {
    case 0: {
        if (argc < 3) {
            helper();
            return -1;
        }

        std::string filepath = argv[2];
        double seek_s = 0.0;
        if (argc > 3) seek_s = atof(argv[3]);
        video::Decoder decoder = video::Decoder(filepath);
        display::Display disp = display::Display();
        audio::Player *p = NULL;
        (void)p;
        uint64_t loop_ms = time::ticks_ms(), last_us = time::ticks_us();
        log::info("filepath:%s seek_s:%f", filepath.c_str(), seek_s);
        log::info("has video:%d has audio:%d resolution:%dx%d bitrate:%d duration:%.2f s fps:%d seek_s:%f", decoder.has_video(), decoder.has_audio(), decoder.width(), decoder.height(), decoder.bitrate(), decoder.duration(), decoder.fps(), seek_s);
        if (decoder.has_video()) {
            err::check_bool_raise(decoder.seek(seek_s) >= 0, "decoder.seek failed");
            log::info("decoder.seek:%f", decoder.seek());
        }

        if (decoder.has_audio()) {
            log::info("audio_sample_rate:%d audio_format:%d audio_channels:%d", decoder.audio_sample_rate(), decoder.audio_format(),decoder.audio_channels());
            p = new audio::Player("", decoder.audio_sample_rate(), decoder.audio_format(), decoder.audio_channels());
        }

        while (!app::need_exit()) {
            video::Context *ctx = decoder.decode_video();
            if (!ctx) {
                log::info("decode video over, set seek to 0");
                decoder.seek(0);
                continue;
            }
            if (ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                image::Image *img = ctx->image();
                disp.show(*img);

                while (time::ticks_us() - last_us < ctx->duration_us()) {
                    time::sleep_ms(1);
                }
                last_us = time::ticks_us();

                delete img;
                delete ctx;
            }
            log::info("loop time:%.2d ms", (time::ticks_ms() - loop_ms));
            loop_ms = time::ticks_ms();
        }
        break;
    }
    case 1: {
        if (argc < 3) {
            helper();
            return -1;
        }

        std::string filepath = argv[2];
        double seek_s = 0.0;
        if (argc > 3) seek_s = atof(argv[3]);
        video::Decoder decoder = video::Decoder(filepath);
        audio::Player *p = NULL;
        uint64_t loop_ms = time::ticks_ms(), last_us = time::ticks_us();
        log::info("filepath:%s seek_s:%f", filepath.c_str(), seek_s);
        log::info("has video:%d has audio:%d resolution:%dx%d bitrate:%d duration:%.2f s fps:%d seek_s:%f", decoder.has_video(), decoder.has_audio(), decoder.width(), decoder.height(), decoder.bitrate(), decoder.duration(), decoder.fps(), seek_s);
        err::check_bool_raise(decoder.has_audio(), "no audio");
        log::info("audio_sample_rate:%d audio_format:%d audio_channels:%d", decoder.audio_sample_rate(), decoder.audio_format(),decoder.audio_channels());
        p = new audio::Player("", decoder.audio_sample_rate(), decoder.audio_format(), decoder.audio_channels());

        while (!app::need_exit()) {
            video::Context *audio_ctx = decoder.decode_audio();
            if (!audio_ctx) {
                log::info("decode video over");
                break;
            }
            if (audio_ctx && audio_ctx->media_type() == video::MEDIA_TYPE_AUDIO) {
                Bytes *pcm = audio_ctx->get_pcm();
                log::info("data:%p length:%d sample_rate:%d format:%d channels:%d",
                pcm->data, pcm->data_len, audio_ctx->audio_sample_rate(), audio_ctx->audio_format(), audio_ctx->audio_channels());
                p->play(pcm);
                delete pcm;
            }
            delete audio_ctx;

            log::info("loop time:%.2d ms", (time::ticks_ms() - loop_ms));
            loop_ms = time::ticks_ms();
        }
        break;
    }
    case 2: {
        if (argc < 3) {
            helper();
            return -1;
        }

        std::string filepath = argv[2];
        double seek_s = 0.0;
        if (argc > 3) seek_s = atof(argv[3]);
        video::Decoder decoder = video::Decoder(filepath);
        video::Decoder decoder2 = video::Decoder(filepath);
        display::Display disp = display::Display();
        audio::Player *p = NULL;
        uint64_t loop_ms = time::ticks_ms(), last_us = time::ticks_us();
        log::info("filepath:%s seek_s:%f", filepath.c_str(), seek_s);
        log::info("has video:%d has audio:%d resolution:%dx%d bitrate:%d duration:%.2f s fps:%d seek_s:%f", decoder.has_video(), decoder.has_audio(), decoder.width(), decoder.height(), decoder.bitrate(), decoder.duration(), decoder.fps(), seek_s);
        if (decoder.has_video()) {
            err::check_bool_raise(decoder.seek(seek_s) >= 0, "decoder.seek failed");
            log::info("decoder.seek:%f", decoder.seek());
        }

        if (decoder.has_audio()) {
            log::info("audio_sample_rate:%d audio_format:%d audio_channels:%d", decoder.audio_sample_rate(), decoder.audio_format(),decoder.audio_channels());
            p = new audio::Player("", decoder.audio_sample_rate(), decoder.audio_format(), decoder.audio_channels());
        }

        std::list<video::Context *> *audio_list = new std::list<video::Context *>();
        std::list<video::Context *> *video_list = new std::list<video::Context *>();
        bool find_first_pts = false;
        uint64_t last_pts = 0;
        while (!app::need_exit()) {
            uint64_t t = time::ticks_ms();
            video::Context *ctx = NULL;
            while ((ctx = decoder.decode()) != NULL) {
                if (ctx->media_type() == video::MEDIA_TYPE_VIDEO) {
                    video_list->push_back(ctx);
                    break;
                } else if (ctx->media_type() == video::MEDIA_TYPE_AUDIO) {
                    audio_list->push_back(ctx);
                }
            }
            if (!ctx) {
                log::info("decode video over");
                break;
            }
            log::info("decode used %lld ms", time::ticks_ms() - t);

            t = time::ticks_ms();
            std::list<video::Context *>::iterator iter;
            for(iter=video_list->begin();iter!=video_list->end();iter++) {
                video::Context *video_ctx = *iter;

                if (!find_first_pts) {
                    last_pts = video_ctx->pts();
                    find_first_pts = true;
                }

                if (last_pts == video_ctx->pts()) {
                    last_pts += video_ctx->duration();
                    log::info("[VIDEO] play pts:%.2f ms next_pts:%d curr wait:%lld need wait:%lld",
                    video::timebase_to_ms(video_ctx->timebase(), video_ctx->pts()), last_pts,
                    (time::ticks_us() - last_us) / 1000, video_ctx->duration_us() / 1000);

                    video_list->erase(iter);

                    image::Image *img = video_ctx->image();
                    disp.show(*img);

                    while (time::ticks_us() - last_us < video_ctx->duration_us() - 5000) {
                        time::sleep_ms(1);
                    }
                    last_us = time::ticks_us();

                    delete img;
                    delete video_ctx;
                    break;
                }
            }
            log::info("playback video use %lld ms, video list size:%d", time::ticks_ms() - t, video_list->size());

            t = time::ticks_ms();
            while (audio_list->size() > 0) {
                video::Context *audio_ctx = *audio_list->begin();
                if (audio_ctx) {
                    audio_list->pop_front();
                    if (audio_ctx->media_type() == video::MEDIA_TYPE_AUDIO) {
                        Bytes *pcm = audio_ctx->get_pcm();
                        static uint64_t last_ms = 0;
                        log::info("[AUDIO] play pts:%.2fms last:%lld data:%p length:%d sample_rate:%d format:%d channels:%d duration:%lld(%d) ms",
                        video::timebase_to_ms(audio_ctx->timebase(), audio_ctx->pts()), time::ticks_ms() - last_ms,
                        pcm->data, pcm->data_len, audio_ctx->audio_sample_rate(), audio_ctx->audio_format(), audio_ctx->audio_channels(), audio_ctx->duration_us() / 1000, audio_ctx->duration());
                        last_ms = time::ticks_ms();
                        if (p) p->play(pcm);
                        delete pcm;
                    }
                    delete audio_ctx;
                }
            }
            log::info("playback audio use %lld ms, audio list size:%d", time::ticks_ms() - t, audio_list->size());

            log::info("loop time:%.2d ms, count:%d", (time::ticks_ms() - loop_ms));
            loop_ms = time::ticks_ms();
        }
        break;
    }
    default:
        helper();
        break;
    }
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

