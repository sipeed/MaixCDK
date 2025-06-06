/**
 * LLM InternVL implementation on MaixCam2
 * @license Apache-2.0
 * @author neucrack@sipeed
 * @date 2025-06-03
 */

 #include "maix_vlm_internvl.hpp"
 #include "maix_nn.hpp"

 namespace maix::nn
 {
     class InternVLObj
     {
     public:
         MUD mud;
         InternVLResp resp;
         int image_w;
         int image_h;
         maix::image::Format image_fmt;
         std::vector<unsigned short> img_embed;
     };

     InternVL::InternVL(const std::string &model)
     {
         _data = new InternVLObj();
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

     void InternVL::set_log_level(log::LogLevel level, bool color)
     {
     }

     err::Err InternVL::load(const std::string &model)
     {
         InternVLObj *obj = (InternVLObj *)_data;
         _model_path = model;
         err::Err e = obj->mud.load(model);
         if(e != err::ERR_NONE)
             return e;
         std::string model_dir = fs::dirname(model);

         obj->image_fmt = maix::image::Format::FMT_RGB888;

        // print params
        // log::info("model info:");
        // log::print(log::LogLevel::LEVEL_INFO, "\tmodel type: %s\n", _version.c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\tmodel path: %s\n", obj->mud.items["basic"]["model_npu"].c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\tpost model path: %s\n", obj->mud.items["extra"]["post_model"].c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\ttokens embed path: %s\n", obj->mud.items["extra"]["tokens_embed"].c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\tuse_mmap_load_embed: %s\n", obj->mud.items["extra"]["use_mmap_load_embed"].c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\tmodel num: %d\n", attr.axmodel_num);
        // log::print(log::LogLevel::LEVEL_INFO, "\ttokens embed num: %d\n", attr.tokens_embed_num);
        // log::print(log::LogLevel::LEVEL_INFO, "\ttokens embed size: %d\n", attr.tokens_embed_size);
        // log::print(log::LogLevel::LEVEL_INFO, "\ttokenizer url: %s\n", attr.url_tokenizer_model.c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\tinput image size: %d x %d\n", obj->image_w, obj->image_h);
        // log::print(log::LogLevel::LEVEL_INFO, "\tinput image format: %s\n", maix::image::format_name(obj->image_fmt).c_str());
        // log::print(log::LogLevel::LEVEL_INFO, "\n");

         _loaded = true;
         return err::ERR_NONE;
     }

     err::Err InternVL::unload()
     {
        //  InternVLObj *obj = (InternVLObj *)_data;
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
        if (need_free)
            delete p_img;
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
         return obj->resp;
     }

     void InternVL::cancel()
     {
 
     }

 } // namespace maix::nn
