/**
 * LLM Qwen
 * @license: Apache-2.0
 * @author: neucrack@sipeed
 * @date: 2025-05-20
 */
#pragma once

#include "maix_basic.hpp"

namespace maix::nn
{

    /**
     * Qwen model response
     * @maixpy maix.nn.QwenResp
     */
    class QwenResp
    {
    public:
        QwenResp()
        {
            err_code = err::ERR_NONE;
            err_msg = "";
        }

        /**
         * Model response full message.
         * @maixpy maix.nn.QwenResp.msg
         */
        std::string msg;

        /**
         * Model response new message.
         * @maixpy maix.nn.QwenResp.msg_new
         */
        std::string msg_new;

        /**
         * Model response error code, maix.Err type, should be err.Err.ERR_NONE if no error.
         * @maixpy maix.nn.QwenResp.err_code
         */
        err::Err err_code;

        /**
         * Model response error message.
         * @maixpy maix.nn.QwenResp.err_msg
         */
        std::string err_msg;
    };

    /**
     * Qwen model post config
     * @maixpy maix.nn.QwenPostConfig
     */
    class QwenPostConfig
    {
    public:
        QwenPostConfig()
        {
            enable_temperature = true;
            temperature = 0.9;

            enable_repetition_penalty = false;
            repetition_penalty = 1.2;
            penalty_window = 20;

            enable_top_p_sampling = false;
            top_p = 0.8;

            enable_top_k_sampling = true;
            top_k = 10;
        }

        /**
         * Enable temperature sampling
         * @maixpy maix.nn.QwenPostConfig.enable_temperature
         */
        bool enable_temperature;

        /**
         * Temperature sampling value
         * @maixpy maix.nn.QwenPostConfig.temperature
         */
        float temperature;

        /**
         * Enable repetition penalty
         * @maixpy maix.nn.QwenPostConfig.enable_repetition_penalty
         */
        bool enable_repetition_penalty;

        /**
         * Repetition penalty value
         * @maixpy maix.nn.QwenPostConfig.repetition_penalty
         */
        float repetition_penalty;

        /**
         * Repetition penalty window
         * @maixpy maix.nn.QwenPostConfig.penalty_window
         */
        int penalty_window;

        /**
         * Enable diversity penalty
         * @maixpy maix.nn.QwenPostConfig.enable_top_p_sampling
         */
        bool enable_top_p_sampling;

        /**
         * Diversity penalty value
         * @maixpy maix.nn.QwenPostConfig.top_p
         */
        float top_p;

        /**
         * Enable top k sampling
         * @maixpy maix.nn.QwenPostConfig.enable_top_k_sampling
         */
        bool enable_top_k_sampling;

        /**
         * Top k sampling value
         * @maixpy maix.nn.QwenPostConfig.top_k
         */
        int top_k;
    };

    /**
     * Qwen model
     * @maixpy maix.nn.Qwen
     */
    class Qwen
    {
    public:
        /**
         * Qwen constructor
         * @param[in] model model file path, model format can be MUD(model universal describe file) file.
         *                  If model_path set, will load model from file, load failed will raise err.Exception.
         *                  If model_path not set, you can load model later by load function.
         * @maixpy maix.nn.Qwen.__init__
         * @maixcdk maix.nn.Qwen.Qwen
         */
         Qwen(const std::string &model);

        ~Qwen();

        /**
         * Load model from file
         * @param[in] model model file path, model format can be MUD(model universal describe file) file.
         * @return error code, if load success, return err::ERR_NONE
         * @maixpy maix.nn.Qwen.load
         */
        err::Err load(const std::string &model);

        /**
         * Unload model
         * @return error code, if unload success, return err::ERR_NONE
         * @maixpy maix.nn.Qwen.unload
         */
        err::Err unload();

        /**
         * Is model loaded
         * @return true if model loaded, else false
         * @maixpy maix.nn.Qwen.loaded
         */
        bool loaded()
        {
            return _loaded;
        }

        /**
         * Set system prompt, will auto call clear_context.
         * @param prompt system prompt
         * @maixpy maix.nn.Qwen.set_system_prompt
         */
        void set_system_prompt(const std::string &prompt);

        /**
         * Get system prompt
         * @return system prompt
         * @maixpy maix.nn.Qwen.get_system_prompt
         */
        std::string get_system_prompt()
        {
            return _system_prompt;
        }

        /**
         * Set log level
         * @param level log level, @see maix.log.LogLevel
         * @param color true to enable color, false to disable color
         * @maixpy maix.nn.Qwen.set_log_level
         */
        void set_log_level(log::LogLevel level, bool color);

        /**
         * Set reply callback
         * @param callback reply callback, when token(words) generated, this function will be called,
         * so you can get response message in real time in this callback funtion.
         * If set to None(nullptr in C++), you can get response after all response message generated.
         * @maixpy maix.nn.Qwen.set_reply_callback
         */
        void set_reply_callback(std::function<void(nn::Qwen &, const nn::QwenResp &)> callback = nullptr)
        {
            _callback = callback;
        }

        /**
         * Get reply callback
         * @return reply callback
         * @maixpy maix.nn.Qwen.get_reply_callback
         */
        std::function<void(nn::Qwen &, const nn::QwenResp &)> get_reply_callback()
        {
            return _callback;
        }

        /**
         * Send message to model
         * @param msg message to send
         * @return model response
         * @maixpy maix.nn.Qwen.send
         */
        nn::QwenResp send(const std::string &msg);

        /**
         * Clear context
         * @return error code, if clear success, return err::ERR_NONE
         * @maixpy maix.nn.Qwen.clear_context
         */
        err::Err clear_context();


        /**
         * Get model version
         * @return model version
         * @maixpy maix.nn.Qwen.version
         */
        std::string version()
        {
            return _version;
        }

    public:
        /**
         * Qwen post config, default will read config from model mud file, you can also set it manually here.
         * @maixpy maix.nn.Qwen.post_config
         */
        nn::QwenPostConfig post_config;

    private:
        bool _loaded = false;
        std::string _system_prompt;
        std::string _model_path;
        std::string _version;
        std::string _tokenizer_type;
        std::function<void(Qwen &, const QwenResp &)> _callback = nullptr;
        void *_data; // for implementation
    };

}


