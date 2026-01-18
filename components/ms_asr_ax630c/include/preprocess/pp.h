#ifndef _PP_H
#define _PP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "device.h"

int  pp_init(asr_device_t* asr_dev, char* device_name);
void pp_deinit(void);
void  pp_clear(void);
uint8_t* pp_get(void);  //逐帧计算得到mel谱给am，计算出错或者run out返回NULL


#endif