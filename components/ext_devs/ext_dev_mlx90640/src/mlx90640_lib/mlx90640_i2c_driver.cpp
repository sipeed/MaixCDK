

#define MLX90640_LINUX_SYSCALL_I2C  0
#define MLX90640_MAIX_I2C           1
#define MLX90640_SOFT_I2C           2

#define MLX_90640_I2C_MODE MLX90640_LINUX_SYSCALL_I2C

#if MLX_90640_I2C_MODE == MLX90640_LINUX_SYSCALL_I2C

#include "MLX90640_I2C_Driver.h"
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <string>

//#define I2C_MSG_FMT char
#ifndef I2C_FUNC_I2C
#include <linux/i2c.h>
#define I2C_MSG_FMT __u8
#endif

#include <sys/ioctl.h>

int i2c_fd = 0;
static int i2c_bus_num = -1;
static std::string i2c_device;

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    if (!i2c_fd) {
        i2c_fd = open(i2c_device.c_str(), O_RDWR);
    }

    char cmd[2] = {(char)(startAddress >> 8), (char)(startAddress & 0xFF)};
    char buf[1664];
    uint16_t *p = data;
    struct i2c_msg i2c_messages[2];
    struct i2c_rdwr_ioctl_data i2c_messageset[1];

    ::memset(buf, 0x00, std::size(buf));

    i2c_messages[0].addr = slaveAddr;
    i2c_messages[0].flags = 0;
    i2c_messages[0].len = 2;
    i2c_messages[0].buf = (I2C_MSG_FMT *)cmd;

    i2c_messages[1].addr = slaveAddr;
    i2c_messages[1].flags = I2C_M_RD;
    i2c_messages[1].len = nMemAddressRead * 2;
    i2c_messages[1].buf = (I2C_MSG_FMT *)buf;

    i2c_messageset[0].msgs = i2c_messages;
    i2c_messageset[0].nmsgs = 2;

    memset(buf, 0, nMemAddressRead * 2);

    if (ioctl(i2c_fd, I2C_RDWR, &i2c_messageset) < 0) {
        printf("I2C Read Error!\n");
        return -1;
    }

    for (int count = 0; count < nMemAddressRead; count++) {
        int i = count << 1;
        *p++ = ((uint16_t)buf[i] << 8) | buf[i + 1];
    }

    return 0;
}

void MLX90640_I2CFreqSet([[maybe_unused]]int freq)
{
#if 1 // PLATFORM_MAIXCAM
    if (i2c_bus_num == 5) {
        const char* priv_freq_path = "/sys/devices/platform/i2c5@gpio/udelay_value/udelay_v";
        const char* wd = "0";

        int _fd = ::open(priv_freq_path, O_WRONLY);
        if (_fd < 0) return;

        int ret = ::write(_fd, wd, 1);
        if (ret < 0) return;
    }
#endif
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    char cmd[4] = {(char)(writeAddress >> 8), (char)(writeAddress & 0x00FF), (char)(data >> 8), (char)(data & 0x00FF)};

    struct i2c_msg i2c_messages[1];
    struct i2c_rdwr_ioctl_data i2c_messageset[1];

    i2c_messages[0].addr = slaveAddr;
    i2c_messages[0].flags = 0;
    i2c_messages[0].len = 4;
    i2c_messages[0].buf = (I2C_MSG_FMT *)cmd;

    i2c_messageset[0].msgs = i2c_messages;
    i2c_messageset[0].nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &i2c_messageset) < 0) {
        printf("I2C Write Error!\n");
        // printf("0x%02x w<4>: 0x%02x 0x%02x 0x%02x 0x%02x\n", slaveAddr, cmd[0], cmd[1], cmd[2], cmd[3]);
        return -1;
    }

    return 0;
}

int MLX90640_I2CGeneralReset(void)
{
	MLX90640_I2CWrite(0x33,0x06,0x00);
	return 0;
}

void MLX90640_I2CInit(int bus_num)
{
    i2c_bus_num = bus_num;
    i2c_device = "/dev/i2c-" + std::to_string(bus_num);
    MLX90640_I2CFreqSet(0);
}


#endif