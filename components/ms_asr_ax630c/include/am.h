#ifndef _AM_H
#define _AM_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ms_asr.h"

#ifdef __cplusplus
extern "C" {
#endif
int am_init(char* model_name, int phone_type);
void am_deinit(void);
pnyp_t* am_run(uint8_t* melbuf);
#ifdef __cplusplus
}
#endif

#endif



