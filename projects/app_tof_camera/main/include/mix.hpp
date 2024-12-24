#ifndef __AHDJKBJCBQAJHXCIIIG_MIX_HPP__
#define __AHDJKBJCBQAJHXCIIIG_MIX_HPP__

#include <cstdint>
#include <vector>
#include <limits>
#include "opencv2/opencv.hpp"
#include "maix_cmap.hpp"
#include "maix_image.hpp"

#if 0

std::vector<uint8_t> mix(const uint8_t* gray, const uint8_t* rgb, const uint32_t pixel_num, float gray_propor)
{
    std::vector<uint8_t> res(pixel_num*3);

    float gp = gray_propor + 0.3;
    gp = (gp > 0.99) ? 0.99 : gp;

    for (uint32_t i = 0; i < pixel_num; ++i) {
        // if (gray[i] == 255) {
        //     gray_propor = 0.65;
        //     res[i*3+0] = static_cast<uint8_t>(gray[i]*gray_propor + rgb[i*3+0]*(1-gray_propor));
        //     res[i*3+1] = static_cast<uint8_t>(gray[i]*gray_propor + rgb[i*3+1]*(1-gray_propor));
        //     res[i*3+2] = static_cast<uint8_t>(gray[i]*gray_propor + rgb[i*3+2]*(1-gray_propor));
        // } else {
        float gpropor = (gray[i] == 255) ? gp : gray_propor;
        res[i*3+0] = static_cast<uint8_t>(gray[i]*gpropor + rgb[i*3+0]*(1-gpropor));
        res[i*3+1] = static_cast<uint8_t>(gray[i]*gpropor + rgb[i*3+1]*(1-gpropor));
        res[i*3+2] = static_cast<uint8_t>(gray[i]*gpropor + rgb[i*3+2]*(1-gpropor));
        // }
    }

    return res;
}

#else

std::vector<uint8_t> mix(const uint8_t* gray, const uint8_t* rgb, const uint32_t pixel_num, float gray_propor)
{
    using namespace maix::ext_dev::cmap;
    using namespace maix::image;
    const CmapArray* cmaparray = get(Cmap::JET);
    float c = (static_cast<float>(cmaparray->size()) / 255);

    std::vector<uint8_t> thermal(pixel_num*3);
    for (uint32_t i = 0; i < pixel_num; ++i) {
            thermal[i*3+0] = std::get<0>((*cmaparray)[static_cast<size_t>(gray[i]*c)]);
            thermal[i*3+1] = std::get<1>((*cmaparray)[static_cast<size_t>(gray[i]*c)]);
            thermal[i*3+2] = std::get<2>((*cmaparray)[static_cast<size_t>(gray[i]*c)]);
    }

    cv::Mat thermal_image = cv::Mat(50, 50, CV_8UC3, thermal.data());
    cv::Mat visible_image(50, 50, CV_8UC3, const_cast<uint8_t*>(rgb));

    cv::Mat fused_image;

    double alpha = 1 - gray_propor; // 可见光图像的权重
    double beta = static_cast<double>(gray_propor);  // 热成像图像的权重
    double gamma = 0.0; // 增益常数

    cv::addWeighted(visible_image, alpha, thermal_image, beta, gamma, fused_image);

    size_t vec_size = fused_image.total() * fused_image.elemSize();
    std::vector<uint8_t> vec(vec_size);
    std::memcpy(vec.data(), fused_image.data, vec_size);

    return vec;
    // return thermal;
}
#endif

template<typename T>
std::vector<uint8_t> mix2(const std::vector<std::vector<T>>& matrix, const T mmax, const T mmin, const T vx, const T vn, const uint8_t* rgb, float gray_propor, bool reverse=false)
{
    if (matrix.empty()) return {};
    if (matrix.at(0).empty()) return {};

    uint32_t pixel_num = static_cast<uint32_t>(matrix.size() * matrix[0].size());
    std::vector<uint8_t> data;
    data.reserve(pixel_num);

    if (mmax <= mmin) {
        std::cerr << "mmax should be greater than mmin" << std::endl;
        return {};
    }

    for (const auto& row : matrix) {
        for (const T& value : row) {
            uint8_t mapped_value = 0;
            if (value > vx) {
                mapped_value = 255; //static_cast<uint8_t>(((value - mmin) * 255) / (mmax - mmin));
            } else if (value < vn) {
                mapped_value = 0;
            } else {
                mapped_value = static_cast<uint8_t>(((value - mmin) * 255) / (mmax - mmin));
            }
            if (reverse) {
                data.push_back(std::numeric_limits<uint8_t>::max()-mapped_value);
            } else {
                data.push_back(mapped_value);
            }
        }
    }

    return mix(data.data(), rgb, pixel_num, gray_propor);
}


#endif // __AHDJKBJCBQAJHXCIIIG_MIX_HPP__