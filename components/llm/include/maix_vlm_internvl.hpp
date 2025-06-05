/**
 * VLM InternVL
 * @license: Apache-2.0
 * @author: neucrack@sipeed
 * @date: 2025-05-30
 */
#pragma once

#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_llm_qwen.hpp"

namespace maix::nn
{
        /**
     * InternVL model response
     * @maixpy maix.nn.InternVLResp
     */
     class InternVLResp
     {
     public:
     InternVLResp()
         {
             err_code = err::ERR_NONE;
             err_msg = "";
         }

         /**
          * Model response full message.
          * @maixpy maix.nn.InternVLResp.msg
          */
         std::string msg;

         /**
          * Model response new message.
          * @maixpy maix.nn.InternVLResp.msg_new
          */
         std::string msg_new;

         /**
          * Model response error code, maix.Err type, should be err.Err.ERR_NONE if no error.
          * @maixpy maix.nn.InternVLResp.err_code
          */
         err::Err err_code;

         /**
          * Model response error message.
          * @maixpy maix.nn.InternVLResp.err_msg
          */
         std::string err_msg;
     };

     /**
      * InternVL model post config
      * @maixpy maix.nn.InternVLPostConfig
      */
     class InternVLPostConfig
     {
     public:
        InternVLPostConfig()
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
          * @maixpy maix.nn.InternVLPostConfig.enable_temperature
          */
         bool enable_temperature;

         /**
          * Temperature sampling value
          * @maixpy maix.nn.InternVLPostConfig.temperature
          */
         float temperature;

         /**
          * Enable repetition penalty
          * @maixpy maix.nn.InternVLPostConfig.enable_repetition_penalty
          */
         bool enable_repetition_penalty;

         /**
          * Repetition penalty value
          * @maixpy maix.nn.InternVLPostConfig.repetition_penalty
          */
         float repetition_penalty;

         /**
          * Repetition penalty window
          * @maixpy maix.nn.InternVLPostConfig.penalty_window
          */
         int penalty_window;

         /**
          * Enable diversity penalty
          * @maixpy maix.nn.InternVLPostConfig.enable_top_p_sampling
          */
         bool enable_top_p_sampling;

         /**
          * Diversity penalty value
          * @maixpy maix.nn.InternVLPostConfig.top_p
          */
         float top_p;

         /**
          * Enable top k sampling
          * @maixpy maix.nn.InternVLPostConfig.enable_top_k_sampling
          */
         bool enable_top_k_sampling;

         /**
          * Top k sampling value
          * @maixpy maix.nn.InternVLPostConfig.top_k
          */
         int top_k;
     };

    /**
     * InternVL model
     * @maixpy maix.nn.InternVL
     */
    class InternVL
    {
    public:
        /**
         * InternVL constructor
         * @param[in] model model file path, model format can be MUD(model universal describe file) file.
         *                  If model_path set, will load model from file, load failed will raise err.Exception.
         *                  If model_path not set, you can load model later by load function.
         * @maixpy maix.nn.InternVL.__init__
         * @maixcdk maix.nn.InternVL.InternVL
         */
         InternVL(const std::string &model);

        ~InternVL();

        /**
         * Load model from file
         * @param[in] model model file path, model format can be MUD(model universal describe file) file.
         * @return error code, if load success, return err::ERR_NONE
         * @maixpy maix.nn.InternVL.load
         */
        err::Err load(const std::string &model);

        /**
         * Unload model
         * @return error code, if unload success, return err::ERR_NONE
         * @maixpy maix.nn.InternVL.unload
         */
        err::Err unload();

        /**
         * Is model loaded
         * @return true if model loaded, else false
         * @maixpy maix.nn.InternVL.loaded
         */
        bool loaded()
        {
            return _loaded;
        }

        /**
         * Set system prompt
         * @param prompt system prompt
         * @maixpy maix.nn.InternVL.set_system_prompt
         */
        void set_system_prompt(const std::string &prompt);

        /**
         * Get system prompt
         * @return system prompt
         * @maixpy maix.nn.InternVL.get_system_prompt
         */
        std::string get_system_prompt()
        {
            return _system_prompt;
        }

        /**
         * Set log level
         * @param level log level, @see maix.log.LogLevel
         * @param color true to enable color, false to disable color
         * @maixpy maix.nn.InternVL.set_log_level
         */
        void set_log_level(log::LogLevel level, bool color);

        /**
         * Set reply callback.
         * @param callback reply callback, when token(words) generated, this function will be called,
         * so you can get response message in real time in this callback funtion.
         * If set to None(nullptr in C++), you can get response after all response message generated.
         * @maixpy maix.nn.InternVL.set_reply_callback
         */
        void set_reply_callback(std::function<void(nn::InternVL &, const nn::InternVLResp &)> callback = nullptr)
        {
            _callback = callback;
        }

        /**
         * Get reply callback
         * @return reply callback
         * @maixpy maix.nn.InternVL.get_reply_callback
         */
        std::function<void(nn::InternVL &, const nn::InternVLResp &)> get_reply_callback()
        {
            return _callback;
        }

        /**
         * Image input width
         * @return input width.
         * @maixpy maix.nn.InternVL.input_width
         */
        int input_width();

        /**
         * Image input height
         * @return input height.
         * @maixpy maix.nn.InternVL.input_height
         */
        int input_height();

        /**
         * Image input format
         * @return input format.
         * @maixpy maix.nn.InternVL.input_format
         */
        maix::image::Format input_format();

        /**
         * Set image and will encode image.
         * You can set image once and call send multiple times.
         * @param img the image you want to use.
         * @param fit Image resize fit method, only used when img size not equal to model input.
         * @return err.Err return err.Err.ERR_NONE is no error happen.
         * @maixpy maix.nn.InternVL.set_image
         */
        err::Err set_image(maix::image::Image &img, maix::image::Fit fit = maix::image::Fit::FIT_CONTAIN);

        /**
         * Clear image, InternVL2.5 based on Qwen2.5, so you can clear image and only use LLM function.
         * @maixpy maix.nn.InternVL.clear_image
         */
         void clear_image();

         /**
          * Whether image set by set_image
          * @return Return true if image set by set_image function, or return false.
          * @maixpy maix.nn.InternVL.is_image_set
          */
         bool is_image_set();

        /**
         * Send message to model
         * @param msg message to send
         * @return model response
         * @maixpy maix.nn.InternVL.send
         */
        nn::InternVLResp send(const std::string &msg);

        /**
         * Get model version
         * @return model version
         * @maixpy maix.nn.InternVL.version
         */
        std::string version()
        {
            return _version;
        }

    public:
        /**
         * InternVL post config, default will read config from model mud file, you can also set it manually here.
         * @maixpy maix.nn.InternVL.post_config
         */
        nn::InternVLPostConfig post_config;

    private:
        bool _loaded = false;
        std::string _system_prompt;
        std::string _model_path;
        std::string _version;
        std::string _tokenizer_type;
        std::function<void(InternVL &, const InternVLResp &)> _callback = nullptr;
        void *_data; // for implementation
    };

}


