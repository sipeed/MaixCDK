
#include "maix_basic.hpp"
#include "main.h"
#include "maix_llm_qwen.hpp"
#include <iostream>

using namespace maix;

void on_reply(nn::Qwen &qwen, const nn::QwenResp &resp)
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
    nn::Qwen qwen(model_path);
    const char *system_prompt = "You are Qwen, created by Alibaba Cloud. You are a helpful assistant.";
    if (argc > 2)
    {
        system_prompt = argv[2];
    }
    log::info("System prompt: %s", system_prompt);
    qwen.set_system_prompt(system_prompt);
    qwen.set_reply_callback(on_reply);
    log::info("'q' to quit, 'clear' to clear context");
    while(!app::need_exit())
    {
        std::string input;
        printf(">> ");
        fflush(stdout);
        std::getline(std::cin, input);
        if (input.empty())
            continue;
        if (input == "exit" || input == "quit" || input == "q")
            break;
        if (input == "clear")
        {
            qwen.clear_context();
            continue;
        }
        nn::QwenResp resp = qwen.send(input);
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


