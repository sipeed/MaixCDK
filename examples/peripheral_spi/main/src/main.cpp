#include "maix_basic.hpp"
#include "main.h"
#include "maix_spi.hpp"
#include "maix_pinmap.hpp"
#include <map>


#define USE_SPI4

#ifndef USE_SPI4
#define USE_SPI2
#endif

using namespace maix;
using namespace maix::peripheral;

int _main(int argc, char* argv[])
{
    log::info("Program start");

    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    /**
     * The MaixCAM(Soc:SG2002) SPI only supported by the master.
    */

#ifdef USE_SPI4
    const std::map<std::string, std::string> pin_functions{
        {"A24", "SPI4_CS"},     /* Default CS of SPI4, currently the valid level of this CS is high
                                    and does not support to change, please wait for the fix.
                                    You can specify another GPIO as the CS pin. */
        {"A23", "SPI4_MISO"},   /* MISO */
        {"A25", "SPI4_MOSI"},   /* MOSI */
        {"A22", "SPI4_SCK"},    /* SCK */
        //{"A27", "GPIOA27"}      /* SOFT CS */
    };

    log::info("Use pinmap to configure SPI4.");
    for (const auto& item : pin_functions) {
        if (err::ERR_NONE != pinmap::set_pin_function(item.first, item.second)) {
            log::error("Set pin{%s} to function{%s} failed!",
                item.first.c_str(), item.second.c_str());
            return -1;
        }
    }

    log::info("Init spi4");
    spi::SPI dev(4, spi::Mode::MASTER, 1250000);   /* Use SPI4_CS */
    // spi::SPI dev(4, spi::Mode::MASTER, 1250000, 0, 0, 8, 0, true, "A27"); /* Use SOFT CS */
    log::info("Init spi4 end");
#endif
#ifdef USE_SPI2
    const std::map<std::string, std::string> pin_functions{
        {"P18", "SPI2_CS"},
        {"P21", "SPI2_MISO"},
        {"P22", "SPI2_MOSI"},
        {"P23", "SPI2_SCK"}
    };

    log::info("Use pinmap to configure SPI2.");
    for (const auto& item : pin_functions) {
        if (err::ERR_NONE != pinmap::set_pin_function(item.first, item.second)) {
            log::error("Set pin{%s} to function{%s} failed!",
                item.first.c_str(), item.second.c_str());
            return -1;
        }
    }

    log::info("Init spi2");
    spi::SPI dev(2, spi::Mode::MASTER, 400000);
    log::info("Init spi2 end");
#endif



    unsigned char str[] = "hello world\r\n";
    std::vector<unsigned char> v(str, str+sizeof(str));

    while(!app::need_exit())
    {
        /* Full-duplex transmission. */
        {
            auto res = dev.write_read(v, v.size());
            if (!res.size()) {
                log::error("Read error.");
                break;
            }

            log::info0("Send %d Bytes: ", v.size());
            for (auto i : v)
                printf("0x%02x ", i);
            printf("\n");

            log::info0("Recv %d Bytes: ", res.size());
            for (auto i : res)
                printf("0x%02x ", i);
            printf("\n");
        }

        app::set_exit_flag(true);
    }
    log::info("Program exit");

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch SIGINT signal(e.g. Ctrl + C), and set exit flag to true.
    signal(SIGINT, [](int sig){ app::set_exit_flag(true); });

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


