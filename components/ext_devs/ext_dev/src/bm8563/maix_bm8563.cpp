/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.6: Add framework, create this file.
 */

#include "bm8563.h"
#include "maix_i2c.hpp"
#include <memory>
#include <ctime>
#include <sys/time.h>
#include "maix_bm8563.hpp"
#include <mutex>

namespace maix::ext_dev::bm8563::priv {

// #define BM8563_DEBUG

#if 0
static const char* INFO = "This class will initialize the I2C bus when the program is first constructed, \
subsequent construction of the object will not initialize the I2C bus again will be out of the warning but \
does not affect the use of the new object. This APIs operations on the BM8563 are thread-safe.";
#endif

static const int DEFAULT_I2C_BUS_NUM = 4;
static const char* TAG = "MAIX BM8563";

/********************************************
 *
 * !!! Normally you don't need to modify the following global variables.
 *
 ********************************************/
static std::recursive_mutex mtx;
static bm8563_t bm8563;
static ::maix::peripheral::i2c::I2C* i2cdev{nullptr};
static int32_t dev_num{0};
static const std::vector<int> empty_time_tuple;

static int32_t maix_i2c_read(void *handle, uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t size)
{
    ::maix::Bytes* res = nullptr;

    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        /* i2cdev->readfrom_mem() */
        i2cdev->writeto((int)address, &reg, 1);
        res = i2cdev->readfrom((int)address, (int)size);
    }

    if (res == nullptr) return BM8563_ERROR_NOTTY;

    std::copy(res->data, res->data+res->data_len, buffer);

#ifdef BM8563_DEBUG
    int32_t len = res->data_len;
    log::info0("read %d: ", len);
    for (size_t i = 0; i < res->data_len; ++i) {
        printf("0x%x ", res->at(i));
    } printf("\n");
#endif // BM8563_DEBUG

    delete res;
    return BM8563_OK;
}

static int32_t maix_i2c_write(void *handle, uint8_t address, uint8_t reg, const uint8_t *buffer, uint16_t size)
{
    auto buff = new uint8_t[size+1];
    buff[0] = reg;
    std::copy(buffer, buffer+size, buff+1);

#ifdef BM8563_DEBUG
    log::info0("write %d: ", size);
    for (int i = 0; i < size; ++i) {
        printf("0x%x ", buffer[i]);
    } printf("\n");
#endif // BM8563_DEBUG

    {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        i2cdev->writeto((int)address, buff, size+1);
    }

    delete[] buff;
    return BM8563_OK;
}

static void maix_i2c_init(int bus)
{
    if (i2cdev) {
        dev_num++;
        log::warn("[%s] BM8563 already init, finish...", TAG);
        // log::info("[%s:%d] I2C already init, finish...", __PRETTY_FUNCTION__, __LINE__);
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (bus < 0)
        i2cdev = new ::maix::peripheral::i2c::I2C(DEFAULT_I2C_BUS_NUM, ::maix::peripheral::i2c::MASTER);
    else
        i2cdev = new ::maix::peripheral::i2c::I2C(bus, ::maix::peripheral::i2c::MASTER);
    bm8563.read = maix_i2c_read;
    bm8563.write = maix_i2c_write;
    bm8563.handle = nullptr;
    bm8563_init(&bm8563);
    dev_num++;
    // log::info("[%s] %s", TAG, INFO);
}

static err::Err maix_i2c_deinit()
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    dev_num--;
    if (dev_num > 0) return err::Err::ERR_NONE;
    if (bm8563_close(&bm8563) != BM8563_OK) {
        return err::Err::ERR_RUNTIME;
    }
    dev_num = 0;
    delete i2cdev;
    i2cdev = nullptr;
    return err::Err::ERR_NONE;
}

static std::vector<int> make_time_tuple(tm& t)
{
    // (year, month, day[, hour[, minute[, second]]]])
    return {
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec,
    };
}

static tm make_tm_from_timetuple(std::vector<int>& timetuple)
{
    tm t;
    ::memset(&t, 0x00, sizeof(tm));
    if (!timetuple.empty()) {
        t.tm_year = timetuple[0] - 1900;
        t.tm_mon = timetuple[1] - 1;
        t.tm_mday = timetuple[2];
        t.tm_hour = timetuple.size() > 3 ? timetuple[3] : 0;
        t.tm_min = timetuple.size() > 4 ? timetuple[4] : 0;
        t.tm_sec = timetuple.size() > 5 ? timetuple[5] : 0;
    }
    std::mktime(&t);
    return t;
}

