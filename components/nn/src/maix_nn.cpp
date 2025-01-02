/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include "maix_nn.hpp"
#include "maix_basic.hpp"
#include "inifile.h"
#include "maix_nn_self_learn_classifier.hpp"

#if PLATFORM_MAIXCAM
    #include "maix_nn_maixcam.hpp"
    #include "speech/dr_wav.h"
#endif


namespace maix::nn
{
    MUD::MUD(const std::string &model_path)
    {
        this->model_path = model_path;
        if (!model_path.empty())
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

    static err::Err _load_labels_from_file(std::vector<std::string> &labels, const std::string &label_path)
    {
        // load labels from labels file
        labels.clear();
        fs::File *f = fs::open(label_path, "r");
        if (!f)
        {
            log::error("open label file %s failed", label_path.c_str());
            return err::ERR_ARGS;
        }
        std::string line;
        while (f->readline(line) > 0)
        {
            // strip line
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            labels.push_back(line);
        }
        f->close();
        delete f;
        return err::ERR_NONE;
    }

    err::Err MUD::parse_labels(std::vector<std::string> &labels, const std::string key)
    {
        auto it = items["extra"].find(key);
        if (it == items["extra"].end())
        {
            log::error("Key %s not found in items['extra']", key.c_str());
            return err::Err::ERR_ARGS;
        }
        labels.clear();

        auto is_potential_file_path = [](const std::string& str) {
            // Consider it a file path if it doesn't contain commas or whitespace
            return str.find(',') == std::string::npos && str.find_first_of(" \t\r\n") == std::string::npos;
        };

        const std::string &label_value = it->second;
        std::string label_file = fs::dirname(model_path) + "/" + label_value;

        if (is_potential_file_path(label_value) && fs::exists(label_file) && fs::isfile(label_file))
        {
            err::Err result = _load_labels_from_file(labels, label_file);
            if (result != err::ERR_NONE)
            {
                log::error("Failed to load labels from file %s", label_file.c_str());
                return result;
            }
        }
        else
        {
            size_t start = 0;
            size_t end = label_value.find(',');

            while (end != std::string::npos)
            {
                std::string label = label_value.substr(start, end - start);

                // 去掉前后空字符
                label.erase(0, label.find_first_not_of(" \t\r\n"));
                label.erase(label.find_last_not_of(" \t\r\n") + 1);

                labels.push_back(label);
                start = end + 1;
                end = label_value.find(',', start);
            }

            // 处理最后一个标签
            std::string label = label_value.substr(start);
            label.erase(0, label.find_first_not_of(" \t\r\n"));
            label.erase(label.find_last_not_of(" \t\r\n") + 1);
            if (!label.empty())
            {
                labels.push_back(label);
            }
        }
        return err::Err::ERR_NONE;
    }

    std::vector<std::string> MUD::parse_labels(const std::string key)
    {
        std::vector<std::string> labels;
        parse_labels(labels, key);
        return labels;
    }


    err::Err MUD::load(const std::string &model_path)
    {
        this->model_path = model_path;
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

    NN::NN(const std::string &model_path, bool dual_buff)
    {
        _impl = nullptr;
#if PLATFORM_MAIXCAM
        _impl = new NN_MaixCam(dual_buff);
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

    void NN::set_dual_buff(bool enable)
    {
        _impl->set_dual_buff(enable);
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

    std::vector<std::string> NN::extra_info_labels()
    {
        return _mud.parse_labels();
    }

    err::Err NN::extra_info_labels(std::vector<std::string> &labels)
    {
        return _mud.parse_labels(labels);
    }

    nn::MUD &NN::mud()
    {
        return _mud;
    }

    err::Err NN::forward(tensor::Tensors &inputs, tensor::Tensors &outputs, bool copy_result, bool dual_buff_wait)
    {
        return _impl->forward(inputs, outputs, copy_result, dual_buff_wait);
    }

    tensor::Tensors *NN::forward(tensor::Tensors &inputs, bool copy_result, bool dual_buff_wait)
    {
        return _impl->forward(inputs, copy_result, dual_buff_wait);
    }

    tensor::Tensors *NN::forward_image(image::Image &img, std::vector<float> mean, std::vector<float> scale, image::Fit fit, bool copy_result, bool dual_buff_wait, bool chw)
    {
        return _impl->forward_image(img, mean, scale, fit, copy_result, dual_buff_wait, chw);
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
