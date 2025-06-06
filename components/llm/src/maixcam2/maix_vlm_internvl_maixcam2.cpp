/**
 * LLM InternVL implementation on MaixCam2
 * @license Apache-2.0
 * @author neucrack@sipeed
 * @date 2025-06-03
 */

 #include "maix_vlm_internvl.hpp"
 #include "maix_nn.hpp"
 #include "LLM_InternVL.hpp"
 #include "ax_middleware.hpp"
 #include "tokenizer_service_util.hpp"

 namespace maix::nn
 {
     class InternVLObj
     {
     public:
         MUD mud;
         VLM_InternVL::LLM lLaMa;
         maix::middleware::maixcam2::SYS *ax_sys;
         maix::middleware::maixcam2::ENGINE *ax_engine;
         std::vector<std::vector<unsigned short>> k_caches, v_caches;
         int precompute_len = 0;
         InternVLResp resp;
         InternVL *obj;
         int image_w;
         int image_h;
         maix::image::Format image_fmt;
         std::vector<unsigned short> img_embed;
     };

     InternVL::InternVL(const std::string &model)
     {
         _data = new InternVLObj();
         ((InternVLObj*)_data)->obj = this;
         _model_path = model;
         _system_prompt = "You are InternVL. You are a helpful vision-to-text assistant.";
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

     InternVL::~InternVL()
     {
         unload();
         if (_data)
         {
             delete (InternVLObj *)_data;
             _data = nullptr;
         }
     }

     static void _on_msg(int *p_token, int n_token, const char *p_str, float token_per_sec, void *reserve)
     {
         InternVLObj *obj = (InternVLObj *)reserve;
         obj->resp.msg_new = p_str;
         obj->resp.msg += p_str;
         obj->resp.err_code = err::ERR_NONE;
         obj->resp.err_msg = "";
         auto callback = obj->obj->get_reply_callback();
         if (callback)
         {
             callback(*obj->obj, obj->resp);
         }
         // fprintf(stdout, "%s", p_str);
         // fflush(stdout);
     }

     void InternVL::set_log_level(log::LogLevel level, bool color)
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

     err::Err InternVL::load(const std::string &model)
     {
         InternVLObj *obj = (InternVLObj *)_data;
         _model_path = model;
         err::Err e = obj->mud.load(model);
         if(e != err::ERR_NONE)
             return e;
         std::string model_dir = fs::dirname(model);
         // init llm model
         VLM_InternVL::LLMAttrType attr;
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
             attr.filename_vpm_resampler_axmodedl = fs::join({model_dir, obj->mud.items["extra"]["vpm_resampler_model"]});
             attr.vpm_len = std::stoi(obj->mud.items["extra"]["vpm_len"]);
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
         QwenPostConfig config;
         config.enable_temperature = post_config.enable_temperature;
         config.temperature = post_config.temperature;
         config.enable_repetition_penalty = post_config.enable_repetition_penalty;
         config.repetition_penalty = post_config.repetition_penalty;
         config.penalty_window = post_config.penalty_window;
         config.enable_top_p_sampling = post_config.enable_top_p_sampling;
         config.top_p = post_config.top_p;
         config.enable_top_k_sampling = post_config.enable_top_k_sampling;
         config.top_k = post_config.top_k;
         if(!obj->lLaMa.Init(attr, config, _tokenizer_type, obj->image_w, obj->image_h))
         {
             log::error("InternVL Init failed");
             delete obj->ax_engine;
             obj->ax_engine = nullptr;
             delete obj->ax_sys;
             obj->ax_sys = nullptr;
             return err::ERR_RUNTIME;
         }

         obj->image_fmt = maix::image::Format::FMT_RGB888;

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
        log::print(log::LogLevel::LEVEL_INFO, "\tinput image size: %d x %d\n", obj->image_w, obj->image_h);
        log::print(log::LogLevel::LEVEL_INFO, "\tinput image format: %s\n", maix::image::format_name(obj->image_fmt).c_str());
        log::print(log::LogLevel::LEVEL_INFO, "\n");

         _loaded = true;
         return err::ERR_NONE;
     }

     err::Err InternVL::unload()
     {
         InternVLObj *obj = (InternVLObj *)_data;
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


     void InternVL::set_system_prompt(const std::string &prompt)
     {
         _system_prompt = prompt;
     }

     int InternVL::input_width()
     {
        InternVLObj *obj = (InternVLObj *)_data;
        return obj->image_w;
     }

     int InternVL::input_height()
     {
        InternVLObj *obj = (InternVLObj *)_data;
        return obj->image_h;
     }

     maix::image::Format InternVL::input_format()
     {
        InternVLObj *obj = (InternVLObj *)_data;
        return obj->image_fmt;
     }

     err::Err InternVL::set_image(maix::image::Image &img, maix::image::Fit fit)
     {
        InternVLObj *obj = (InternVLObj *)_data;
        maix::image::Image *p_img = &img;
        bool need_free = false;
        if(img.width() != obj->image_w || img.height() != obj->image_h)
        {
            p_img = img.resize(obj->image_w, obj->image_h, fit);
            need_free = true;
        }
        int ret = obj->lLaMa.Encode(*p_img, obj->img_embed);
        if (need_free)
            delete p_img;
        if(ret != 0)
        {
            log::error("Encode image failed, ret: %d", ret);
            return err::ERR_RUNTIME;
        }
        return err::ERR_NONE;
     }

     void InternVL::clear_image()
     {
        InternVLObj *obj = (InternVLObj *)_data;
        obj->img_embed.clear();
     }

     bool InternVL::is_image_set()
     {
        InternVLObj *obj = (InternVLObj *)_data;
        return !obj->img_embed.empty();
     }

     nn::InternVLResp InternVL::send(const std::string &msg)
     {
         InternVLObj *obj = (InternVLObj *)_data;
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
         int ret = obj->lLaMa.Encode(prompt_data, obj->img_embed, msg);
         if(ret != 0)
         {
             log::error("Encode msg failed, ret: %d", ret);
             obj->resp.err_code = err::ERR_RUNTIME;
             obj->resp.err_msg = "Encode msg failed";
             return obj->resp;
         }
         /*obj->resp.msg = */obj->lLaMa.Run(prompt_data);

         return obj->resp;
     }

     void InternVL::cancel()
     {
        InternVLObj *obj = (InternVLObj *)_data;
        obj->lLaMa.Stop();
     }

 } // namespace maix::nn
