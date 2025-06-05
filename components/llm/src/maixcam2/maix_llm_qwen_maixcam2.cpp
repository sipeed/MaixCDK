/**
 * LLM Qwen implementation on MaixCam2
 * @license Apache-2.0
 * @author neucrack@sipeed
 * @date 2025-05-21
 */

#include "maix_llm_qwen.hpp"
#include "maix_nn.hpp"
#include "LLM_Qwen.hpp"
#include "ax_middleware.hpp"
#include "tokenizer_service_util.hpp"

namespace maix::nn
{
    class QwenObj
    {
    public:
        MUD mud;
        LLM_Qwen::LLM lLaMa;
        maix::middleware::maixcam2::SYS *ax_sys;
        maix::middleware::maixcam2::ENGINE *ax_engine;
        std::string last_reply;
        std::vector<std::vector<unsigned short>> k_caches, v_caches;
        int precompute_len = 0;
        QwenResp resp;
        Qwen *qwen;
    };

    Qwen::Qwen(const std::string &model)
    {
        _data = new QwenObj();
        ((QwenObj*)_data)->qwen = this;
        _model_path = model;
        _system_prompt = "You are Qwen, created by Alibaba Cloud. You are a helpful assistant.";
        set_log_level(log::get_log_level(), log::get_log_use_color());
        if(!model.empty())
        {
            err::Err e = load(model);
            if(e != err::ERR_NONE)
            {
                char buf[128];
                snprintf(buf, sizeof(buf), "load model %s failed", model.c_str());
                throw err::Exception(e, buf);
            }
        }
    }

    Qwen::~Qwen()
    {
        unload();
        if (_data)
        {
            delete (QwenObj *)_data;
            _data = nullptr;
        }
    }

    static void _on_msg(int *p_token, int n_token, const char *p_str, float token_per_sec, void *reserve)
    {
        QwenObj *obj = (QwenObj *)reserve;
        obj->resp.msg_new = p_str;
        obj->resp.msg += p_str;
        obj->resp.err_code = err::ERR_NONE;
        obj->resp.err_msg = "";
        auto callback = obj->qwen->get_reply_callback();
        if (callback)
        {
            callback(*obj->qwen, obj->resp);
        }
        // fprintf(stdout, "%s", p_str);
        // fflush(stdout);
    }

    void Qwen::set_log_level(log::LogLevel level, bool color)
    {
        ax_log_use_color = color;
        switch(level)
        {
        case log::LogLevel::LEVEL_DEBUG:
            ax_log_level = SAMPLE_LOG_DEBUG;
            break;
        case log::LogLevel::LEVEL_WARN:
            ax_log_level = SAMPLE_LOG_WARN;
            break;
        case log::LogLevel::LEVEL_ERROR:
            ax_log_level = SAMPLE_LOG_ERROR;
            break;
        default:
            ax_log_level = SAMPLE_LOG_INFO;
            break;
        }
    }

