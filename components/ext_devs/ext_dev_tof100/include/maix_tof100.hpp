#ifndef __MAIX_Tof100_HPP__
#define __MAIX_Tof100_HPP__

#include <vector>
#include <cstdint>
#include <memory>

#include "maix_cmap.hpp"
#include "maix_image.hpp"

namespace maix::ext_dev::tof100 {

#if CONFIG_BUILD_WITH_MAIXPY
    #define TOFMatrix std::vector<std::vector<uint32_t>>
    #define TOFPoint  std::tuple<int, int, uint32_t>
#else
    using TOFMatrix = std::vector<std::vector<uint32_t>>;
    using TOFPoint  = std::tuple<int, int, uint32_t>;
#endif

/**
 * @brief Tof100 Resolution
 * @maixpy maix.ext_dev.tof100.Resolution
 */
enum class Resolution : uint32_t {
    RES_100x100 = 100,
    RES_50x50 = 50,
    RES_25x25 = 25,
};

/**
 * @brief Tof100 TOF
 * @maixpy maix.ext_dev.tof100.Tof100
 */
class Tof100 final {
public:

    /**
     * @brief Construct a new Tof100 object
     *
     * @param spi_bus_num SPI bus number.
     * @param resolution @see Resolution
     * @param cmap The color mapping to be used for generating the pseudo color image
     * @param dis_min The minimum reference distance (in mm) for generating the pseudo color image. Default is -1.
     * @param dis_max The maximum reference distance (in mm) for generating the pseudo color image. Default is -1.
     *                 If both max and min are equal, it operates in auto mode:
     *                 the maximum distance in the frame is taken as the maximum reference distance,
     *                 and the minimum distance in the frame is taken as the minimum reference distance.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.__init__
     */
    Tof100(int spi_bus_num,
            ::maix::ext_dev::tof100::Resolution resolution=::maix::ext_dev::tof100::Resolution::RES_50x50,
            ::maix::ext_dev::cmap::Cmap cmap=::maix::ext_dev::cmap::Cmap::JET,
            int dis_min=-1, int dis_max=-1);

    /**
     * @brief Retrieves sensor data and returns a distance matrix.
     *
     * @return Matrix containing the distance data, or an empty matrix ([]) if the operation fails.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.matrix
     */
    TOFMatrix matrix();

    /**
     * @brief Obtains sensor data and converts it into a pseudo-color image
     *
     * @return ::maix::image::Image* A raw pointer to a maix image object.
     *         It is the responsibility of the caller to free this memory
     *         in C/C++ to prevent memory leaks.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.image
     */
    ::maix::image::Image* image();

    /**
     * @brief Finds the pixel with the maximum distance from the most recent reading
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, distance) of the pixel with the maximum distance.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.max_dis_point
     */
    TOFPoint max_dis_point();

    /**
     * @brief Finds the pixel with the minimum distance from the most recent reading
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, distance) of the pixel with the minimum distance.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.min_dis_point
     */
    TOFPoint min_dis_point();

    /**
     * @brief Finds the center pixel from the most recent reading
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, distance) of the center pixel in the distance matrix.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.center_point
     */
    TOFPoint center_point();

    /**
     * @brief Converts a given matrix of distance data into an image
     *
     * @param matrix The distance matrix to be converted.
     * @return ::maix::image::Image* A pointer to the generated image.
     *         It is the responsibility of the caller to free this memory
     *         in C/C++ to prevent memory leaks.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.image_from
     */
    ::maix::image::Image* image_from(const TOFMatrix& matrix);

    /**
     * @brief Finds the pixel with the maximum distance from the given matrix
     *
     * @param matrix The distance matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, distance) of the pixel with the maximum distance.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.max_dis_point_from
     */
    static TOFPoint max_dis_point_from(const TOFMatrix& matrix);

    /**
     * @brief Finds the pixel with the minimum distance from the given matrix
     *
     * @param matrix The distance matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, distance) of the pixel with the minimum distance.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.min_dis_point_from
     */
    static TOFPoint min_dis_point_from(const TOFMatrix& matrix);

    /**
     * @brief Finds the center pixel from the given matrix
     *
     * @param matrix The distance matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, distance) of the center pixel in the matrix.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.tof100.Tof100.center_point_from
     */
    static TOFPoint center_point_from(const TOFMatrix& matrix);

private:
    maix::ext_dev::cmap::Cmap _cmap;
    int _min;
    int _max;
    uint32_t _wh;
    int _fps_limit;
    TOFPoint _dis_min;
    TOFPoint _dis_max;
    TOFPoint _dis_center;
    uint32_t _mode;
    uint32_t _data_size;
    std::unique_ptr<uint8_t[]> _frame_buffer;
    uint8_t _quantization_step{0};
};


}




#endif // __MAIX_Tof100_HPP__