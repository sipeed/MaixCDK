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
        "ja"
    };

    /**
     * i18n language names list
     * @maixpy maix.i18n.names
    */
    const static std::vector<std::string> names = {
        "English",
        "简体中文",
        "繁體中文",
        "日本語"
    };

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
        Trans(const std::map<string, const std::map<string, string>> &locales_dict)
        :locales_dict(locales_dict)
        {
            this->locale = ""; // DO NOT load from file system here， will coredump
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
            if(this->locale.empty())
                this->locale = i18n::get_locale();
            // if locale not in locales_dict, return key
            const std::map<string, const std::map<string, string>>::const_iterator iter = locales_dict.find(locale.empty() ? this->locale : locale);
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
            if(this->locale.empty())
                this->locale = i18n::get_locale();
            return this->locale;
        }

    private:
        std::map<string, const std::map<string, string>> locales_dict; // copy dict to here to avoid memory problem, e.g. For python
        string locale;
    };
}