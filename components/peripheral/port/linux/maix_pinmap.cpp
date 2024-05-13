#include "maix_pinmap.hpp"

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace maix::peripheral::pinmap
{

#if PLATFORM_MAIXCAM

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
            "P18",
            "P19",
            "P20",
            "P21",
            "P22",
            "P23",
        };
        return pins;
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
                "GPIOA15"};
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
                "SPI1_SCK",
            };
            return funcs;
        }
        else if (pin == "A23")
        {
            std::vector<std::string> funcs = {
                "GPIOA23",
                "SPI1_MISO",
            };
            return funcs;
        }
        else if (pin == "A24")
        {
            std::vector<std::string> funcs = {
                "GPIOA24",
                "SPI1_CS",
            };
            return funcs;
        }
        else if (pin == "A25")
        {
            std::vector<std::string> funcs = {
                "GPIOA25",
                "SPI1_MOSI",
            };
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
            };
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
        else if (pin == "P18")
        {
            // TODO:
            return std::vector<std::string>();
        }
        else if (pin == "P19")
        {
            // TODO:
            return std::vector<std::string>();
        }
        else if (pin == "P20")
        {
            // TODO:
            return std::vector<std::string>();
        }
        else if (pin == "P21")
        {
            // TODO:
            return std::vector<std::string>();
        }
        else if (pin == "P22")
        {
            // TODO:
            return std::vector<std::string>();
        }
        else if (pin == "P23")
        {
            // TODO:
            return std::vector<std::string>();
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
            return err::ERR_NONE;
        }
        else if (pin == "A16")
        {
            if (func == "GPIOA16")
                set_pinmux(0x03001040, 3);
            else if (func == "UART0_TX")
                set_pinmux(0x03001040, 0);
            else if (func == "PWM4")
                set_pinmux(0x03001040, 2);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A17")
        {
            if (func == "GPIOA17")
                set_pinmux(0x03001044, 3);
            else if (func == "UART0_RX")
                set_pinmux(0x03001044, 0);
            else if (func == "PWM5")
                set_pinmux(0x03001044, 2);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A18")
        {
            if (func == "GPIOA18")
                set_pinmux(0x03001068, 3);
            else if (func == "UART1_RX")
                set_pinmux(0x03001068, 6);
            else if (func == "PWM6")
                set_pinmux(0x03001068, 2);
            else if (func == "JTAG_TCK")
                set_pinmux(0x03001068, 0);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A19")
        {
            if (func == "GPIOA19")
                set_pinmux(0x03001064, 3);
            else if (func == "UART1_TX")
                set_pinmux(0x03001064, 6);
            else if (func == "PWM7")
                set_pinmux(0x03001064, 2);
            else if (func == "JTAG_TMS")
                set_pinmux(0x03001064, 0);
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A22")
        {
            if (func == "GPIOA22")
                set_pinmux(0x03001050, 3);
            else if (func == "SPI1_SCK")
                set_pinmux(0x03001050, 1); // TODO: nor nand?
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A23")
        {
            if (func == "GPIOA23")
                set_pinmux(0x0300105C, 3);
            else if (func == "SPI1_MISO")
                set_pinmux(0x0300105C, 1); // TODO: nor nand?
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A24")
        {
            if (func == "GPIOA24")
                set_pinmux(0x03001060, 3);
            else if (func == "SPI1_CS")
                set_pinmux(0x03001060, 1); // TODO: nor nand?
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A25")
        {
            if (func == "GPIOA25")
                set_pinmux(0x03001054, 3);
            else if (func == "SPI1_MOSI")
                set_pinmux(0x03001054, 1); // TODO: nor nand?
            else
                return err::ERR_ARGS;
            return err::ERR_NONE;
        }
        else if (pin == "A26")
        {
            return err::ERR_NONE;
        }
        else if (pin == "A27")
        {
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
        else if (pin == "P18")
        {
            // TODO:
            return err::ERR_NOT_IMPL;
        }
        else if (pin == "P19")
        {
            // TODO:
            return err::ERR_NOT_IMPL;
        }
        else if (pin == "P20")
        {
            // TODO:
            return err::ERR_NOT_IMPL;
        }
        else if (pin == "P21")
        {
            // TODO:
            return err::ERR_NOT_IMPL;
        }
        else if (pin == "P22")
        {
            // TODO:
            return err::ERR_NOT_IMPL;
        }
        else if (pin == "P23")
        {
            // TODO:
            return err::ERR_NOT_IMPL;
        }
        else
        {
            throw err::Exception(err::ERR_ARGS);
        }
        return err::ERR_NOT_IMPL;
    }

#else // #if PLATFORM_MAIXCAM

    std::vector<std::string> get_pins()
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> get_pin_functions(const std::string &pin)
    {
        return std::vector<std::string>();
    }

    err::Err set_pin_function(const std::string &pin, const std::string &func)
    {
        return err::ERR_NOT_IMPL;
    }

#endif // #if PLATFORM_MAIXCAM
}
