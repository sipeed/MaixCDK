
#include "maix_basic.hpp"
#include "main.h"
#include "maix_spi.hpp"
#include "maix_pinmap.hpp"

using namespace maix;

using namespace maix::peripheral;

void print_bytes(Bytes* bytes)
{
    for (long unsigned int i = 0; i < bytes->data_len; ++i)
        log::info("\t[%d] = 0x%x", i, (*bytes)[i]);
}

void print_bytes(std::vector<unsigned char>& v)
{
    int cnt = 0;
    for (auto i : v) {
        log::info("\t[%d] = 0x%x", cnt, i);
        ++cnt;
    }      
}

int _main(int argc, char* argv[])
{
    log::info("Program start");

    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    /**
     * The MaixCAM(Soc:SG2002) SPI only supported by the master.
    */


    log::info("Use pinmap to configure SPI1.");
    pinmap::set_pin_function("A24", "SPI1_CS");
    pinmap::set_pin_function("A23", "SPI1_MISO");
    pinmap::set_pin_function("A25", "SPI1_MOSI");
    pinmap::set_pin_function("A22", "SPI1_SCK");


    log::info("Init spi1");
    // spi::SPI dev(1, spi::Mode::MASTER, 400000, true, 0, 0, 8, "A19", 0);
    spi::SPI dev(1, spi::Mode::MASTER, 400000);
    log::info("Init spi1 end");

    unsigned char str[] = "hello world\r\n";
    Bytes b(str, sizeof(str));
    std::vector<unsigned char> v(str, str+sizeof(str));

    log::info("====================DATA======================");
    log::info("Bytes:");
    print_bytes(&b);
    log::info("");
    log::info("vector:");
    print_bytes(v);
    log::info("==============================================");


    while(!app::need_exit())
    {

        int ret = -1;

        /* Full-duplex transmission. */
        {
            log::info("");
            log::info("write_read start...");
            /* Specifies that the number of bytes read is 2 bytes 
                less than the number of bytes sent. */
            int rlen = v.size() - 2;
            auto res = dev.write_read(v, rlen);
            if (!res.size()) {
                log::error("read error");
                break;
            }
            log::info("write_read %d bytes:", res.size());
            print_bytes(res);
            for (int i = 0; i < rlen; ++i) {
                if (res[i] != b[i]) {
                    log::error("loopback check Failed! w[%d]{0x%x}, r[%d]{0x%x}", 
                                    i, b[i], i, res[i]);
                    break;
                }
            }
            log::info("write_read end");
            log::info("");
        }

#if 0
        /* Half-duplex transmission. */
        {
            log::info("");
            log::info("write start...");
            ret = dev.write(&b);
            log::info("write %d bytes", ret);
            ret = dev.write(v);
            log::info("write %d bytes", ret);
            log::info("write end");
            log::info("");
        }

        /* Half-duplex transmission. */
        {
            log::info("");
            log::info("read start...");
            auto res = dev.read(10);
            if (nullptr == res) {
                log::error("read error");
                break;
            }
            print_bytes(res);
            delete res;
            log::info("read end");
            log::info("");
        }

        /* Full-duplex transmission. */
        {
            log::info("");
            log::info("write_read start...");
            int rlen = b.data_len+8;
            auto res = dev.write_read(&b, rlen);
            if (nullptr == res) {
                log::error("read error");
                break;
            }
            log::info("write_read %d bytes:", res->data_len);
            print_bytes(res);
            for (long unsigned int i = 0; i < b.data_len; ++i) {
                if ((*res)[i] != b[i]) {
                    log::error("loopback check Failed! w[%d]{0x%x}, r[%d]{0x%x}", 
                                    i, b[i], i, (*res)[i]);
                    break;
                }
            }
            delete res;
            log::info("write_read end");
            log::info("");
        }
#endif

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


