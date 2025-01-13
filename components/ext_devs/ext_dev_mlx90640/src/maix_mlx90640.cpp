#include "maix_mlx90640.hpp"
#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include "maix_basic.hpp"

namespace maix::ext_dev::mlx90640 {

using maix::ext_dev::cmap::Cmap;
using maix::ext_dev::cmap::get;

constexpr Point empty_point = {-1,-1,0.0};
constexpr uint8_t MLX_ADDR = 0x33;
constexpr float KC = 273.15;

static const char* TAG()
{
    return "[MAIX MLX90640]";
}

template<typename T>
static bool check_matrix(const Matrix<T>& matrix)
{
    if (matrix.size() != MLX_H)
        return false;
    if (matrix.at(0).size() != MLX_W)
        return false;
    return true;
}

template<typename Closure>
static void for_each_in_matrix(Closure closure)
{
    for (int y = 0; y < static_cast<int>(MLX_H); ++y) {
        for (int x = 0; x < static_cast<int>(MLX_W); ++x) {
            closure(x, y);
        }
    }
}

static inline Point c2k(const Point& c)
{
    auto [x, y, t] = c;
    return std::make_tuple(x, y, t+KC);
}

KMatrix to_kmatrix(const CMatrix& matrix)
{
    if (!check_matrix(matrix)) {
        log::info("%s matrix <format != 24x32>!", TAG());
        return {};
    }

    KMatrix kmatrix(24, std::vector<uint16_t>(32));

    for_each_in_matrix([&](int x, int y){
        float celsius = matrix[y][x];
        float kelvin = (celsius+KC);
        kmatrix[y][x] = static_cast<uint16_t>(kelvin*100);
    });

    return kmatrix;
}

CMatrix to_cmatrix(const KMatrix& matrix)
{
    if (!check_matrix(matrix)) {
        log::info("%s matrix <format != 24x32>!", TAG());
        return {};
    }

    CMatrix cmatrix(24, std::vector<float>(32));

    for_each_in_matrix([&](int x, int y){
        float kelvin = static_cast<float>(matrix[y][x])/100;
        float celsius = kelvin-KC;
        cmatrix[y][x] = celsius;
    });

    return cmatrix;
}

void make_maix_image_pixel(float min, float max, float value, uint8_t* buffer, const cmap::CmapArray* array)
{
    float vrange = max - min;
    size_t index = 0;
    if (std::isnan(value)) {
        index = 0;
    } else if (std::isinf(value)) {
        index = array->size()-1;
    } else {
        value -= min;
        value /= vrange;
        if (value <= 0)
            index = 0;
        else if (value >= 1)
            index = array->size()-1;
        else {
            value *= (array->size()-1);
            index = ::floor(value);
        }
    }

    auto ir = (*array)[index][0];
    auto ig = (*array)[index][1];
    auto ib = (*array)[index][2];

    buffer[0] = ir;
    buffer[1] = ig;
    buffer[2] = ib;
}


MLX90640Kelvin::MLX90640Kelvin(int i2c_bus_num, FPS fps, Cmap cmap, float temp_min, float temp_max, float emissivity)
{
    float ctemp_min = temp_min - KC;
    float ctemp_max = temp_max - KC;

    this->_mlx = std::make_unique<MLX90640Celsius>(i2c_bus_num, fps, cmap, ctemp_min, ctemp_max, emissivity);
}



KMatrix MLX90640Kelvin::matrix()
{
    return to_kmatrix(this->_mlx->matrix());
}

maix::image::Image* MLX90640Kelvin::image()
{
    return this->_mlx->image();
}

Point MLX90640Kelvin::max_temp_point()
{
    return c2k(this->_mlx->max_temp_point());
}

Point MLX90640Kelvin::min_temp_point()
{
    return c2k(this->_mlx->min_temp_point());
}

Point MLX90640Kelvin::center_point()
{
    return c2k(this->_mlx->center_point());
}

maix::image::Image* MLX90640Kelvin::image_from(const KMatrix& matrix)
{
    return this->_mlx->image_from(to_cmatrix(matrix));
}

Point MLX90640Kelvin::max_temp_point_from(const KMatrix& matrix)
{
    return MLX90640Celsius::max_temp_point_from(to_cmatrix(matrix));
}

Point MLX90640Kelvin::min_temp_point_from(const KMatrix& matrix)
{
    return MLX90640Celsius::min_temp_point_from(to_cmatrix(matrix));
}

Point MLX90640Kelvin::center_point_from(const KMatrix& matrix)
{
    return MLX90640Celsius::center_point_from(to_cmatrix(matrix));
}

MLX90640Celsius::MLX90640Celsius(int i2c_bus_num, FPS fps, Cmap cmap, float temp_min, float temp_max,  float emissivity)
{
    this->_cmap = cmap;
    this->_max = temp_max;
    this->_min = temp_min;
    this->_emissivity = emissivity;
    this->_mlx90640 = std::make_unique<paramsMLX90640>();

    ::memset(this->_eeMLX90640, 0x00, std::size(this->_eeMLX90640)*sizeof(uint16_t));
    ::memset(this->_frame, 0x00, std::size(this->_frame)*sizeof(uint16_t));
    ::memset(this->_mlx90640To, 0x00, std::size(this->_mlx90640To)*sizeof(float));

    MLX90640_I2CInit(i2c_bus_num);
    MLX90640_SetResolution(MLX_ADDR, 0x03);
    MLX90640_SetRefreshRate(MLX_ADDR, static_cast<uint8_t>(fps));

    MLX90640_SetChessMode(MLX_ADDR);
    MLX90640_DumpEE(MLX_ADDR, this->_eeMLX90640);
    MLX90640_ExtractParameters(this->_eeMLX90640, this->_mlx90640.get());

}

MLX90640Celsius::~MLX90640Celsius() {}

CMatrix MLX90640Celsius::matrix()
{

    MLX90640_GetFrameData(MLX_ADDR, this->_frame);

    auto eTa = MLX90640_GetTa(this->_frame, this->_mlx90640.get());
    auto eTr = eTa-8.0;

    MLX90640_CalculateTo(this->_frame, this->_mlx90640.get(), this->_emissivity, eTr, this->_mlx90640To);

    CMatrix m(MLX_H, std::vector<float>(MLX_W));

    float temp_min = std::numeric_limits<float>::max();
    int temp_min_x{};
    int temp_min_y{};

    float temp_max = std::numeric_limits<float>::min();
    int temp_max_x{};
    int temp_max_y{};

    for_each_in_matrix([&](int x, int y){
        auto xx = MLX_W-1-x;
        auto yy = y;
        m[yy][xx] = this->_mlx90640To[(y*MLX_W)+x];
        if (m[yy][xx] < temp_min) {
            temp_min_x = xx;
            temp_min_y = yy;
            temp_min = m[yy][xx];
        }
        if (m[yy][xx] > temp_max) {
            temp_max_x = xx;
            temp_max_y = yy;
            temp_max = m[yy][xx];
        }
    });

    /* make points */
    this->_temp_min = std::make_tuple(temp_min_x, temp_min_y, temp_min);
    this->_temp_max = std::make_tuple(temp_max_x, temp_max_y, temp_max);
    this->_center   = std::make_tuple(MLX_W/2, MLX_H/2, m[MLX_H/2][MLX_W/2]);

    return m;
}

maix::image::Image* MLX90640Celsius::image()
{
    CMatrix m = this->matrix();
    return image_from(m);
}

Point MLX90640Celsius::max_temp_point()
{
    return this->_temp_max;
}

Point MLX90640Celsius::min_temp_point()
{
    return this->_temp_min;
}

Point MLX90640Celsius::center_point()
{
    return this->_center;
}

maix::image::Image* MLX90640Celsius::image_from(const CMatrix& matrix)
{
    const cmap::CmapArray* array = get(this->_cmap);

    // switch (this->_cmap) {
    // case Cmap::WHITE_HOT:
    //     array = &cmap::white_hot_yp0103;
    //     break;
    // case Cmap::WHITE_HOT_SD:
    //     array = &cmap::whitehotsd_yp0100;
    //     break;
    // case Cmap::BLACK_HOT:
    //     array = &cmap::black_hot_yp0203;
    //     break;
    // case Cmap::BLACK_HOT_SD:
    //     array = &cmap::blackhotsd_yp0204;
    //     break;
    // case Cmap::RED_HOT:
    //     array = &cmap::red_hot_yp1303;
    //     break;
    // case Cmap::RED_HOT_SD:
    //     array = &cmap::redhotsd_yp1304;
    //     break;
    // case Cmap::NIGHT:
    //     array = &cmap::night_yp0901;
    //     break;
    // case Cmap::IRONBOW:
    //     array = &cmap::ironbow_yp0301;
    //     break;
    // default:
    //     maix::log::error("%s Unknown CMAP!", TAG());
    //     return {};
    // }
    uint8_t img_buffer[MLX_H][MLX_W][3];

    auto tmin = this->_min;
    auto tmax = this->_max;
    if (tmin == tmax) {
        tmin = std::get<2>(this->_temp_min);
        tmax = std::get<2>(this->_temp_max);
    }

    for_each_in_matrix([&](int x, int y){
        make_maix_image_pixel(tmin, tmax, matrix[y][x], img_buffer[y][x], array);
    });

    return new maix::image::Image(MLX_W, MLX_H, image::FMT_RGB888, reinterpret_cast<uint8_t*>(img_buffer), MLX_W*MLX_H*3, true);
}

Point MLX90640Celsius::max_temp_point_from(const CMatrix& matrix)
{
    if (!check_matrix(matrix)) {
        log::error("%s matrix <format != 24x32> !", TAG());
        return empty_point;
    }

    float temp = std::numeric_limits<float>::min();
    int temp_x{-1};
    int temp_y{-1};

    for_each_in_matrix([&](int x, int y){
        if (matrix[y][x] > temp) {
            temp_x = x;
            temp_y = y;
            temp = matrix[y][x];
        }
    });

    return std::make_tuple(temp_x, temp_y, temp);
}

Point MLX90640Celsius::min_temp_point_from(const CMatrix& matrix)
{
    if (!check_matrix(matrix)) {
        log::error("%s matrix <format != 24x32> !", TAG());
        return empty_point;
    }

    float temp = std::numeric_limits<float>::max();
    int temp_x{-1};
    int temp_y{-1};

    for_each_in_matrix([&](int x, int y){
        if (matrix[y][x] < temp) {
            temp_x = x;
            temp_y = y;
            temp = matrix[y][x];
        }
    });

    return std::make_tuple(temp_x, temp_y, temp);
}

Point MLX90640Celsius::center_point_from(const CMatrix& matrix)
{
    if (!check_matrix(matrix)) {
        log::error("%s matrix <format != 24x32> !", TAG());
        return empty_point;
    }

    return std::make_tuple(MLX_W/2, MLX_H/2, matrix[MLX_H/2][MLX_W/2]);
}


}