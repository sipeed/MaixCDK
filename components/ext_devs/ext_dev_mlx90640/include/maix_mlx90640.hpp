#ifndef __MAIX_MLX90640_HPP__
#define __MAIX_MLX90640_HPP__

#include <vector>
#include <memory>
#include <cstdint>

#include "maix_image.hpp"

#include "maix_cmap.hpp"

struct paramsMLX90640;

namespace maix::ext_dev::mlx90640 {

/**
 * @brief MLX90640 FPS
 * @maixpy maix.ext_dev.mlx90640.FPS
 */
enum class FPS : uint8_t {
    FPS_1   = 0b001,
    FPS_2   = 0b010,
    FPS_4   = 0b011,
    FPS_8   = 0b100,
    FPS_16  = 0b101,
    FPS_32  = 0b110,
    FPS_64  = 0b111,
};

/**
 * @brief MLX90640 Image Width.
 * @maixpy maix.ext_dev.mlx90640.MLX_W
 */
constexpr uint32_t MLX_W = 32;

/**
 * @brief MLX90640 Image Height.
 * @maixpy maix.ext_dev.mlx90640.MLX_H
 */
constexpr uint32_t MLX_H = 24;

template<typename T>
using Matrix = std::vector<std::vector<T>>;

#if CONFIG_BUILD_WITH_MAIXPY
/* for maixpy */
    #define KMatrix ::maix::ext_dev::mlx90640::Matrix<uint16_t>
    #define CMatrix ::maix::ext_dev::mlx90640::Matrix<float>
    #define Point   std::tuple<int, int, float>
#else
/* for maixcdk */
    using KMatrix   = Matrix<uint16_t>;
    using CMatrix   = Matrix<float>;
    using Point     = std::tuple<int, int, float>;
#endif
/**
 * @brief CMatrix to KMatrix.
 *
 * @param matrix CMatrix type.
 * @return KMatrix
 *
 * @maixpy maix.ext_dev.mlx90640.to_kmatrix
 */
KMatrix to_kmatrix(const CMatrix& matrix);

/**
 * @brief KMatrix to CMatrix
 *
 * @param matrix KMatrix type.
 * @return CMatrix
 *
 * @maixpy maix.ext_dev.mlx90640.to_cmatrix
 */
CMatrix to_cmatrix(const KMatrix& matrix);

/**
 * @brief MLX90640 (℃)
 * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius
 */
class MLX90640Celsius final {
public:
    /**
     * @brief Construct a new MLX90640Celsius object
     *
     * This constructor initializes an MLX90640Celsius object with the specified parameters
     * to configure the I2C bus communication, frame rate, color mapping, temperature ranges,
     * and emissivity for the MLX90640 thermal camera.
     *
     * @param i2c_bus_num The I2C bus number to which the MLX90640 is connected.
     * @param fps The preferred frame rate for the MLX90640, default is FPS::FPS_32.
     * @param cmap The color mapping to be used for generating the pseudo color image, default is Cmap::WHITE_HOT.
     * @param temp_min The minimum reference temperature (in °C) for generating the pseudo color image. Default is -1.
     * @param temp_max The maximum reference temperature (in °C) for generating the pseudo color image. Default is -1.
     *                 If both max and min are equal, it operates in auto mode:
     *                 the maximum temperature in the frame is taken as the maximum reference temperature,
     *                 and the minimum temperature in the frame is taken as the minimum reference temperature.
     * @param emissivity The emissivity parameter for the MLX90640, default is 0.95.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.__init__
     */
    MLX90640Celsius(int i2c_bus_num,
                    ::maix::ext_dev::mlx90640::FPS fps=::maix::ext_dev::mlx90640::FPS::FPS_32,
                    ::maix::ext_dev::cmap::Cmap cmap=::maix::ext_dev::cmap::Cmap::WHITE_HOT,
                    float temp_min=-1, float temp_max=-1, float emissivity=0.95);

    ~MLX90640Celsius();

    /**
     * @brief Retrieves sensor data and returns a temperature matrix of size MLX_H * MLX_W
     *
     *              MLX_W: 32
     *          ---------------
     *          |
     *   MLX_H  |
     *   : 24   |
     *
     * The matrix structure is represented as list[MLX_H][MLX_W],
     * where MLX_H is the number of rows (24) and MLX_W is the number of columns (32).
     *
     * @return CMatrix containing the temperature data, or an empty matrix ([]) if the operation fails.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.matrix
     */
    CMatrix matrix();

