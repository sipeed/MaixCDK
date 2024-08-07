/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include "maix_err.hpp"
#include "maix_log.hpp"

using namespace std;

namespace maix::i18n
{
    /**
     * i18n locales list
     * @maixpy maix.i18n.locales
     */
    static std::vector<std::string> locales = {
        "en",
        "zh",
        "zh-tw",
        "ja"};

    /**
     * i18n language names list
     * @maixpy maix.i18n.names
     */
    const static std::vector<std::string> names = {
        "English",
        "简体中文",
        "繁體中文",
        "日本語"};

    /**
     * Get system config of locale.
     * @return language locale, e.g. en, zh, zh_CN, zh_TW, etc.
     * @maixpy maix.i18n.get_locale
     */
    string get_locale();

    /**
     * Get system config of language name.
     * @return language name, e.g. English, 简体中文, 繁體中文, etc.
     * @maixpy maix.i18n.get_language_name
     */
    string get_language_name();

    /**
     * Load translations from yaml files.
     * @param locales_dir translation yaml files directory.
     * @return A dict contains all translations, e.g. {"zh":{"hello": "你好"}, "en":{"hello": "hello"}}, you should delete it after use in C++.
     * @maixpy maix.i18n.load_trans_yaml
     */
    const std::map<string, std::map<string, string>> *load_trans_yaml(const std::string &locales_dir);

    /**
     * Load translations from yaml files.
     * @param locales_dir translation yaml files directory.
     * @param dict dict to store key values. A dict contains all translations, e.g. {"zh":{"hello": "你好"}, "en":{"hello": "hello"}}, you should delete it after use in C++.
     * @return err::ERR
     * @maixcdk maix.i18n.load_trans_yaml
     */
    err::Err load_trans_yaml(const std::string &locales_dir, std::map<string, std::map<string, string>> &dict);

    /**
     * Translate helper class.
     * @maixpy maix.i18n.Trans
     */
    class Trans
    {
    public:
        /**
         * Translate helper class constructor.
         * By default locale is get by `i18n.get_locale()` function which set by system settings.
         * But you can also manually set by `set_locale` function temporarily.
         * @param locales_dict locales dict, e.g. {"zh": {"Confirm": "确认", "OK": "好的"}, "en": {"Confirm": "Confirm", "OK": "OK"}}
         * @maixpy maix.i18n.Trans.__init__
         * @maixcdk maix.i18n.Trans.Trans
         */
        Trans(const std::map<string, const std::map<string, string>> &locales_dict = std::map<string, const std::map<string, string>>())
            : locales_dict_const(locales_dict)
        {
            this->locale = ""; // DO NOT load from file system here， will coredump
        }

        /**
         * Load translation from yaml files generated by `maixtool i18n` command.
         * @param locales_dir the translation files directory.
         * @return err.Err type, no error will return err.Err.ERR_NONE.
         * @maixpy maix.i18n.Trans.load
         */
        err::Err load(const std::string &locales_dir)
        {
            return load_trans_yaml(locales_dir, locales_dict);
        }

        /**
         * Update translation dict.
         * @param dict the new translation dict.
         * @return err.Err type, no error will return err.Err.ERR_NONE.
         * @maixpy maix.i18n.Trans.update_dict
         */
        err::Err update_dict(const std::map<std::string, const std::map<std::string, std::string>> &dict)
        {
            try
            {
                for (const auto &file_pair : dict)
                {
                    const std::string &filename = file_pair.first;
                    for (const auto &kv_pair : file_pair.second)
                    {
                        locales_dict[filename][kv_pair.first] = kv_pair.second;
                    }
                }
                return err::Err();
            }
            catch (const std::exception &e)
            {
                log::error("copy dict value failed");
                return err::Err::ERR_ARGS;
            }
        }

        /**
         * Translate string by key.
         * @param key string key, e.g. "Confirm"
         * @param locale locale name, if not assign, use default locale set by system settings or set_locale function.
         * @return translated string, if find translation, return it, or return key, e.g. "确认", "Confirm", etc.
         * @maixpy maix.i18n.Trans.tr
         */
        string tr(const string &key, const string locale = "")
        {
            if (this->locale.empty())
                this->locale = i18n::get_locale();
            // if locale not in locales_dict, return key
            if (!locales_dict.empty())
            {
                const std::map<string, std::map<string, string>>::const_iterator iter = locales_dict.find(locale.empty() ? this->locale : locale);
                if (iter == locales_dict.end())
                {
                    return key;
                }
                // get key value from locales_dict[locale][key], default value is key
                const std::map<string, string> dict = iter->second;
                const std::map<string, string>::const_iterator iter2 = dict.find(key);
                if (iter2 == dict.end())
                {
                    return key;
                }
                return iter2->second;
            }
            const std::map<string, const std::map<string, string>>::const_iterator iter = locales_dict_const.find(locale.empty() ? this->locale : locale);
            if (iter == locales_dict_const.end())
            {
                return key;
            }
            // get key value from locales_dict[locale][key], default value is key
            const std::map<string, string> dict = iter->second;
            const std::map<string, string>::const_iterator iter2 = dict.find(key);
            if (iter2 == dict.end())
            {
                return key;
            }
            return iter2->second;
        }

        /**
         * Set locale temporarily, will not affect system settings.
         * @param locale locale name, e.g. "zh", "en", etc. @see maix.i18n.locales
         * @maixpy maix.i18n.Trans.set_locale
         */
        void set_locale(const string &locale)
        {
            this->locale = locale;
        }

        /**
         * Get current locale.
         * @return locale name, e.g. "zh", "en", etc. @see maix.i18n.locales
         * @maixpy maix.i18n.Trans.get_locale
         */
        string get_locale()
        {
            if (this->locale.empty())
                this->locale = i18n::get_locale();
            return this->locale;
        }

    private:
        std::map<string, std::map<string, string>> locales_dict; // copy dict to here to avoid memory problem, e.g. For python
        std::map<string, const std::map<string, string>> locales_dict_const;
        string locale;
    };
}
