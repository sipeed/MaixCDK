#ifndef __ZBAR_HPP__
#define __ZBAR_HPP__

#include <stdint.h>
#include <vector>
#include <string>

typedef struct {
    int counter;
    std::vector<std::string> data;
    std::vector<std::vector<int>> corners;  // numebr*{x1,y1,x2,y2,x3,y3,x4,y4}
} zbar_qrcode_result_t;

int zbar_scan_qrcode_in_gray(uint8_t *gray, int width, int height, zbar_qrcode_result_t *result);

#endif