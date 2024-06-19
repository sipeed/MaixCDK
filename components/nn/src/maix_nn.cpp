/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_nn.hpp"
#include "maix_basic.hpp"
#include "inifile.h"

#if PLATFORM_MAIXCAM
    #include "maix_nn_maixcam.hpp"
#endif


namespace maix::nn
{
    MUD::MUD(const char *model_path)
    {
        if (model_path)
        {
            err::Err e = load(model_path);
            if (e != err::ERR_NONE)
            {
                throw err::Exception(e, "load model failed");
            }
        }
    }

    MUD::~MUD()
    {
    }

    static void _get_section_keys(inifile::IniFile &ini, const char *section, std::vector<std::string> *keys)
    {
        inifile::IniSection *sect = ini.getSection(section);

        if (sect != NULL)
        {
            for (inifile::IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it)
            {
                keys->push_back(it->key);
            }
        }
    }

    err::Err MUD::load(const std::string &model_path)
    {
        if (model_path.empty() || !fs::exists(model_path.c_str()))
        {
            log::error("model path %s not exists\n", model_path.c_str());
            return err::ERR_ARGS;
        }
#if PLATFORM_MAIXCAM
        // if model_path end with .cvimodel, load cvi model
        if(model_path.find(".cvimodel") != std::string::npos)
        {
            return maixcam_load_cvimodel(model_path, this);
        }
#endif
        if(model_path.find(".mud") == std::string::npos)
        {
            log::error("model path %s not end with .mud\n", model_path.c_str());
            return err::ERR_ARGS;
        }

        inifile::IniFile ini;
        int ret = ini.Load(model_path);
        if (ret != 0)
        {
            log::error("parse model %s failed, err %d\n", model_path.c_str(), ret);
            return err::ERR_ARGS;
        }
        if (ini.GetStringValue("basic", "type", &this->type) != 0)
        {
            log::error("parse model %s failed, not found type\n", model_path.c_str());
            return err::ERR_ARGS;
        }
        std::vector<std::string> sections;
        ret = ini.GetSections(&sections);
        if ( ret <= 0)
        {
            log::error("parse model %s failed, get sections\n", model_path.c_str());
            return err::ERR_ARGS;
        }
        for (std::string &section : sections)
        {
            std::vector<std::string> keys;
            _get_section_keys(ini, section.c_str(), &keys);
            for (std::string &key : keys)
            {
                std::string value;
                if (ini.GetStringValue(section, key, &value) != 0)
                {
                    log::error("parse model %s failed, get keys\n", model_path.c_str());
                    return err::ERR_ARGS;
                }
                this->items[section][key] = value;
            }
        }
        return err::ERR_NONE;
    }

    NN::NN(const std::string &model_path)
    {
        _impl = nullptr;
#if PLATFORM_MAIXCAM
        _impl = new NN_MaixCam();
#endif
        if(!_impl)
        {
            throw err::Exception(err::ERR_NOT_IMPL, "NN not support this platform yet");
        }
        if (!model_path.empty())
        {
            err::Err e = load(model_path);
            if (e != err::ERR_NONE)
                throw err::Exception(e, "load model failed");
        }
    }

    NN::~NN()
    {
        unload();
        if (_impl)
        {
            delete _impl;
            _impl = nullptr;
        }
    }

    err::Err NN::load(const std::string &model_path)
    {
        if(_impl->loaded())
        {
            log::error("model already loaded\n");
            return err::ERR_NOT_PERMIT;
        }
        if(model_path.empty() || !fs::exists(model_path))
        {
            log::error("model path %s not exists\n", model_path.c_str());
            return err::ERR_ARGS;
        }
        err::Err e = _mud.load(model_path);
        if (e != err::ERR_NONE)
        {
            return e;
        }
        std::string dir = fs::abspath(fs::dirname(model_path));
        e = _impl->load(_mud, dir);
        if (e != err::ERR_NONE)
        {
            return e;
        }
        return err::ERR_NONE;
    }

    err::Err NN::unload()
    {
        return _impl->unload();
    }

    bool NN::loaded()
    {
        return _impl->loaded();
    }

    std::vector<nn::LayerInfo> NN::inputs_info()
    {
        return _impl->inputs_info();
    }

    std::vector<nn::LayerInfo> NN::outputs_info()
    {
        return _impl->outputs_info();
    }

    std::map<std::string, std::string> NN::extra_info()
    {
        return _mud.items["extra"];
    }

    err::Err NN::forward(tensor::Tensors &inputs, tensor::Tensors &outputs)
    {
        return _impl->forward(inputs, outputs);
    }

    tensor::Tensors *NN::forward(tensor::Tensors &inputs)
    {
        return _impl->forward(inputs);
    }

    tensor::Tensors *NN::forward_image(image::Image &img, std::vector<float> mean, std::vector<float> scale, image::Fit fit, bool copy_result)
    {
        return _impl->forward_image(img, mean, scale, fit, copy_result);
    }

    int SelfLearnClassifier::learn()
    {
        #if PLATFORM_MAIXCAM
            return maix_nn_self_learn_classifier_learn(_features, _features_sample, _feature_num);
        #else
            throw err::Exception(err::ERR_NOT_IMPL);
        #endif
    }

} // namespace maix::nn
