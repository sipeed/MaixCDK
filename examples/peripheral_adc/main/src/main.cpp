
#include "maix_basic.hpp"
#include "main.h"
#include "maix_adc.hpp"

using namespace maix;
using namespace maix::peripheral::adc;

int _main(int argc, char* argv[])
{
    log::info("Program start");


    /**
     * 
     * The vref is usually 4.6~5.0V, and the typical value is 4.8V
     * 
     * MaixCAM ADC:
     * 1. Controller operating frequency 12.5MHz
     * 2. The scanning frequency cannot be higher than 320K/s
     * 3. 12bit sampling accuracy
     * 4. 1.5V Vref (https://doc.sophgo.com/cvitek-develop-docs/master/docs_latest_release/CV180x_CV181x/zh/01.software/BSP/Peripheral_Driver/build/html/11_ADC_Operation_Guide.html)
     * 
     * ADC_PIN ------[R1 10KΩ]-----------[R2 5.1KΩ]-------- GND
     *  (Vin)                       |
     *                              |----- ADC1
     *                                     (Vadc)
     * According to the test, the Vref is not necessarily 1.5V
     * Vref = (Vadc/adc_data)*(2^12)
     * 
     * 
     * In general, there is an impedance inside the ADC peripheral, 
     * which can actually be seen as the following circuit diagram
     * 
     * ADC_PIN ------[R1 10KΩ]-----------[R2 5.1KΩ]----------- GND
     *  (Vin)                       |                   |
     *                              |                   |
     *                              |----[Radc ?KΩ]-----|
     *                              |
     *                              |
     *                              |----- ADC1 (Vadc)       
     * and
     * 
     * ADC_PIN ------[R1 10KΩ]-----------[R3 ?KΩ]-------- GND
     *  (Vin)                       |
     *                              |----- ADC1
     *                                     (Vadc)
     * 
     * Radc = Vadc/((Vin-Vadc)/R1-Vadc/R2)
     * R3 = Vadc*R1/(Vin-Vadc)
     * 
     * ADC_PIN Vin_max = (Vref/R3)*(R1+R3)
     * 
     * 
     * eg:
     *      Vin = 3.3V      Vadc = 1.06V    adc_data = 2700
     *      R1 = 10KΩ       R2 = 5.1KΩ
     * then we can know:
     *      Vref = 1.6081V  R3 = 4.7321KΩ   Vin_max = 5.0062V
    */

    ADC adc(0, RES_BIT_12);

    // Run until app want to exit, for example app::switch_app API will set exit flag.
    // And you can also call app::set_exit_flag(true) to mark exit.
    while(!app::need_exit())
    {
        int data = adc.read();
        log::info("Get ADC data: %d", data);

        maix::time::sleep_ms(50);

        float vol = adc.read_vol();
        log::info("Read ADC vol: %f", vol);

        maix::time::sleep_ms(50);
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


