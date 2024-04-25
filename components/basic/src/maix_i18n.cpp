/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_i18n.hpp"
#include "maix_app.hpp"

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

} // namespace maix::i18n
