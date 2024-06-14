/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

namespace maix::peripheral::adc
{
    /**
     * @brief 8-bit resolution, supported by the actual hardware
     * @maixpy maix.peripheral.adc.RES_BIT_8
     */
    const int RES_BIT_8 = 8;

    /**
     * @brief 10-bit resolution, supported by the actual hardware
     * @maixpy maix.peripheral.adc.RES_BIT_10
     */
    const int RES_BIT_10 = 10;

    /**
     * @brief 12-bit resolution, supported by the actual hardware
     * @maixpy maix.peripheral.adc.RES_BIT_12
     */
    const int RES_BIT_12 = 12;

    /**
     * @brief 16-bit resolution, supported by the actual hardware
     * @maixpy maix.peripheral.adc.RES_BIT_16
     */
    const int RES_BIT_16 = 16;

    /**
     * Peripheral adc class
     * @maixpy maix.peripheral.adc.ADC
     */
    class ADC
    {
    public:
        /**
         * @brief ADC constructor
         * @param[in] pin adc pin, int type
         * @param[in] resolution adc resolution. default is -1, means use default resolution
         * option:
         * resolution = adc.RES_BIT_8, means 8-bit resolution
         * resolution = adc.RES_BIT_10, means 10-bit resolution
         * resolution = adc.RES_BIT_12, means 12-bit resolution
         * resolution = adc.RES_BIT_16, means 16-bit resolution
         * the default resolution is determined by actual hardware.
         * @param[in] vref adc refer voltage. default is -1, means use default refer voltage.
         * the default vref is determined by actual hardware. range: [0.0, 10.0]
         * @maixpy maix.peripheral.adc.ADC.__init__
         */
        ADC(int pin, int resolution, float vref = -1);
        ~ADC();

        /**
         * @brief read adc value
         * @return adc data, int type
         * if resolution is 8-bit, return value range is [0, 255]
         * if resolution is 10-bit, return value range is [0, 1023]
         * if resolution is 12-bit, return value range is [0, 4095]
         * if resolution is 16-bit, return value range is [0, 65535]
         * @maixpy maix.peripheral.adc.ADC.read
         */
        int read();

        /**
         * @brief read adc voltage
         * @return adc voltage, float type。the range is [0.0, vref]
         * @maixpy maix.peripheral.adc.ADC.read_vol
         */
        float read_vol();

    private:
        int _fd;
        int _pin;
        int _resolution;
        float _vref;
    };
}; // namespace maix::peripheral::adc