    /**
     * @brief Obtains sensor data and converts it into a pseudo-color image
     *
     * This function retrieves the thermal data from the sensor and processes it
     * to generate a pseudo-color representation of the temperature distribution.
     *
     * @return maix::image::Image* A raw pointer to a maix image object.
     *         It is the responsibility of the caller to free this memory
     *         in C/C++ to prevent memory leaks.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.image
     */
    ::maix::image::Image* image();

    /**
     * @brief Finds the pixel with the minimum temperature from the most recent reading
     *
     * This function identifies the pixel with the minimum temperature
     * from the latest data obtained from the sensor.
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the minimum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.min_temp_point
     */
    Point min_temp_point();

    /**
     * @brief Finds the pixel with the maximum temperature from the most recent reading
     *
     * This function identifies the pixel with the maximum temperature
     * from the latest data obtained from the sensor.
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the maximum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.max_temp_point
     */
    Point max_temp_point();

    /**
     * @brief Finds the center pixel from the most recent reading
     *
     * This function determines the center pixel of the temperature matrix
     * based on the most recent data obtained from the sensor.
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the center pixel in the temperature matrix.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.center_point
     */
    Point center_point();

    /**
     * @brief Converts a given matrix of temperature data into an image
     *
     * This function takes a temperature matrix and generates
     * a corresponding image representation based on the
     * configured color map and other parameters.
     *
     * @param matrix The temperature matrix to be converted.
     * @return maix::image::Image* A pointer to the generated image.
     *         It is the responsibility of the caller to free this memory
     *         in C/C++ to prevent memory leaks.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.image_from
     */
    ::maix::image::Image* image_from(const CMatrix& matrix);

    /**
     * @brief Finds the pixel with the maximum temperature from the given matrix
     *
     * This static function identifies the pixel with the maximum temperature
     * from the specified temperature matrix.
     *
     * @param matrix The temperature matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the maximum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.max_temp_point_from
     */
    static Point max_temp_point_from(const CMatrix& matrix);

    /**
     * @brief Finds the pixel with the minimum temperature from the given matrix
     *
     * This static function identifies the pixel with the minimum temperature
     * from the specified temperature matrix.
     *
     * @param matrix The temperature matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the minimum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.min_temp_point_from
     */
    static Point min_temp_point_from(const CMatrix& matrix);

    /**
     * @brief Finds the center pixel from the given matrix
     *
     * This static function determines the center pixel of the
     * specified temperature matrix based on its dimensions.
     *
     * @param matrix The temperature matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the center pixel in the matrix.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Celsius.center_point_from
     */
    static Point center_point_from(const CMatrix& matrix);

private:
    maix::ext_dev::cmap::Cmap _cmap;
    float _min;
    float _max;
    float _emissivity;
    uint16_t _eeMLX90640[832];
    uint16_t _frame[834];
    float _mlx90640To[768];
    // paramsMLX90640 _mlx90640;
    std::unique_ptr<paramsMLX90640> _mlx90640;
    Point _temp_min;
    Point _temp_max;
    Point _center;
};

/**
 * @brief MLX90640 (K))
 * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin
 */
class MLX90640Kelvin final {
public:
    /**
     * @brief Construct a new MLX90640Kelvin object
     *
     * This constructor initializes an MLX90640Kelvin object with the specified parameters
     * to configure the I2C bus communication, frame rate, color mapping, temperature ranges,
     * and emissivity for the MLX90640 thermal camera.
     *
     * @param i2c_bus_num The I2C bus number to which the MLX90640 is connected.
     * @param fps The preferred frame rate for the MLX90640, default is FPS::FPS_32.
     * @param cmap The color mapping to be used for generating the pseudo color image, default is Cmap::WHITE_HOT.
     * @param temp_min The minimum reference temperature (in K) for generating the pseudo color image. Default is -1.
     * @param temp_max The maximum reference temperature (in K) for generating the pseudo color image. Default is -1.
     *                 If both max and min are equal, it operates in auto mode:
     *                 the maximum temperature in the frame is taken as the maximum reference temperature,
     *                 and the minimum temperature in the frame is taken as the minimum reference temperature.
     * @param emissivity The emissivity parameter for the MLX90640, default is 0.95.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.__init__
     */
    MLX90640Kelvin( int i2c_bus_num,
                    ::maix::ext_dev::mlx90640::FPS fps=::maix::ext_dev::mlx90640::FPS::FPS_32,
                    ::maix::ext_dev::cmap::Cmap cmap=::maix::ext_dev::cmap::Cmap::WHITE_HOT,
                    float temp_min=-1, float temp_max=-1,  float emissivity=0.95);

