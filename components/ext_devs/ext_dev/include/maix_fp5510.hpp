#ifndef __MAIX_FP5510_H
#define __MAIX_FP5510_H

#include "stdint.h"

namespace maix::ext_dev::fp5510 {
/**
 * FP5510 class
 * @maixpy maix.ext_dev.fp5510.FP5510
*/
class FP5510 {
private:
    void *_param;
public:
    /**
     * @brief Construct a new FP5510 object
     *
     * @param id iic number, default is 4
     * @param slave_addr slave address of fp5510, default is 0x0c.
     * @param freq iic frequency, default is 400k
     *
     * @maixpy maix.ext_dev.fp5510.FP5510.__init__
     */
    FP5510(int id = 4, int slave_addr = 0x0c, int freq = 400000);
    ~FP5510();

    /**
     * @brief Set fp5510 position
     * @param pos the position of fp5510, range is [0, 1023]
     * @maixpy maix.ext_dev.fp5510.FP5510.set_pos
    */
    void set_pos(uint32_t pos);

    /**
     * @brief Get fp5510 position
     * @return returns the position of fp5510, range is [0, 1023]
     * @maixpy maix.ext_dev.fp5510.FP5510.get_pos
    */
    uint32_t get_pos();
};
}
#endif // __MAIX_FP5510_H

