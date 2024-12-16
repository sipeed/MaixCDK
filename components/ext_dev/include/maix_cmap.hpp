#ifndef __MAIX_CMAP_HPP__
#define __MAIX_CMAP_HPP__

#include <cstdint>
#include <vector>
#include <array>

namespace maix::ext_dev::cmap {


using RGB = std::array<uint8_t, 3>;
using CmapArray = std::vector<RGB>;

/**
 * @brief Cmap
 * @maixpy maix.ext_dev.cmap.Cmap
 */
enum class Cmap {
    WHITE_HOT = 0,
    BLACK_HOT,
    IRONBOW,
    NIGHT,
    RED_HOT,
    WHITE_HOT_SD,
    BLACK_HOT_SD,
    RED_HOT_SD,
    JET,
};

/**
 * @brief Get cmap ptr
 *
 * @param cmap @see Cmap
 * @return const CmapArray*
 *
 * @maixcdk maix.ext_dev.cmap.get
 */
const CmapArray* get(Cmap cmap);


}


#endif