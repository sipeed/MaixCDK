
#include "maix_basic.hpp"
#include "main.h"
#include "maix_vlm_internvl.hpp"
#include <iostream>

using namespace maix;

void on_reply(nn::InternVL &obj, const nn::InternVLResp &resp)
{
    printf("%s", resp.msg_new.c_str());
    fflush(stdout);
}


int _main(int argc, char* argv[])
{
    log::info("Program start");

    if (argc < 2)
    {
        log::error("Usage: %s <model_path(.mud)> [system prompt]", argv[0]);
        return -1;
    }
    std::string model_path = argv[1];
    log::set_log_level(log::LogLevel::LEVEL_INFO, true);
    nn::InternVL internvl(model_path);
    const char *system_prompt = "You are InternVL, a multimodal model and a helpful vision-to-text assistant.";
    if (argc > 2)
    {
        system_prompt = argv[2];
    }
    log::info("System prompt: %s", system_prompt);
    internvl.set_system_prompt(system_prompt);
    internvl.set_reply_callback(on_reply);
    log::info("Input image path and prompt, you can set image once and ask multiple times.");
    log::info("'q' to quit");
    std::string image_path;
    while(!app::need_exit())
    {
        // 1. input image path
        std::string input;
        printf("image path: >> ");
        fflush(stdout);
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit" || input == "q")
            break;
        if(!input.empty())
        {
            image::Image *img = image::load(input);
            if(!img)
            {
                log::error("load image %s failed", input.c_str());
                continue;
            }
            err::Err e = internvl.set_image(*img, maix::image::Fit::FIT_CONTAIN);
            delete img;
            if(e != err::Err::ERR_NONE)
            {
                log::error("set image failed, error: %s", err::to_str(e).c_str());
                continue;
            }
            image_path = input;
        }
        else if(!internvl.is_image_set())
        {
            log::info("image not set, only text input mode");
        }
        else
        {
            log::info("image remain use last set: %s", image_path.c_str());
        }

        // 2. input prompt
        std::string prompt;
        printf("prompt: >> ");
        fflush(stdout);
        std::getline(std::cin, prompt);
        if (prompt.empty())
            continue;
        if (prompt == "exit" || prompt == "quit" || prompt == "q")
            break;

        // send message to InterVL
        nn::InternVLResp resp = internvl.send(prompt);
        if (resp.err_code != err::Err::ERR_NONE)
        {
            if(resp.err_code == err::Err::ERR_BUFF_FULL)
            {
                log::error("context buffer full, please clear context");
                continue;
            }
            log::error("Error: %s, %s", err::to_str(resp.err_code).c_str(), resp.err_msg.c_str());
            break;
        }
        // log::info("%s", resp.msg.c_str());
    }
    log::info("Program exit");
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


