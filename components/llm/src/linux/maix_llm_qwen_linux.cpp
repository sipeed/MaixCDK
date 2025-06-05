/**
 * LLM Qwen implementation on Linux
 * @license Apache-2.0
 * @author neucrack@sipeed
 * @date 2025-05-21
 */

#include "maix_llm_qwen.hpp"


namespace maix::nn
{
    Qwen::Qwen(const std::string &model)
    {
        _model_path = model;
        if(!model.empty())
        {
            err::Err e = load(model);
            if(e != err::ERR_NONE)
            {
                char buf[128];
                snprintf(buf, sizeof(buf), "load model failed: %s", err::to_str(e).c_str());
                throw err::Exception(e, buf);
            }
        }
    }

    Qwen::~Qwen()
    {
        unload();
    }

    void Qwen::set_log_level(log::LogLevel level, bool color)
    {

    }

    err::Err Qwen::load(const std::string &model)
    {
        _loaded = true;
        return err::ERR_NOT_IMPL;
    }

    err::Err Qwen::unload()
    {
        _loaded = false;
        return err::ERR_NOT_IMPL;
    }


    void Qwen::set_system_prompt(const std::string &prompt)
    {
        _system_prompt = prompt;
        clear_context();
    }

    nn::QwenResp Qwen::send(const std::string &msg)
    {
        QwenResp resp;
        resp.msg = msg;
        resp.err_code = err::ERR_NONE;
        resp.err_msg = "";
        std::string resp_msg = "hello, not implement for this platform yet\n";
        for(auto c : resp_msg)
        {
            time::sleep_ms(20);
            resp.msg += c;
            resp.msg_new = c;
            if (_callback)
                _callback(*this, resp);
        }
        return resp;
    }

    err::Err Qwen::clear_context()
    {
        return err::ERR_NONE;
    }
} // namespace maix::nn
