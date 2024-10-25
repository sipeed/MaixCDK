#ifndef __QUIRC_HPP__
#define __QUIRC_HPP__

#include <stdint.h>

typedef struct {
    char data[512];
} quirc_qrcode_result_t;

void quirc_scan_qrcode_in_gray(uint8_t *gray, int width, int height, quirc_qrcode_result_t *result);

#endif