    err::Err Qwen::load(const std::string &model)
    {
        QwenObj *obj = (QwenObj *)_data;
        _model_path = model;
        err::Err e = obj->mud.load(model);
        if(e != err::ERR_NONE)
            return e;
        std::string model_dir = fs::dirname(model);
        // init llm model
        LLM_Qwen::LLMAttrType attr;
        attr.system_prompt = _system_prompt;
        attr.tokenizer_type = TKT_HTTP;
        bool ai_isp_on = app::get_sys_config_kv("npu", "ai_isp", "0") == "1" ? true : false;
        if(ai_isp_on)
        {
            log::warn("npu_ai_isp_on from config is on, but LLM model only support npu model, please not use camera or turn off ai_isp");
        }
        try
        {
            _version = obj->mud.items["extra"]["model_type"];
            _tokenizer_type = _version;
            attr.url_tokenizer_model = obj->mud.items["extra"]["tokenizer_url"];
            attr.filename_tokens_embed = fs::join({model_dir, obj->mud.items["extra"]["tokens_embed"]});
            attr.filename_post_axmodel = fs::join({model_dir, obj->mud.items["extra"]["post_model"]});
            attr.template_filename_axmodel = fs::join({model_dir, obj->mud.items["basic"]["model_npu"]});
            attr.axmodel_num = std::stoi(obj->mud.items["extra"]["model_num"]);
            attr.tokens_embed_num = std::stoi(obj->mud.items["extra"]["tokens_embed_num"]);
            attr.tokens_embed_size = std::stoi(obj->mud.items["extra"]["tokens_embed_size"]);
            attr.b_use_mmap_load_embed = (obj->mud.items["extra"]["use_mmap_load_embed"] == "true" || obj->mud.items["extra"]["use_mmap_load_embed"] == "1") ? true : false;
        }
        catch(...)
        {
            log::error("load model failed, key-value error in mud's extra section");
            return err::ERR_ARGS;
        }
        try
        {
            post_config.enable_temperature = obj->mud.items["post_config"]["enable_temperature"] == "true" ? true : false;
            post_config.temperature = std::stof(obj->mud.items["post_config"]["temperature"]);
            post_config.enable_repetition_penalty = obj->mud.items["post_config"]["enable_repetition_penalty"] == "true" ? true : false;
            post_config.repetition_penalty = std::stof(obj->mud.items["post_config"]["repetition_penalty"]);
            post_config.penalty_window = std::stoi(obj->mud.items["post_config"]["penalty_window"]);
            post_config.enable_top_p_sampling = obj->mud.items["post_config"]["enable_top_p_sampling"] == "true" ? true : false;
            post_config.top_p = std::stof(obj->mud.items["post_config"]["top_p"]);
            post_config.enable_top_k_sampling = obj->mud.items["post_config"]["enable_top_k_sampling"] == "true" ? true : false;
            post_config.top_k = std::stoi(obj->mud.items["post_config"]["top_k"]);
        }
        catch(...)
        {
            log::error("load model failed, key-value error in mud's post_config section");
            return err::ERR_ARGS;
        }
        if(obj->mud.items["extra"].find("tokenizer_type") != obj->mud.items["extra"].end())
        {
            _tokenizer_type = obj->mud.items["extra"]["tokenizer_type"];
        }

        attr.runing_callback = nullptr;
        attr.reserve = obj;

        // print params
        log::info("model info:");
        log::print(log::LogLevel::LEVEL_INFO, "\tmodel type: %s\n", _version.c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\tmodel path: %s\n", obj->mud.items["basic"]["model_npu"].c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\tpost model path: %s\n", obj->mud.items["extra"]["post_model"].c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\ttokens embed path: %s\n", obj->mud.items["extra"]["tokens_embed"].c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\tuse_mmap_load_embed: %s\n", obj->mud.items["extra"]["use_mmap_load_embed"].c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\tmodel num: %d\n", attr.axmodel_num);
        log::print(log::LogLevel::LEVEL_INFO, "\ttokens embed num: %d\n", attr.tokens_embed_num);
        log::print(log::LogLevel::LEVEL_INFO, "\ttokens embed size: %d\n", attr.tokens_embed_size);
        log::print(log::LogLevel::LEVEL_INFO, "\ttokenizer url: %s\n", attr.url_tokenizer_model.c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\n");

        // init npu engine
        log::info("init middleware SYSTEM");
        obj->ax_sys =  new middleware::maixcam2::SYS();
        e = obj->ax_sys->init();
        if(e != err::ERR_NONE)
        {
            log::error("init middleware SYSTEM failed: %s", err::to_str(e).c_str());
            delete obj->ax_sys;
            obj->ax_sys = nullptr;
            return e;
        }
        log::info("init middleware NPU ENGINE");
        obj->ax_engine =  new middleware::maixcam2::ENGINE(AX_ENGINE_VIRTUAL_NPU_DISABLE);
        e = obj->ax_engine->init();
        if(e != err::ERR_NONE)
        {
            log::error("init middleware ENGINE failed: %s, maybe there's other program using NPU", err::to_str(e).c_str());
            delete obj->ax_engine;
            obj->ax_engine = nullptr;
            delete obj->ax_sys;
            obj->ax_sys = nullptr;
            return e;
        }

        // check tokenizer service
        // find http://127.0.0.1 in obj->mud.items["extra"]["tokenizer_url"]
        e = check_start_tokenizer_service(obj->mud.items["extra"]["tokenizer_url"]);
        if(e != err::ERR_NONE)
        {
            delete obj->ax_engine;
            obj->ax_engine = nullptr;
            delete obj->ax_sys;
            obj->ax_sys = nullptr;
            return e;
        }

        // init llm model
        if(!obj->lLaMa.Init(attr, post_config, _tokenizer_type))
        {
            log::error("Qwen Init failed");
            delete obj->ax_engine;
            obj->ax_engine = nullptr;
            delete obj->ax_sys;
            obj->ax_sys = nullptr;
            return err::ERR_RUNTIME;
        }

        // kvcache
        std::vector<int> _token_ids;
        obj->lLaMa.SetSystemPrompt(attr.system_prompt, _token_ids);
        obj->lLaMa.GenerateKVCachePrefill(_token_ids, obj->k_caches, obj->v_caches, obj->precompute_len);
        log::info("System prompt tokens size: %d", obj->precompute_len);

        _loaded = true;
        return err::ERR_NONE;
    }

    err::Err Qwen::unload()
    {
        QwenObj *obj = (QwenObj *)_data;
        obj->lLaMa.Stop();
        obj->lLaMa.Deinit();
        if(obj->ax_engine)
        {
            delete obj->ax_engine;
            obj->ax_engine = nullptr;
        }
        if(obj->ax_sys)
        {
            delete obj->ax_sys;
            obj->ax_sys = nullptr;
        }
        _loaded = false;
        return err::ERR_NONE;
    }


    void Qwen::set_system_prompt(const std::string &prompt)
    {
        _system_prompt = prompt;
        clear_context();
    }

    nn::QwenResp Qwen::send(const std::string &msg)
    {
        QwenObj *obj = (QwenObj *)_data;
        obj->resp.msg = "";
        obj->resp.err_code = err::ERR_NONE;
        obj->resp.err_msg = "";
        if(msg.empty())
        {
            obj->resp.err_code = err::ERR_ARGS;
            obj->resp.err_msg = "msg is empty";
            return obj->resp;
        }

        // check callback
        auto attr = obj->lLaMa.getAttr();
        if(_callback)
        {
            attr->runing_callback = _on_msg;
        }
        else
        {
            attr->runing_callback = nullptr;
        }

        // run LLM model
        std::vector<unsigned short> prompt_data;
        std::vector<int> tokens_ids, tokens_diff;
        obj->lLaMa.Encode(prompt_data, msg, obj->last_reply, tokens_ids, tokens_diff);
        if (auto ret = obj->lLaMa.SetKVCache(obj->k_caches, obj->v_caches, obj->precompute_len, tokens_diff.size()); ret != 0)
        {
            obj->resp.err_code = err::Err::ERR_BUFF_FULL;
            obj->resp.err_msg = "";
            char buf[128];
            snprintf(buf, sizeof(buf), "SetKVCache failed: %d,the context may be full, try clear context", ret);
            obj->resp.msg = buf;
            return obj->resp;
        }
        obj->last_reply = obj->lLaMa.Run(prompt_data);
        obj->lLaMa.GetKVCache(obj->k_caches, obj->v_caches, obj->precompute_len);

        // obj->resp.msg = obj->last_reply;
        return obj->resp;
    }

    err::Err Qwen::clear_context()
    {
        if(_loaded)
        {
            QwenObj *obj = (QwenObj *)_data;
            std::vector<int> _token_ids;
            obj->lLaMa.SetSystemPrompt(_system_prompt, _token_ids);
            obj->lLaMa.GenerateKVCachePrefill(_token_ids, obj->k_caches, obj->v_caches, obj->precompute_len);
            // log::info("precompute_len: %d", obj->precompute_len);
            return err::ERR_NONE;
        }
        return err::ERR_NOT_OPEN;
    }
} // namespace maix::nn
