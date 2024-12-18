#ifndef __CMAP_UTILS_HPP__
#define __CMAP_UTILS_HPP__

#include "maix_cmap.hpp"

namespace maix::ext_dev::cmap {

constexpr uint8_t as_u8(int d)
{
    return static_cast<uint8_t>(d);
}

constexpr RGB rgb_tuple_from(int r, int g, int b)
{
    return {as_u8(r), as_u8(g), as_u8(b)};
}

}


#endif // __CMAP_UTILS_HPP__