#ifndef _DECODER_WFST_H
#define _DECODER_WFST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ms_asr.h"
#include "ms_asr_cfg.h"

typedef void (*decoder_cb_t)(void* data, int cnt);

int  decoder_wfst_init(decoder_cb_t decoder_cb, size_t* decoder_args, int decoder_argc);
void decoder_wfst_deinit(void);
int decoder_wfst_run(pnyp_t* pnyp_list);
void decoder_wfst_clear(void);

#endif



