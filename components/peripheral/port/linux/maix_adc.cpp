/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_adc.hpp"
#include "cstdio"

namespace maix::peripheral::adc{    
    ADC::ADC(int pin, int resolution, float vref)
    {
        printf("ADC::ADC(%d, %d, %f)\r\n", pin, resolution, vref);
    }

    ADC::~ADC()
    {
        printf("ADC::~ADC()\r\n");
    }

    int ADC::read()
    {
        printf("ADC::read()\r\n");
        static int val = 0;
        val += 5;
        return val;
    }

    float ADC::read_vol()
    {
        printf("ADC::read_vol()\r\n");
        static int val = 0;
        val += 5;
        return (float)val * 3.3 / 65536;
    }
}