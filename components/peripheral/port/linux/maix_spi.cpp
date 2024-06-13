/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_spi.hpp"
#include "maix_log.hpp"
#include "cstdio"

namespace maix::peripheral::spi
{
    SPI::SPI(int id, spi::Mode mode, int freq, int polarity, int phase, int bits, unsigned char cs_enable, bool soft_cs, std::string cs)
    {
        throw err::Exception(err::Err::ERR_NOT_IMPL, "SPI not implemented");
    }

    SPI::~SPI()
    {
    }

    Bytes *SPI::read(int length)
    {
        return nullptr;
    }

    int SPI::write(std::vector<unsigned char> data)
    {
        return 0;
    }

    int SPI::write(Bytes *data)
    {
        return 0;
    }

    std::vector<unsigned char> SPI::write_read(std::vector<unsigned char> data, int read_len)
    {
        return std::vector<unsigned char>();
    }

    Bytes *SPI::write_read(Bytes *data, int read_len)
    {
        return nullptr;
    }

    bool SPI::is_busy()
    {
        return false;
    }
}; // namespace maix
