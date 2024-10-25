#include "zbar.hpp"
#include "zbar.h"
#include <cstring>

using namespace zbar;

int zbar_scan_qrcode_in_gray(uint8_t *gray, int width, int height, zbar_qrcode_result_t *result)
{
    zbar_image_scanner_t *scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    zbar_image_t *zbar_image = zbar_image_create();
    zbar_image_set_format(zbar_image, *(int*)"Y800");
    zbar_image_set_size(zbar_image, width, height);
    zbar_image_set_data(zbar_image, gray, width * height, NULL);
    int n = zbar_scan_image(scanner, zbar_image);
    result->counter = n;
    if (n > 0) {
        const zbar_symbol_t *symbol = zbar_image_first_symbol(zbar_image);
        for (; symbol; symbol = zbar_symbol_next(symbol)) {
            std::vector<int> corner(8);
            if(zbar_symbol_get_loc_size(symbol) >= 4) {
                for(int i = 0; i < 4; i++) {
                    int x = zbar_symbol_get_loc_x(symbol, i);
                    int y = zbar_symbol_get_loc_y(symbol, i);
                    corner[i * 2] = x;
                    corner[i * 2 + 1] = y;
                }
            }

            zbar_symbol_type_t type = zbar_symbol_get_type(symbol);
            const char *data = zbar_symbol_get_data(symbol);
            std::string new_data = data;
            result->data.push_back(new_data);
            result->corners.push_back(corner);
        }
    }
    zbar_image_destroy(zbar_image);
    zbar_image_scanner_destroy(scanner);
    return 0;
}