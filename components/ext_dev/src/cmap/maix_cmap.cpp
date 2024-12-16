#include "maix_cmap.hpp"
#include <stdexcept>

/* cmap include */
#include "cmap_black_hot_yp0203.hpp"
#include "cmap_blackhotsd_yp0204.hpp"
#include "cmap_ironbow_yp0301.hpp"
#include "cmap_night_yp0901.hpp"
#include "cmap_red_hot_yp1303.hpp"
#include "cmap_redhotsd_yp1304.hpp"
#include "cmap_white_hot_yp0103.hpp"
#include "cmap_whitehotsd_yp0100.hpp"
#include "cmap_jet.hpp"

namespace maix::ext_dev::cmap {

const CmapArray* get(Cmap cmap)
{
    switch (cmap) {
    case Cmap::BLACK_HOT:
        return &black_hot_yp0203;
        break;
    case Cmap::BLACK_HOT_SD:
        return &blackhotsd_yp0204;
        break;
    case Cmap::WHITE_HOT:
        return &white_hot_yp0103;
        break;
    case Cmap::WHITE_HOT_SD:
        return &whitehotsd_yp0100;
        break;
    case Cmap::RED_HOT:
        return &red_hot_yp1303;
        break;
    case Cmap::RED_HOT_SD:
        return &redhotsd_yp1304;
        break;
    case Cmap::IRONBOW:
        return &ironbow_yp0301;
        break;
    case Cmap::NIGHT:
        return &night_yp0901;
        break;
    case Cmap::JET:
        return &jet;
        break;
    default:
        throw std::runtime_error("Unknown cmap!");
        break;
    }
}


}