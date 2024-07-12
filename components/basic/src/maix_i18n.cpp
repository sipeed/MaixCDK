/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_i18n.hpp"
#include "maix_app.hpp"
#include <yaml-cpp/yaml.h>
#include "maix_fs.hpp"

namespace maix::i18n
{
    string get_locale()
    {
        std::string locale = app::get_sys_config_kv("language", "locale");
        if (locale.empty())
        {
            locale = "en";
        }
        return locale;
    }

    string get_language_name()
    {
        string locale = get_locale();
        for (size_t i = 0; i < locales.size(); i++)
        {
            if (locales[i] == locale)
            {
                return names[i];
            }
        }
        return "English";
    }

    const std::map<std::string, std::map<std::string, std::string>> *load_trans_yaml(const std::string &locales_dir)
    {
        auto *translations = new std::map<std::string, std::map<std::string, std::string>>();

        // 遍历目录中的所有文件
        std::vector<std::string> *files = fs::listdir(locales_dir, true, true);
        for (std::string file : *files)
        {
            std::string filename = fs::basename(file);
            std::vector<std::string> splitname = fs::splitext(file);
            if (splitname[1] == ".yaml")
            {
                YAML::Node node = YAML::LoadFile(file);
                std::map<std::string, std::string> content;

                // 遍历 YAML 文件中的所有键值对
                for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
                {
                    content[it->first.as<std::string>()] = it->second.as<std::string>();
                }

                (*translations)[splitname[0]] = content;
            }
        }
        delete files;
        return translations;
    }

    err::Err load_trans_yaml(const std::string &locales_dir, std::map<string, std::map<string, string>> &dict)
    {
        if(!fs::exists(locales_dir))
        {
            log::error("dir [ %s ] not found", locales_dir.c_str());
            return err::ERR_ARGS;
        }

        // 遍历目录中的所有文件
        std::vector<std::string> *files = fs::listdir(locales_dir, true, true);
        if(!files)
        {
            log::error("no trans yaml files");
            return err::ERR_ARGS;
        }
        for (std::string file : *files)
        {
            std::string filename = fs::basename(file);
            std::vector<std::string> splitname = fs::splitext(filename);
            if (splitname[1] == ".yaml")
            {
                YAML::Node node = YAML::LoadFile(file);
                std::map<std::string, std::string> content;

                // 遍历 YAML 文件中的所有键值对
                for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
                {
                    content[it->first.as<std::string>()] = it->second.as<std::string>();
                }

                dict[splitname[0]] = content;
            }
        }
        delete files;
        return err::ERR_NONE;
    }

} // namespace maix::i18n
