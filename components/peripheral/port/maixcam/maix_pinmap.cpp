/**
 * @author neucrack@sipeed iawak9lkm@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.5.13: create this file.
 */

#include "maix_pinmap.hpp"

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace maix::peripheral::pinmap
{
    /**
     * @brief ismod module.ko or rmmod module.ko
     *
     * @param is_install true means insmod, false means rmmod
     * @param dev_node device node, e.g. /dev/spidev4.0
     * @param module_path path to module files, e.g. /mnt/system/ko/,
     *                      this parameter is ignored if the module is uninstalled.
     * @param module_files modules that need to be executed,
     *                      starting with vector::begin for installing modules and vector::end()-1 for uninstalling modules.
     * @return err::Err
     */
    static inline err::Err __insmod(bool is_install, const std::string& dev_node,
                                    const std::string& module_path, const std::vector<std::string>& module_files)
    {
        if (!is_install) {
            // rmmod
            if (fs::exists(dev_node)) {
                for (int i = module_files.size()-1; i >= 0; --i) {
                    int ret = ::system((std::string("rmmod ")+module_files[i]).c_str());
                    if (ret != 0) {
                        log::error("rmmod %s error: %d", module_files[i].c_str(), ret);
                        return err::ERR_RUNTIME;
                    }
                }
            }
            return err::ERR_NONE;
        }
        // insmod
        if (!fs::exists(dev_node)) {
            for (auto& mod : module_files) {
                int ret = ::system((std::string("insmod ")+module_path+mod).c_str());
                if (ret != 0) {
                    log::error("insmod %s error: %d", mod.c_str(), ret);
                    return err::ERR_RUNTIME;
                }
            }
        }
        return err::ERR_NONE;
    }
    static inline err::Err __insmod_spi4(bool is_install)
    {
        static const std::string dev_node("/dev/spidev4.0");
        static const std::string module_path("/mnt/system/ko/");
        static const std::vector<std::string> module_files{"spi-bitbang.ko", "spi-gpio.ko"};
        return __insmod(is_install, dev_node, module_path, module_files);
    }
// 假设我们的系统页大小为4KB
#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)

    static int set_pinmux(uint64_t addr, uint32_t value)
    {
        int fd;
        void *map_base, *virt_addr;

        /* 打开 /dev/mem 文件 */
        if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        {
            perror("Error opening /dev/mem");
            return -1;
        }

        /* 映射需要访问的物理内存页到进程空间 */
        map_base = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~PAGE_MASK);
        if (map_base == (void *)-1)
        {
            perror("Error mapping memory");
            close(fd);
            return -1;
        }

        /* 计算目标寄存器的虚拟地址 */
        virt_addr = (char *)map_base + (addr & PAGE_MASK);

        /* 写入值到目标寄存器 */
        *((uint32_t *)virt_addr) = value;

        /* 取消映射并关闭文件描述符 */
        if (munmap(map_base, PAGE_SIZE) == -1)
        {
            perror("Error unmapping memory");
        }

        close(fd);

        return 0;
    }

    std::vector<std::string> get_pins()
    {
        std::vector<std::string> pins = {
            "A14",
            "A15",
            "A16",
            "A17",
            "A18",
            "A19",
            "A22",
            "A23",
            "A24",
            "A25",
            "A26",
            "A27",
            "A28",
            "A29",
            "B3",
            "B26",
            "B27",
            "C2",
            "P18",
            "P19",
            "P20",
            "P21",
            "P22",
            "P23",
        };
        return pins;
    }

    static void _config_eth_pin(bool en) {
        if (en) {
            set_pinmux(0x03009070, 0x0000BABE);
            set_pinmux(0x03009074, 0x0000BABE);
            set_pinmux(0x03009078, 0x0000BABE);
            set_pinmux(0x0300907c, 0x0000BABE);
            time::sleep_us(10);
            set_pinmux(0x03009800, 0x0000090E);
            set_pinmux(0x03009808, 0x00000188);
            set_pinmux(0x03009804, 0x00000000);
        } else {
            set_pinmux(0x03009804, 0x00000001); // [0] = 1
            set_pinmux(0x03009808, 0x00000181); // [4:0] = 0x01
            set_pinmux(0x03009800, 0x00000905); // 0x905
            time::sleep_us(10);
            set_pinmux(0x0300907c, 0x0000A5BE); // [12:8] = 0x5
            set_pinmux(0x03009078, 0x0000BF00); // [11:0] = 0xF00
            set_pinmux(0x03009074, 0x00000606); // 0x606
            set_pinmux(0x03009070, 0x00000606); // 0x606
        }
    }

    std::vector<std::string> get_pin_functions(const std::string &pin)
    {
        if (pin == "A14")
        {
            std::vector<std::string> funcs = {
                "GPIOA14"};
            return funcs;
        }
        else if (pin == "A15")
        {
            std::vector<std::string> funcs = {
                "GPIOA15",
                "I2C5_SCL"};
            return funcs;
        }
        else if (pin == "A16")
        {
            std::vector<std::string> funcs = {
                "GPIOA16",
                "PWM4",
                "UART0_TX"};
            return funcs;
        }
        else if (pin == "A17")
        {
            std::vector<std::string> funcs = {
                "GPIOA17",
                "PWM5",
                "UART0_RX"};
            return funcs;
        }
        else if (pin == "A18")
        {
            std::vector<std::string> funcs = {
                "GPIOA18",
                "PWM6",
                "UART1_RX",
                "JTAG_TCK"};
            return funcs;
        }
        else if (pin == "A19")
        {
            std::vector<std::string> funcs = {
                "GPIOA19",
                "PWM7",
                "UART1_TX",
                "JTAG_TMS"};
            return funcs;
        }
        else if (pin == "A22")
        {
            std::vector<std::string> funcs = {
                "GPIOA22",
                "SPI4_SCK"};
            return funcs;
        }
        else if (pin == "A23")
        {
            std::vector<std::string> funcs = {
                "GPIOA23",
                "SPI4_MISO"};
            return funcs;
        }
        else if (pin == "A24")
        {
            std::vector<std::string> funcs = {
                "GPIOA24",
                "SPI4_CS"};
            return funcs;
        }
        else if (pin == "A25")
        {
            std::vector<std::string> funcs = {
                "GPIOA25",
                "SPI4_MOSI"};
            return funcs;
        }
        else if (pin == "A26")
        {
            std::vector<std::string> funcs = {
                "GPIOA26",
            };
            return funcs;
        }
        else if (pin == "A27")
        {
            std::vector<std::string> funcs = {
                "GPIOA27",
                "I2C5_SDA"};
            return funcs;
        }
        else if (pin == "A28")
        {
            std::vector<std::string> funcs = {
                "GPIOA28",
                "UART2_TX",
                "JTAG_TDI"};
            return funcs;
        }
        else if (pin == "A29")
        {
            std::vector<std::string> funcs = {
                "GPIOA29",
                "UART2_RX",
                "JTAG_TDO"};
            return funcs;
        }
        else if (pin == "B3")
        {
            return std::vector<std::string>{
                "GPIOB3",
                "ADC"
            };
        }
        else if (pin == "B26")
        {
            return std::vector<std::string>{
                "GPIOB26",
                "PWM14"
            };
        }
        else if (pin == "B27")
        {
            return std::vector<std::string>{
                "GPIOB27",
                "PWM15"
            };
        }
        else if (pin == "C2")
        {
            return std::vector<std::string>{
                "GPIOC2",
                "CAM_MCLK0"
            };
        }
        else if (pin == "P18")
        {
            std::vector<std::string> funcs = {
                "GPIOP18",
                "UART3_CTS",
                "I2C1_SCL",
                "PWM4",
                "SPI2_CS",
                "SDIO1_D3"};
            return funcs;
        }
        else if (pin == "P19")
        {
            std::vector<std::string> funcs = {
                "GPIOP19",
                "UART3_TX",
                "PWM5",
                "SDIO1_D2"};
            return funcs;
        }
        else if (pin == "P20")
        {
            std::vector<std::string> funcs = {
                "GPIOP20",
                "UART3_RX",
                "PWM6",
                "SDIO1_D1"};
            return funcs;
        }
        else if (pin == "P21")
        {
            std::vector<std::string> funcs = {
                "GPIOP21",
                "UART3_RTS",
                "I2C1_SDA",
                "PWM7",
                "SPI2_MISO",
                "SDIO1_D0"};
            return funcs;
        }
        else if (pin == "P22")
        {
            std::vector<std::string> funcs = {
                "GPIOP22",
                "I2C3_SCL",
                "PWM8",
                "SPI2_MOSI",
                "SDIO1_CMD"};
            return funcs;
        }
        else if (pin == "P23")
        {
            std::vector<std::string> funcs = {
                "GPIOP23",
                "I2C3_SDA",
                "PWM9",
                "SPI2_SCK",
                "SDIO1_CLK"};
            return funcs;
        }
        else if (pin == "P24")
        {
            std::vector<std::string> funcs = {
                "GPIOP24",
                "PWM2",};
            return funcs;
        }
        else
        {
            throw err::Exception(err::ERR_ARGS);
        }
    }

    err::Err set_pin_function(const std::string &pin, const std::string &func)
    {
        if (pin == "A14")
        {
            return err::ERR_NONE;
        }
        else if (pin == "A15")
        {
            if (func == "GPIOA15")
            {
                // rmmod
                if (fs::exists("/dev/i2c-5"))
                {
                    int ret = system("rmmod i2c-gpio");
                    if (ret != 0)
                    {
                        log::error("rmmod error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                    ret = system("rmmod i2c-algo-bit");
                    if (ret != 0)
                    {
                        log::error("rmmod error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                }
            }
            else if (func == "I2C5_SCL")
            {
                // insmod
                if (!fs::exists("/dev/i2c-5"))
                {
                    int ret = system("insmod /mnt/system/ko/i2c-algo-bit.ko");
                    if (ret != 0)
                    {
                        log::error("insmod i2c-algo-bit.ko error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                    ret = system("insmod /mnt/system/ko/i2c-gpio.ko");
                    if (ret != 0)
                    {
                        log::error("insmod i2c-gpio error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                }
            }
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A16")
        {
            if (func == "GPIOA16") {
                set_pinmux(0x03001040, 3);
                set_pinmux(0x0300190c, 0x44);
            } else if (func == "UART0_TX") {
                set_pinmux(0x03001040, 0);
                set_pinmux(0x0300190c, 0x84);
            } else if (func == "PWM4") {
                set_pinmux(0x03001040, 2);
                set_pinmux(0x0300190c, 0x84);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "A17")
        {
            if (func == "GPIOA17") {
                set_pinmux(0x03001044, 3);
                set_pinmux(0x03001910, 0x44);
            } else if (func == "UART0_RX") {
                set_pinmux(0x03001044, 0);
                set_pinmux(0x03001910, 0x44);
            } else if (func == "PWM5") {
                set_pinmux(0x03001044, 2);
                set_pinmux(0x03001910, 0x84);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "A18")
        {
            if (func == "GPIOA18") {
                set_pinmux(0x03001068, 3);
                set_pinmux(0x03001934, 0x44);
            } else if (func == "UART1_RX") {
                set_pinmux(0x03001068, 6);
                set_pinmux(0x03001934, 0x44);
            } else if (func == "PWM6") {
                set_pinmux(0x03001068, 2);
                set_pinmux(0x03001934, 0x84);
            }  else if (func == "JTAG_TCK") {
                set_pinmux(0x03001068, 0);
                set_pinmux(0x03001934, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "A19")
        {
            if (func == "GPIOA19") {
                set_pinmux(0x03001064, 3);
                set_pinmux(0x03001930, 0x44);
            } else if (func == "UART1_TX") {
                set_pinmux(0x03001064, 6);
                set_pinmux(0x03001930, 0x44);
            } else if (func == "PWM7") {
                set_pinmux(0x03001064, 2);
                set_pinmux(0x03001930, 0x84);
            } else if (func == "JTAG_TMS") {
                set_pinmux(0x03001064, 0);
                set_pinmux(0x03001930, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "A22")
        {
            if (func == "GPIOA22") {
                set_pinmux(0x03001050, 3);
                return __insmod_spi4(false);
            } else if (func == "SPI4_SCK") {
                set_pinmux(0x03001050, 3);
                return __insmod_spi4(true);
            } else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A23")
        {
            if (func == "GPIOA23") {
                set_pinmux(0x0300105C, 3);
                return __insmod_spi4(false);
            } else if (func == "SPI4_MISO") {
                set_pinmux(0x0300105C, 3);
                return __insmod_spi4(true);
            } else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A24")
        {
            if (func == "GPIOA24") {
                set_pinmux(0x03001060, 3);
                return __insmod_spi4(false);
            } else if (func == "SPI4_CS") {
                set_pinmux(0x03001060, 3);
                return __insmod_spi4(true);
            } else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A25")
        {
            if (func == "GPIOA25") {
                set_pinmux(0x03001054, 3);
                return __insmod_spi4(false);
            } else if (func == "SPI4_MOSI") {
                set_pinmux(0x03001054, 3);
                return __insmod_spi4(true);
            } else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A26")
        {
            if (func == "GPIOA26")
                set_pinmux(0x0300104C, 3);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A27")
        {
            if (func == "GPIOA27")
            {
                set_pinmux(0x03001058, 3);
                // rmmod
                if (fs::exists("/dev/i2c-5"))
                {
                    int ret = system("rmmod i2c-gpio");
                    if (ret != 0)
                    {
                        log::error("rmmod error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                    ret = system("rmmod i2c-algo-bit");
                    if (ret != 0)
                    {
                        log::error("rmmod error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                }
            }
            else if (func == "I2C5_SDA")
            {
                set_pinmux(0x03001058, 3);
                // insmod
                if (!fs::exists("/dev/i2c-5"))
                {
                    int ret = system("insmod /mnt/system/ko/i2c-algo-bit.ko");
                    if (ret != 0)
                    {
                        log::error("insmod i2c-algo-bit.ko error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                    ret = system("insmod /mnt/system/ko/i2c-gpio.ko");
                    if (ret != 0)
                    {
                        log::error("insmod i2c-gpio error: %d", ret);
                        return err::ERR_RUNTIME;
                    }
                }
            }
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A28")
        {
            if (func == "GPIOA28")
                set_pinmux(0x03001070, 3);
            else if (func == "UART2_TX")
                set_pinmux(0x03001070, 2);
            else if (func == "JTAG_TDI")
                set_pinmux(0x03001070, 0);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A29")
        {
            if (func == "GPIOA29")
                set_pinmux(0x03001074, 3);
            else if (func == "UART2_RX")
                set_pinmux(0x03001074, 2);
            else if (func == "JTAG_TDO")
                set_pinmux(0x03001074, 0);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "B3")
        {
            set_pinmux(0x030010F8, 3);
            return err::ERR_NONE;
        }
        else if (pin == "B26")
        {
            if (func == "GPIOB26") {
                _config_eth_pin(false);
                set_pinmux(0x03001130, 3);
            } else if (func == "PWM14") {
                _config_eth_pin(false);
                set_pinmux(0x03001130, 4);
            } else {
                return err::ERR_ARGS;
            }
            return err::ERR_NONE;
        }
        else if (pin == "B27")
        {
            if (func == "GPIOB27") {
                _config_eth_pin(false);
                set_pinmux(0x0300112C, 3);
            } else if (func == "PWM15") {
                _config_eth_pin(false);
                set_pinmux(0x0300112C, 4);
            } else {
                return err::ERR_ARGS;
            }
            return err::ERR_NONE;
        }
        else if (pin == "C2")
        {
            if (func == "GPIOC2")
                set_pinmux(0x0300116C, 3);
            else if (func == "CAM_MCLK0")
                set_pinmux(0x0300116C, 5);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "P18")
        {
            if (func == "GPIOP18") {
                set_pinmux(0x030010D0, 3);
                set_pinmux(0x05027058, 0x44);
            } else if (func == "UART3_CTS") {
                set_pinmux(0x030010D0, 5);
                set_pinmux(0x05027058, 0x44);
            } else if (func == "I2C1_SCL") {
                set_pinmux(0x030010D0, 2);
                set_pinmux(0x05027058, 0x44);
            } else if (func == "PWM4") {
                set_pinmux(0x030010D0, 7);
                set_pinmux(0x05027058, 0x84);
            } else if (func == "SPI2_CS") {
                set_pinmux(0x030010D0, 1);
                set_pinmux(0x05027058, 0x44);
            } else if (func == "SDIO1_D3") {
                set_pinmux(0x030010D0, 0);
                set_pinmux(0x05027058, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "P19")
        {
            if (func == "GPIOP19") {
                set_pinmux(0x030010D4, 3);
                set_pinmux(0x0502705c, 0x44);
            } else if (func == "UART3_TX") {
                set_pinmux(0x030010D4, 5);
                set_pinmux(0x0502705c, 0x44);
            } else if (func == "PWM5") {
                set_pinmux(0x030010D4, 7);
                set_pinmux(0x0502705c, 0x84);
            } else if (func == "SDIO1_D2") {
                set_pinmux(0x030010D4, 0);
                set_pinmux(0x0502705c, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "P20")
        {
            if (func == "GPIOP20") {
                set_pinmux(0x030010D8, 3);
                set_pinmux(0x05027060, 0x44);
            } else if (func == "UART3_RX") {
                set_pinmux(0x030010D8, 5);
                set_pinmux(0x05027060, 0x44);
            } else if (func == "PWM6") {
                set_pinmux(0x030010D8, 7);
                set_pinmux(0x05027060, 0x84);
            } else if (func == "SDIO1_D1") {
                set_pinmux(0x030010D8, 0);
                set_pinmux(0x05027060, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "P21")
        {
            if (func == "GPIOP21") {
                set_pinmux(0x030010DC, 3);
                set_pinmux(0x05027064, 0x44);
            } else if (func == "UART3_RTS") {
                set_pinmux(0x030010DC, 5);
                set_pinmux(0x05027064, 0x44);
            } else if (func == "I2C1_SDA") {
                set_pinmux(0x030010DC, 2);
                set_pinmux(0x05027064, 0x44);
            } else if (func == "PWM7") {
                set_pinmux(0x030010DC, 7);
                set_pinmux(0x05027064, 0x84);
            } else if (func == "SPI2_MISO") {
                set_pinmux(0x030010DC, 1);
                set_pinmux(0x05027064, 0x44);
            } else if (func == "SDIO1_D0") {
                set_pinmux(0x030010DC, 0);
                set_pinmux(0x05027064, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "P22")
        {
            if (func == "GPIOP22") {
                set_pinmux(0x030010E0, 3);
                set_pinmux(0x05027068, 0x44);
            } else if (func == "I2C3_SCL") {
                set_pinmux(0x030010E0, 2);
                set_pinmux(0x05027068, 0x44);
            } else if (func == "PWM8") {
                set_pinmux(0x030010E0, 7);
                set_pinmux(0x05027068, 0x84);
            } else if (func == "SPI2_MOSI") {
                set_pinmux(0x030010E0, 1);
                set_pinmux(0x05027068, 0x44);
            } else if (func == "SDIO1_CMD") {
                set_pinmux(0x030010E0, 0);
                set_pinmux(0x05027068, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "P23")
        {
            if (func == "GPIOP23") {
                set_pinmux(0x030010E4, 3);
                set_pinmux(0x0502706c, 0x44);
            } else if (func == "I2C3_SDA") {
                set_pinmux(0x030010E4, 2);
                set_pinmux(0x0502706c, 0x44);
            } else if (func == "PWM9") {
                set_pinmux(0x030010E4, 7);
                set_pinmux(0x0502706c, 0x44);
            } else if (func == "SPI2_SCK") {
                set_pinmux(0x030010E4, 1);
                set_pinmux(0x0502706c, 0x84);
            } else if (func == "SDIO1_CLK") {
                set_pinmux(0x030010E4, 0);
                set_pinmux(0x0502706c, 0x44);
            } else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else if (pin == "P24")
        {
            if (func == "GPIOP24") {
                set_pinmux(0x030011D0, 3);
                set_pinmux(0x050270E0, 0x44);
            } else if (func == "PWM2") {
                set_pinmux(0x030011D0, 4);
                set_pinmux(0x050270E0, 0x44);
            }else {
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }
        else
        {
            throw err::Exception(err::ERR_ARGS);
        }
        return err::ERR_NOT_IMPL;
    }
}
