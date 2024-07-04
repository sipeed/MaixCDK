#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

void ui_utils_rgb_to_lab(uint8_t rgb_values[3], int8_t lab[3]);

#ifdef __cplusplus
}
#endif

#endif // __UI_UTILS_H__