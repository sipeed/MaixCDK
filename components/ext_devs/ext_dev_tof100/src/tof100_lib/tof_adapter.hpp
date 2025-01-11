/**
 * @file tof_adapter.hpp
 * @author iawak9lkm
 * @brief
 * @version 0.1
 * @date 2024-12-05
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __TOF_ADAPTER_HPP__
#define __TOF_ADAPTER_HPP__

#include <memory>
#include <stdexcept>

#include <cstdio>
#include "maix_basic.hpp"
#include "maix_gpio.hpp"
#include "maix_spi.hpp"


#define bflb_platform_delay_ms(ms) maix::time::sleep_ms(ms)
#define eprintln(fmt, ...) do {maix::log::error0("[%s]", __PRETTY_FUNCTION__); \
                                printf(fmt, ##__VA_ARGS__);\
                                printf("\n");} while(0)
#define println(fmt, ...) do {maix::log::info0("[%s]", __PRETTY_FUNCTION__); \
                                printf(fmt, ##__VA_ARGS__);\
                                printf("\n");} while(0)
#define usbd_printf println

#define panic(fmt, ...) do {eprintln(fmt, ##__VA_ARGS__);\
                            char _buff[128]{0x00}; \
                            ::snprintf(_buff, std::size(_buff), "Panic in \n\tfile <%s> \n\tfunc <%s> \n\tlen <%d>\n", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
                            throw std::runtime_error(std::string(_buff));} while(0)

#define TOF_CS_PIN 0
#define gpio_write(_a, _b)
#define arch_memset ::memset
#define arch_memcpy_fast ::memcpy
#define arm_sqrt_f32 sqrtf

#define SPI_TRANSFER_TYPE_32BIT   3

using namespace maix::peripheral::spi;
using namespace maix::peripheral::gpio;

inline static std::unique_ptr<SPI> _spi{nullptr};

#define g_spi (_spi.get())

int tof_init(int spi_id)
{
    _spi = std::move(std::make_unique<SPI>(spi_id, MASTER, 20000000));

    std::vector<uint8_t> data;
    data.push_back(0x00);

    for (int i = 0; i < 4; ++i)
        g_spi->write(data);

    return 0;
}

void clone_with_rearrange_tx(const uint32_t* from, uint32_t fsize, uint8_t* to)
{
    for (uint32_t i = 0; i < fsize; ++i) {
        uint32_t value = from[i];
        to[i * 4 + 3] = static_cast<uint8_t>(value & 0xFF);
        to[i * 4 + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        to[i * 4 + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        to[i * 4 + 0] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }
}

void clone_with_rearrange_rx(const uint8_t* from, uint32_t fsize, uint32_t* to) {
    for (uint32_t i = 0; i < fsize; ++i) {
        uint32_t value = 0;
        value |= static_cast<uint32_t>(from[i * 4 + 3]) << 0;
        value |= static_cast<uint32_t>(from[i * 4 + 2]) << 8;
        value |= static_cast<uint32_t>(from[i * 4 + 1]) << 16;
        value |= static_cast<uint32_t>(from[i * 4 + 0]) << 24;
        to[i] = value;
    }
}

int spi_transmit_receive(SPI* dev, const void *send_buf, void *recv_buf, uint32_t length, uint8_t type)
{
    if (type != 3)
        return -1;

    uint32_t u8_len = length*4;
    auto tx_buff = std::unique_ptr<uint8_t[]>(new uint8_t[u8_len]);
    clone_with_rearrange_tx(reinterpret_cast<const uint32_t*>(send_buf), length, tx_buff.get());
    maix::Bytes tx_b(tx_buff.get(), u8_len, false, false);

    auto rx_b = std::unique_ptr<maix::Bytes>(dev->write_read(&tx_b, u8_len));
    clone_with_rearrange_rx(rx_b->data, length, reinterpret_cast<uint32_t*>(recv_buf));

#if 0
    maix::log::info0("spi_transmit_receive [RW]Tx: ");
    for (int i = 0; i < u8_len; ++i) {
        printf("[%02x]", tx_buff[i]);
    } printf("\n");

    maix::log::info0("spi_transmit_receive[RW]Rx: ");
    for (int i = 0; i < u8_len; ++i) {
        printf("[%02x]", rx_b->data[i]);
    } printf("\n");
#endif

    return 0;
}

int spi_transmit(SPI* dev, void *buffer, uint32_t size, uint8_t type)
{
    if (type != 3)
        return -1;

    uint32_t u8_len = size*4;
    auto tx_buff = std::unique_ptr<uint8_t[]>(new uint8_t[u8_len]);
    clone_with_rearrange_tx(reinterpret_cast<const uint32_t*>(buffer), size, tx_buff.get());
    maix::Bytes tx_b(tx_buff.get(), u8_len, false, false);

#if 0
    maix::log::info0("spi_transmit [W]Tx: ");
    for (int i = 0; i < u8_len; ++i) {
        printf("[%02x]", tx_buff[i]);
    } printf("\n");
#endif

    return dev->write(&tx_b)>0 ? 0 : -1;
}

#endif