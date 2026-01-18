#ifndef _DECODER_H
#define _DECODER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ms_asr.h"
#include "ms_asr_cfg.h"

#include "decoder_raw.h"
#include "decoder_dig.h"
#include "decoder_wfst.h"
#include "decoder_kws.h"

#define DECODER_RAW   1
#define DECODER_DIG   2
#define DECODER_LVCSR 4
#define DECODER_KWS   8 
#define DECODER_aLL   0xffff 

typedef void (*decoder_cb_t)(void* data, int cnt);

int  decoder_init(int decoder_type, decoder_cb_t decoder_cb, void* decoder_args, int decoder_argc);
void decoder_deinit(int decoder_type);
void decoder_run(pnyp_t* pnyp_buf);
void decoder_clear(void);

#endif



