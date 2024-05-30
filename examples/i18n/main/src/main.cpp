
#include "maix_basic.hpp"
#include "main.h"

using namespace maix;

const std::map<string, string> locale_zh_dict = {
    {"out", "输出"},
    {"hello", "你好"}
};

const std::map<string, string> locale_ja_dict = {
    // {"out", "出力"},
    {"hello", "こんにちは"}
};

const std::map<string, const std::map<string, string>> locales_dict = {
    {"zh", locale_zh_dict},
    {"ja", locale_ja_dict}
};


i18n::Trans trans(locales_dict);

int _main(int argc, char* argv[])
{
    log::info("system locale: %s\n", i18n::get_locale().c_str());
    log::info("%s: %s\n", trans.tr("out").c_str(), trans.tr("hello").c_str());

    trans.set_locale("zh");
    log::info("%s: %s\n", trans.tr("out").c_str(), trans.tr("hello").c_str());

    trans.set_locale("en");
    log::info("%s: %s\n", trans.tr("out").c_str(), trans.tr("hello").c_str());

    trans.set_locale("ja");
    log::info("%s: %s\n", trans.tr("out").c_str(), trans.tr("hello").c_str());
    return 0;
}


int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

