#ifndef _DECODER_RAW_H
#define _DECODER_RAW_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ms_asr.h"
#include "ms_asr_cfg.h"

typedef void (*decoder_cb_t)(void* data, int cnt);

int  decoder_raw_init(decoder_cb_t decoder_cb, size_t* decoder_args, int decoder_argc);
void decoder_raw_deinit(void);
void decoder_raw_run(pnyp_t* pnyp_buf);
void decoder_raw_clear(void);

#endif



