#ifndef __CMAP_UTILS_HPP__
#define __CMAP_UTILS_HPP__

#include <cstdint>
#include <vector>

namespace maix::ext_dev::mlx90640::cmap {

// using RGBTuple = std::tuple<uint8_t, uint8_t, uint8_t>;
using RGBTuple = std::array<uint8_t, 3>;
using CmapArray = std::vector<RGBTuple>;

constexpr uint8_t as_u8(int d)
{
    return static_cast<uint8_t>(d);
}

constexpr RGBTuple rgb_tuple_from(int r, int g, int b)
{
    return {as_u8(r), as_u8(g), as_u8(b)};
}

}


#endif // __CMAP_UTILS_HPP__