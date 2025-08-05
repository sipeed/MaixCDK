
#include "maix_basic.hpp"
#include "maix_pmu.hpp"
#include "main.h"

using namespace maix;
using namespace ext_dev;

int _main(int argc, char* argv[])
{
    pmu::PMU pmu("axp2101");

    // Get battery percent
    log::info("Battery percent: %d%%", pmu.get_bat_percent());

    // Set the max battery charging current
    pmu.set_bat_charging_cur(1000);
    log::info("Max charging current: %dmA", pmu.get_bat_charging_cur());

    // Set DCDC1 voltage
    pmu.set_vol(pmu::PowerChannel::DCDC1, 3300);
    log::info("Set DCDC1 voltage: %dmV", pmu.get_vol(pmu::PowerChannel::DCDC1));

    // Get all channel voltage
    log::info("------ All channel voltage: ------");
    log::info("DCDC1: %d", pmu.get_vol(pmu::PowerChannel::DCDC1));
    log::info("DCDC2: %d", pmu.get_vol(pmu::PowerChannel::DCDC2));
    log::info("DCDC3: %d", pmu.get_vol(pmu::PowerChannel::DCDC3));
    log::info("DCDC4: %d", pmu.get_vol(pmu::PowerChannel::DCDC4));
    log::info("DCDC5: %d", pmu.get_vol(pmu::PowerChannel::DCDC5));
    log::info("ALDO1: %d", pmu.get_vol(pmu::PowerChannel::ALDO1));
    log::info("ALDO2: %d", pmu.get_vol(pmu::PowerChannel::ALDO2));
    log::info("ALDO3: %d", pmu.get_vol(pmu::PowerChannel::ALDO3));
    log::info("ALDO4: %d", pmu.get_vol(pmu::PowerChannel::ALDO4));
    log::info("BLDO1: %d", pmu.get_vol(pmu::PowerChannel::BLDO1));
    log::info("BLDO2: %d", pmu.get_vol(pmu::PowerChannel::BLDO2));
    log::info("----------------------------------");
    return 0;
}

int main(int argc, char* argv[])
{
    sys::register_default_signal_handle();
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