    /**
     * @brief Retrieves sensor data and returns a temperature matrix of size MLX_H * MLX_W
     *
     *              MLX_W: 32
     *          ---------------
     *          |
     *   MLX_H  |
     *   : 24   |
     *
     * The matrix structure is represented as list[MLX_H][MLX_W],
     * where MLX_H is the number of rows (24) and MLX_W is the number of columns (32).
     *
     * @return KMatrix containing the temperature data, or an empty matrix ([]) if the operation fails.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.matrix
     */
    KMatrix matrix();

    /**
     * @brief Obtains sensor data and converts it into a pseudo-color image
     *
     * This function retrieves the thermal data from the sensor and processes it
     * to generate a pseudo-color representation of the temperature distribution.
     *
     * @return maix::image::Image* A raw pointer to a maix image object.
     *         It is the responsibility of the caller to free this memory
     *         in C/C++ to prevent memory leaks.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.image
     */
    ::maix::image::Image* image();

    /**
     * @brief Finds the pixel with the minimum temperature from the most recent reading
     *
     * This function identifies the pixel with the minimum temperature
     * from the latest data obtained from the sensor.
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the minimum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.max_temp_point
     */
    Point max_temp_point();

    /**
     * @brief Finds the pixel with the maximum temperature from the most recent reading
     *
     * This function identifies the pixel with the maximum temperature
     * from the latest data obtained from the sensor.
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the maximum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.min_temp_point
     */
    Point min_temp_point();

    /**
     * @brief Finds the center pixel from the most recent reading
     *
     * This function determines the center pixel of the temperature matrix
     * based on the most recent data obtained from the sensor.
     *
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the center pixel in the temperature matrix.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.center_point
     */
    Point center_point();

    /**
     * @brief Converts a given matrix of temperature data into an image
     *
     * This function takes a temperature matrix and generates
     * a corresponding image representation based on the
     * configured color map and other parameters.
     *
     * @param matrix The temperature matrix to be converted.
     * @return maix::image::Image* A pointer to the generated image.
     *         It is the responsibility of the caller to free this memory
     *         in C/C++ to prevent memory leaks.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.image_from
     */
    ::maix::image::Image* image_from(const KMatrix& matrix);

    /**
     * @brief Finds the pixel with the maximum temperature from the given matrix
     *
     * This static function identifies the pixel with the maximum temperature
     * from the specified temperature matrix.
     *
     * @param matrix The temperature matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the maximum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.max_temp_point_from
     */
    static Point max_temp_point_from(const KMatrix& matrix);

    /**
     * @brief Finds the pixel with the minimum temperature from the given matrix
     *
     * This static function identifies the pixel with the minimum temperature
     * from the specified temperature matrix.
     *
     * @param matrix The temperature matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the pixel with the minimum temperature.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.min_temp_point_from
     */
    static Point min_temp_point_from(const KMatrix& matrix);

    /**
     * @brief Finds the center pixel from the given matrix
     *
     * This static function determines the center pixel of the
     * specified temperature matrix based on its dimensions.
     *
     * @param matrix The temperature matrix to be analyzed.
     * @return Point A tuple of type <int, int, float>, representing
     *         (x, y, temperature) of the center pixel in the matrix.
     *         If the operation fails, the return values will be x, y < 0.
     *
     * @maixpy maix.ext_dev.mlx90640.MLX90640Kelvin.center_point_from
     */
    static Point center_point_from(const KMatrix& matrix);

private:
    std::unique_ptr<MLX90640Celsius> _mlx{nullptr};
};

}   // namespace maix::ext_dev::mlx90640
#endif