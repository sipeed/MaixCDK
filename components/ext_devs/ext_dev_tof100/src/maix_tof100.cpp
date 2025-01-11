#include "maix_tof100.hpp"
#include "dragonfly.h"
#include "maix_basic.hpp"
#include "maix_pinmap.hpp"
#include "maix_gpio.hpp"
#include "dragonfly.h"

#include <functional>

#pragma GCC diagnostic ignored "-Wsign-compare"

#define eprintln(fmt, ...) do {maix::log::error0("[%s]", TAG()); \
                                printf(fmt, ##__VA_ARGS__);\
                                printf("\n");} while(0)
#define println(fmt, ...) do {maix::log::info0("[%s]", TAG()); \
                                printf(fmt, ##__VA_ARGS__);\
                                printf("\n");} while(0)

#define dbg() do {maix::log::info("-- %d", __LINE__);} while(0)

#define panic(fmt, ...) do {eprintln(fmt, ##__VA_ARGS__);\
                            char _buff[256]{0x00}; \
                            ::snprintf(_buff, std::size(_buff), "In \n\tfile <%s> \n\tfunc <%s> \n\tlen <%d>\n", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
                            throw std::runtime_error(std::string(_buff));} while(0)


namespace maix::ext_dev::tof100 {

constexpr TOFPoint empty_point{-1,-1, 0};

const char* TAG() {
    return "Maix Tof100";
}

void _for_each_in_matrix(const TOFMatrix& matrix, std::function<void(int,int,uint32_t)> cb)
{
    if (matrix.empty())
        return;
    if (matrix[0].empty())
        return;

    for (int y = 0; y < matrix.size(); ++y) {
        for (int x = 0; x < matrix.at(0).size(); ++x) {
            cb(x, y, matrix[y][x]);
        }
    }
}


Tof100::Tof100(int spi_bus_num, Resolution resolution, ::maix::ext_dev::cmap::Cmap cmap, int dis_min, int dis_max)
    : _cmap(cmap), _min(dis_min), _max(dis_max), _wh(static_cast<uint32_t>(resolution)), _fps_limit(20) /* FPS: auto */
{

    if (spi_bus_num == 4) {
        using namespace maix::peripheral::pinmap;
        using namespace maix::err;
        const std::vector<std::pair<std::string, std::string>> pins = {
            {"A24", "SPI4_CS"},
            {"A23", "SPI4_MISO"},
            {"A25", "SPI4_MOSI"},
            {"A22", "SPI4_SCK"},
            {"A15", "GPIOA15"},
            {"A27", "GPIOA27"},
        };
        for (auto& i : pins) {
            if (set_pin_function(i.first, i.second) != Err::ERR_NONE) {
                panic("Set %s --> %s failed!", i.first.c_str(), i.second.c_str());
            }
        }
        {
            using namespace maix::peripheral::gpio;
            GPIO a15("A15", Mode::OUT);
            GPIO a27("A27", Mode::OUT);
            a15.low();
            a27.low();
        }
    }

    spi_init(spi_bus_num);

    if (DragSwReset() != 0) {
        panic("TOF reset failed!");
    }

    if (DragISPBooting() != 0) {
        panic("TOF init failed!");
    }

    // msl_process_startup(this->_wh, this->_fps_limit, true);
    if (this->_wh == 25) {
        this->_mode = static_cast<uint32_t>(MSL_BinningMode2_4x4);
    } else if (this->_wh == 50) {
        this->_mode = static_cast<uint32_t>(MSL_BinningMode1_2x2);
    } else if (this->_wh == 100) {
        this->_mode = static_cast<uint32_t>(MSL_BinningMode0_1x1);
    }

    msl_setup(this->_fps_limit, static_cast<BinningMode>(this->_mode), 1);

    if (DragISPInit() != 0) {
        panic("TOF init failed!");
    }

    if (DragSetISPStart() != 0) {
        panic("TOF init failed!");
    }

    maix::time::sleep_ms(2);

    this->_data_size = 100 >> static_cast<uint32_t>(this->_mode);
    this->_data_size *= this->_data_size;

    uint32_t buffer_size = 20 + this->_data_size*2 + 2;
    this->_frame_buffer = std::unique_ptr<uint8_t[]>(new uint8_t[buffer_size]);

}

TOFMatrix Tof100::matrix()
{
    uint8_t* FrameBuf = this->_frame_buffer.get();
    int ret = SPII2CBurstDataRead(DATA_BASE_ADDRESS + DATA_OFFSET_ADDRESS,
                                  (uint32_t *)(FrameBuf),
                                  DATA_HEAD_LENGTH + this->_data_size * 2);
    if (ret) {
        eprintln("tof read frame head failed!");
        return {};
    }

    if (((uint16_t *)FrameBuf)[0] != 0xA0CC) {
        eprintln("tof Head[%02x %02x] is missmatch\r\n", FrameBuf[0], FrameBuf[1]);
        return {};
    }


    ((uint16_t *)FrameBuf)[0] = 0xFF00;
    if ((this->_data_size * 2 + DATA_HEAD_INFO_LENGTH) != ((uint16_t *)FrameBuf)[1]) {
        eprintln("ERROR: Lenth[%d] is missmatch\r\n", ((uint16_t *)FrameBuf)[1]);
        return {};
    }


    uint8_t checksum = 0;
    TOFMatrix res(this->_wh, std::vector<uint32_t>(this->_wh));
    int min_x, min_y, max_x, max_y;
    uint32_t min = std::numeric_limits<decltype(min)>::max();
    uint32_t max = std::numeric_limits<uint32_t>::min();
    int center_x, center_y;
    uint32_t center;
    center_x = center_y = this->_wh/2;


    for (int i = 0; i < this->_data_size; i++) {
		size_t src_idx = (this->_data_size - 1) - i;
		uint16_t tmp = ((uint16_t *)(FrameBuf + DATA_HEAD_LENGTH))[src_idx];

        // this->_dis[src_idx/this->_wh][src_idx%this->_wh] = tmp;
        int _x = static_cast<int>(src_idx%this->_wh);
        int _y = static_cast<int>(src_idx/this->_wh);

        res[_y][_x] = tmp;

        if (tmp < min) {
            min = tmp;
            min_x = _x;
            min_y = _y;
        }

        if (tmp > max) {
            max = tmp;
            max_x = _x;
            max_y = _y;
        }

        if (_y == center_y && _x == center_x) {
            center = tmp;
        }

		float res;
		if (this->_quantization_step) {
		    res = tmp;
		    res /= this->_quantization_step;
		} else {
		    if (tmp > 2500) tmp = 2500;
		    res = 5.1f * sqrtf(tmp);
		}
		tmp = (uint16_t)res;
		(FrameBuf + DATA_HEAD_LENGTH + 2 * this->_data_size - 1)[-i] =
			tmp & 0xff00 ? 0xff : (tmp & 0xff);
		checksum +=
			(FrameBuf + DATA_HEAD_LENGTH + 2 * this->_data_size - 1)[-i];
	}


	for (int i = 0; i < this->_data_size; i++) {
		(FrameBuf + DATA_HEAD_LENGTH)[i] =
			(FrameBuf + DATA_HEAD_LENGTH + 2 * this->_data_size - 1)[-i];
	}


	((uint16_t *)FrameBuf)[1] = DATA_HEAD_INFO_LENGTH + this->_data_size;
	for (int i = 0; i < DATA_HEAD_LENGTH; i++) {
		checksum += FrameBuf[i];
	}
	(FrameBuf + DATA_HEAD_LENGTH + this->_data_size)[0] = checksum;
	(FrameBuf + DATA_HEAD_LENGTH + this->_data_size)[1] = 0xDD;


    /* All succ, update points */
    this->_dis_max = std::make_tuple(max_x, max_y, max);
    this->_dis_min = std::make_tuple(min_x, min_y, min);
    this->_dis_center = std::make_tuple(center_x, center_y, center);
    return res;
}

::maix::image::Image* Tof100::image()
{
    return this->image_from(this->matrix());
}

TOFPoint Tof100::max_dis_point()
{
    return this->_dis_max;
}

TOFPoint Tof100::min_dis_point()
{
    return this->_dis_min;
}

TOFPoint Tof100::center_point()
{
    return this->_dis_center;
}

::maix::image::Image* Tof100::image_from(const TOFMatrix& matrix)
{
    if (matrix.empty()) return nullptr;

    const cmap::CmapArray* array = cmap::get(this->_cmap);

    uint8_t buffer[100*100*3];

    int max = this->_max;
    int min = this->_min;
    if (max == min) {
        max = 1200; //std::get<2>(this->max_dis_point());
        min = std::get<2>(this->min_dis_point());
    }
    int range = max - min;

    int pixel_cnt = 0;

    for (const auto& line : matrix) {
        for (auto dis : line) {
            dis -= min;
            if (dis < 0) dis = 0;
            if (dis > range) dis = range;

            uint32_t index = static_cast<uint32_t>(
                array->size()-1-static_cast<int>(
                    floor(static_cast<float>(dis)/range*(array->size()-1))
                )
            );
            buffer[pixel_cnt*3]   = std::get<0>((*array)[index]);
            buffer[pixel_cnt*3+1] = std::get<1>((*array)[index]);
            buffer[pixel_cnt*3+2] = std::get<2>((*array)[index]);

            pixel_cnt++;
        }
    }

    if (this->_wh == 25) {
        std::vector<uint8_t> new_buffer(50 * 50 * 3);
        for (int y = 0; y < 25; ++y) {
            for (int x = 0; x < 25; ++x) {
                int src_index = (y * 25 + x) * 3;
                for (int dy = 0; dy < 2; ++dy) {
                    for (int dx = 0; dx < 2; ++dx) {
                        int new_y = y * 2 + dy;
                        int new_x = x * 2 + dx;
                        int dst_index = (new_y * 50 + new_x) * 3;

                        new_buffer[dst_index]     = buffer[src_index];
                        new_buffer[dst_index + 1] = buffer[src_index + 1];
                        new_buffer[dst_index + 2] = buffer[src_index + 2];
                    }
                }
            }
        }
        return new ::maix::image::Image(50, 50, image::FMT_RGB888, new_buffer.data(), 50*50*3, true);
    }

    return new ::maix::image::Image(this->_wh, this->_wh, image::FMT_RGB888, buffer, this->_data_size*3, true);
}

TOFPoint Tof100::max_dis_point_from(const TOFMatrix& matrix)
{
    int _x, _y;
    uint32_t v = std::numeric_limits<uint32_t>::min();
    _for_each_in_matrix(matrix, [&](int x, int y, uint32_t value){
        if (value > v) {
            v = value;
            _x = x;
            _y = y;
        }
    });
    return std::make_tuple(_x, _y, v);
}

TOFPoint Tof100::min_dis_point_from(const TOFMatrix& matrix)
{
    int _x, _y;
    uint32_t v = std::numeric_limits<uint32_t>::max();
    _for_each_in_matrix(matrix, [&](int x, int y, uint32_t value){
        if (value < v) {
            v = value;
            _x = x;
            _y = y;
        }
    });
    return std::make_tuple(_x, _y, v);
}

TOFPoint Tof100::center_point_from(const TOFMatrix& matrix)
{
    return std::make_tuple(matrix.size()/2, matrix.at(0).size()/2, matrix[matrix.at(0).size()/2][matrix.size()/2]);
}

}