static err::Err bm8563_err2maix_err(bm8563_err_t err)
{
    switch (err) {
    case BM8563_ERROR_NOTTY:
        return err::Err::ERR_IO; break;
    case BM8563_ERR_LOW_VOLTAGE: {
        maix::log::warn("[%s] Low voltage.", TAG);
        return err::Err::ERR_NONE; break;
    }
    default: break;
    };
    return err::Err::ERR_NONE;
}

static std::vector<int> make_timetuple_from_two(const std::vector<int>& first, const std::vector<int>& second)
{
    const std::vector<int>& old_timetuple = (first.size() == 6) ? first : second;
    const std::vector<int>& new_timetuple = (first.size() == 6) ? second : first;

    std::vector<int> result(new_timetuple.begin(), new_timetuple.end());
    result.resize(6);
    for (std::size_t i = new_timetuple.size(); i < 6; ++i) {
        result[i] = old_timetuple[i];
    }

    return result;
}

}


namespace maix::ext_dev::bm8563 {

BM8563::BM8563(int i2c_bus)
{
    priv::maix_i2c_init(i2c_bus);
}

BM8563::~BM8563()
{
    err::Err ret = this->deinit();
    if (ret != err::Err::ERR_NONE) {
        log::error("[%s][ERROR] ~BM8563 failed. Error code:%d", priv::TAG, ret);
    }
}


std::vector<int> BM8563::datetime(std::vector<int> timetuple)
{
    if (timetuple.empty()) {
        return this->now();
    }

    if (err::Err::ERR_NONE != this->init(timetuple))
        return timetuple;
    return priv::empty_time_tuple;
}

err::Err BM8563::init(std::vector<int> timetuple)
{
    if (timetuple.size() < 3 || timetuple.size() > 6) {
        maix::log::error("[%s] Error setting time!"
            " Reason: Invalid timetuple, it should be "
            "(year, month, day[, hour[, minute[, second]]])", priv::TAG);
        return err::Err::ERR_ARGS;
    }
    tm t;
    if (timetuple.size() == 6)
        t = priv::make_tm_from_timetuple(timetuple);
    else {
        auto now = this->now();
        if (now.empty()) {
            maix::log::error("[%s] Error setting time!"
            " Reason: Get empty timetuple!", priv::TAG);
            return err::Err::ERR_RUNTIME;
        }
        auto new_timetuple = priv::make_timetuple_from_two(now, timetuple);
        t = priv::make_tm_from_timetuple(new_timetuple);
    }
    return priv::bm8563_err2maix_err(bm8563_write(&priv::bm8563, &t));
}

std::vector<int> BM8563::now()
{
    tm t;
    auto ret = bm8563_read(&priv::bm8563, &t);

#ifdef BM8563_DEBUG
    char buffer[128];
    strftime(buffer, 128 ,"%c (day %j)" , &t);
    log::info("now RTC: %s\n", buffer);
#endif // BM8563_DEBUG

    // maix::log::info("ret: %d", ret);
    if (priv::bm8563_err2maix_err(ret) == err::Err::ERR_NONE)
        return priv::make_time_tuple(t);
    return priv::empty_time_tuple;
}

err::Err BM8563::deinit()
{
    return priv::maix_i2c_deinit();
}

err::Err BM8563::hctosys()
{
    auto now_time = this->now();
    tm t = priv::make_tm_from_timetuple(now_time);
    time_t sys_time = ::mktime(&t);
    if (sys_time == -1) {
        log::error("[%s] Failed to convert hardware time to system time.", priv::TAG);
        return err::Err::ERR_RUNTIME;
    }

    struct timeval tv;
    tv.tv_sec = sys_time;
    tv.tv_usec = 0;

    if (::settimeofday(&tv, nullptr) != 0) {
        log::error("[%s] Failed to set system time.", priv::TAG);
        return err::Err::ERR_RUNTIME;
    }

    log::info("[%s] Successfully set system time.", priv::TAG);
    return err::Err::ERR_NONE;
}

err::Err BM8563::systohc()
{
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) != 0) {
        log::error("[%s] Failed to get system time.", priv::TAG);
        return err::Err::ERR_RUNTIME;
    }

    time_t sys_time = tv.tv_sec;

    tm *t = localtime(&sys_time);
    if (t == nullptr) {
        log::error("[%s] Failed to convert system time to tm structure.", priv::TAG);
        return err::Err::ERR_RUNTIME;
    }

    if (this->init(priv::make_time_tuple(*t)) != 0) {
        log::error("[%s] Failed to set hardware time.", priv::TAG);
        return err::Err::ERR_RUNTIME;
    }

    log::info("[%s] Successfully set hardware time from system time.", priv::TAG);
    return err::Err::ERR_NONE;
}



}