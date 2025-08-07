/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2025.8.6: support maixcam2 pinmap by neucrack.
 */

#include "maix_pinmap.hpp"

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdint>

 namespace maix::peripheral::pinmap
 {

    struct PinRegInfo {
        uintptr_t reg_addr;   // 寄存器地址
        int bit_offset;       // 功能控制位偏移
        int bit_width;        // 功能位宽
    };

    struct PinInfo
    {
        std::vector<std::string> funcs;
        PinRegInfo reg;
        std::vector<int> reg_values;
    };


    static const std::map<std::string, PinInfo> pins_info = {
        {"IO0_A0",
            PinInfo {
                std::vector<std::string> {"GPIO0_A0", "I2C6_SDA", "SPI1_MOSI"},
                PinRegInfo               {0x0230000C, 16, 3},
                std::vector<int>         {6, 3, 2}
            }
        },
        {"IO0_A1",
            PinInfo {
                std::vector<std::string> {"GPIO0_A1", "I2C6_SCL", "SPI1_MISO"},
                PinRegInfo               {0x02300018, 16, 3},
                std::vector<int>         {6, 3, 2}
            }
        },
        {"IO0_A2",
            PinInfo {
                std::vector<std::string> {"GPIO0_A2", "SPI1_CS0"},
                PinRegInfo               {0x02300024, 16, 3},
                std::vector<int>         {6, 2}
            }
        },
        {"IO0_A4",
            PinInfo {
                std::vector<std::string> {"GPIO0_A4", "SPI1_SCK", "I2C5_SDA"},
                PinRegInfo               {0x0230003C, 16, 3},
                std::vector<int>         {6, 2, 3}
            }
        },
        {"IO0_A6",
            PinInfo {
                std::vector<std::string> {"GPIO0_A6"}, // state_led
                PinRegInfo               {0x02300054, 16, 3},
                std::vector<int>         {6}
            }
        },
        {"IO0_A8",
            PinInfo {
                std::vector<std::string> {"GPIO0_A8", "I2C7_SCL"},
                PinRegInfo               {0x0230006C, 16, 3},
                std::vector<int>         {6, 3}
            }
        },
        {"IO0_A9",
            PinInfo {
                std::vector<std::string> {"GPIO0_A9", "I2C7_SDA"},
                PinRegInfo               {0x02300078, 16, 3},
                std::vector<int>         {6, 3}
            }
        },
        {"IO0_A21",
            PinInfo {
                std::vector<std::string> {"GPIO0_A21", "UART4_TX"},
                PinRegInfo               {0x02302090, 16, 3},
                std::vector<int>         {6, 1}
            }
        },
        {"IO0_A22",
            PinInfo {
                std::vector<std::string> {"GPIO0_A22", "UART4_RX"},
                PinRegInfo               {0x0230209C, 16, 3},
                std::vector<int>         {6, 1}
            }
        },
        {"IO0_A28",
            PinInfo {
                std::vector<std::string> {"GPIO0_A28", "UART0_TX"},  // system log output
                PinRegInfo               {0x0230403C, 16, 3},
                std::vector<int>         {6, 0}
            }
        },
        {"IO0_A29",
            PinInfo {
                std::vector<std::string> {"GPIO0_A29", "UART0_RX"},  // system log output
                PinRegInfo               {0x02304048, 16, 3},
                std::vector<int>         {6, 0}
            }
        },
        {"IO0_A30",
            PinInfo {
                std::vector<std::string> {"GPIO0_A30", "PWM6", "UART1_TX", "SPI2_CS3"},
                PinRegInfo               {0x02304054, 16, 3},
                std::vector<int>         {6, 3, 0, 1}
            }
        },
        {"IO0_A31",
            PinInfo {
                std::vector<std::string> {"GPIO0_A31", "PWM7", "UART1_RX"},
                PinRegInfo               {0x02304060, 16, 3},
                std::vector<int>         {6, 3, 0}
            }
        },
        {"IO1_A0",
            PinInfo {
                std::vector<std::string> {"GPIO1_A0", "UART2_TX"},
                PinRegInfo               {0x0230406C, 16, 3},
                std::vector<int>         {6, 0}
            }
        },
        {"IO1_A1",
            PinInfo {
                std::vector<std::string> {"GPIO1_A1", "UART2_RX"},
                PinRegInfo               {0x02304078, 16, 3},
                std::vector<int>         {6, 0}
            }
        },
        {"IO1_A2",
            PinInfo {
                std::vector<std::string> {"GPIO1_A2", "PWM4", "UART3_TX", "SPI2_MOSI"},
                PinRegInfo               {0x02304084, 16, 3},
                std::vector<int>         {6, 3, 0, 1}
            }
        },
        {"IO1_A3",
            PinInfo {
                std::vector<std::string> {"GPIO1_A3", "PWM5", "UART3_RX", "SPI2_MISO"},
                PinRegInfo               {0x02304090, 16, 3},
                std::vector<int>         {6, 3, 3}
            }
        },
        {"IO1_A18",
            PinInfo {
                std::vector<std::string> {"GPIO1_A18", "SPI2_MOSI"},
                PinRegInfo               {0x104F00CC, 16, 3},
                std::vector<int>         {6, 4}
            }
        },
        {"IO1_A19",
            PinInfo {
                std::vector<std::string> {"GPIO1_A19", "SPI2_MISO"},
                PinRegInfo               {0x104F00D8, 16, 3},
                std::vector<int>         {6, 4}
            }
        },
        {"IO1_A20",
            PinInfo {
                std::vector<std::string> {"GPIO1_A20", "SPI2_SCK"},
                PinRegInfo               {0x104F00E4, 16, 3},
                std::vector<int>         {6, 4}
            }
        },
        {"IO1_A21",
            PinInfo {
                std::vector<std::string> {"GPIO1_A21", "SPI2_CS1"},
                PinRegInfo               {0x104F00F0, 16, 3},
                std::vector<int>         {6, 4}
            }
        },
        {"IO1_A22",
            PinInfo {
                std::vector<std::string> {"GPIO1_A22", "PWM3"},
                PinRegInfo               {0x104F00FC, 16, 3},
                std::vector<int>         {6, 2}
            }
        },
        {"IO1_A24",
            PinInfo {
                std::vector<std::string> {"GPIO1_A24", "PWM5"},
                PinRegInfo               {0x104F003C, 16, 3},
                std::vector<int>         {6, 2}
            }
        },
        {"IO1_A25",
            PinInfo {
                std::vector<std::string> {"GPIO1_A25", "PWM6"}, // LED light
                PinRegInfo               {0x104F0048, 16, 3},
                std::vector<int>         {6, 2}
            }
        },
        {"IO3_A2",
            PinInfo {
                std::vector<std::string> {"GPIO3_A2"}, // boot/user key
                PinRegInfo               {0x02302024, 16, 3},
                std::vector<int>         {0}
            }
        }
    };




// 假设我们的系统页大小为4KB
#define PAGE_SIZE 4096
#define PAGE_MASK (PAGE_SIZE - 1)
    err::Err read_register(uintptr_t addr, uint32_t &value)
    {
        int fd = open("/dev/mem", O_RDONLY | O_SYNC);
        if (fd == -1)
        {
            log::error("Error opening /dev/mem for read");
            return err::ERR_IO;
        }

        void *map_base = mmap(0, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, addr & ~PAGE_MASK);
        if (map_base == MAP_FAILED)
        {
            log::error("Error mapping memory for read");
            close(fd);
            return err::ERR_IO;
        }

        void *virt_addr = (char *)map_base + (addr & PAGE_MASK);
        value = *((volatile uint32_t *)virt_addr);

        if (munmap(map_base, PAGE_SIZE) == -1)
        {
            log::warn("Error unmapping memory (read)");
        }

        close(fd);
        return err::ERR_NONE;
    }

    /**
    * @brief 读改写某个寄存器的特定位段
    *
    * @param addr       寄存器地址
    * @param bit_offset 位偏移
    * @param bit_width  位宽
    * @param value      要写入的值（不应超过 bit_width 能表示的范围）
    */
    err::Err update_register_bits(uintptr_t addr, int bit_offset, int bit_width, uint32_t value)
    {
        if (bit_width <= 0 || bit_width >= 32) {
            throw err::Exception(err::ERR_ARGS, "update_register_bits: Invalid bit width");
        }

        if (value >= (1U << bit_width)) {
            throw err::Exception(err::ERR_ARGS, "update_register_bits: value too large for target bit width");
        }

        int fd;
        void *map_base, *virt_addr;

        /* 打开 /dev/mem 文件 */
        if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        {
            log::error("Error opening /dev/mem");
            return err::ERR_IO;
        }

        /* 映射需要访问的物理内存页到进程空间 */
        map_base = mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr & ~PAGE_MASK);
        if (map_base == (void *)-1)
        {
            log::error("Error mapping memory");
            close(fd);
            return err::ERR_IO;
        }

        /* 计算目标寄存器的虚拟地址 */
        virt_addr = (char *)map_base + (addr & PAGE_MASK);

        /* 读取值 */
        uint32_t reg_value = *((volatile uint32_t *)virt_addr);

        uint32_t mask = ((1U << bit_width) - 1) << bit_offset;

        reg_value &= ~mask;                      // 清除目标位段
        reg_value |= (value << bit_offset) & mask;  // 设置新值

        /* 写入值到目标寄存器 */
        *((volatile uint32_t *)virt_addr) = reg_value;

        /* 取消映射并关闭文件描述符 */
        if (munmap(map_base, PAGE_SIZE) == -1)
        {
            log::warn("Error unmapping memory");
        }

        close(fd);
        return err::ERR_NONE;
    }

    static std::string uintptr_to_hex_string(uintptr_t addr) {
        std::stringstream ss;
        ss << "0x" << std::hex << std::setw(sizeof(uintptr_t) * 2)
           << std::setfill('0') << addr;
        return ss.str();
    }

    /**
     * pin already valid by caller.
     */
     static std::string _get_pin_function(const std::string &pin)
     {
        const PinInfo &info = pins_info.at(pin);

        // 1. 读取寄存器
        uint32_t reg_val;
        auto e = read_register(info.reg.reg_addr, reg_val);
        if(e != err::ERR_NONE)
        {
            throw err::Exception(e);
        }

        // 2. 提取功能位
        uint32_t value = (reg_val >> info.reg.bit_offset) & ((1 << info.reg.bit_width) - 1);

        // 3. 在 reg_values 中查找 value 所在的索引
        auto it_val = std::find(info.reg_values.begin(), info.reg_values.end(), value);
        if (it_val == info.reg_values.end()) {
            throw err::Exception(err::ERR_NOT_IMPL, "Register " + uintptr_to_hex_string(info.reg.reg_addr) + " read unknown value: " + std::to_string(value));
        }

        size_t index = std::distance(info.reg_values.begin(), it_val);

        if (index >= info.funcs.size()) {
            throw err::Exception(err::ERR_NOT_IMPL, "Pin " + pin + " reg value length != function names, please contact maintainer");
        }

        // 4. 返回当前功能名
        return info.funcs[index];
     }

     /**
     * pin and func already valid by caller.
     */
     err::Err _set_pin_function(const std::string &pin, const std::string &func)
     {
        const PinInfo &info = pins_info.at(pin);

        // 1. 找到 func 对应的下标
        auto it_func = std::find(info.funcs.begin(), info.funcs.end(), func);
        if (it_func == info.funcs.end()) {
            return err::ERR_NOT_IMPL; // 理论上不会发生，因为调用者已验证
        }

        size_t index = std::distance(info.funcs.begin(), it_func);

        if (index >= info.reg_values.size()) {
            throw err::Exception(err::ERR_NOT_IMPL, "Pin " + pin + " reg value length != function names, please contact maintainer");
        }

        int reg_value = info.reg_values[index];

        // 2. 更新寄存器
        update_register_bits(info.reg.reg_addr, info.reg.bit_offset, info.reg.bit_width, reg_value);
        return err::ERR_NONE;
     }

     std::vector<std::string> get_pins()
     {
         std::vector<std::string> keys;
         for (const auto& pair : pins_info) {
             keys.push_back(pair.first);
         }
         return keys;
     }

     std::vector<std::string> get_pin_functions(const std::string &pin)
     {
         for (const auto& pair : pins_info) {
             if (pair.first == pin)
                 return pair.second.funcs;
         }
         log::error("pin %s not valid", pin.c_str());
         throw err::Exception(err::ERR_ARGS);
     }

     std::string get_pin_function(const std::string &pin)
     {
         // check pin
         auto it = pins_info.find(pin);
         if (it == pins_info.end()) {
             log::error("pin %s not valid", pin.c_str());
             throw err::Exception(err::ERR_ARGS);
         }
         return _get_pin_function(pin);
     }

     err::Err set_pin_function(const std::string &pin, const std::string &func)
     {
         // check pin
         auto it = pins_info.find(pin);
         if (it == pins_info.end()) {
             log::error("pin %s not valid", pin.c_str());
             return err::ERR_ARGS;
         }

         // check func
         const auto &valid_funcs = it->second.funcs;
         if(std::find(valid_funcs.begin(), valid_funcs.end(), func) == valid_funcs.end())
         {
             log::error("func %s for pin %s not valid", func.c_str(), pin.c_str());
             return err::ERR_ARGS;
         }

         return _set_pin_function(pin, func);
    }
 }
