#ifndef _ASR_DECODE_DIG_H
#define _ASR_DECODE_DIG_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ms_asr.h"
#include "ms_asr_cfg.h"

/*****************************************************************************/
// Macro definitions
/*****************************************************************************/
#define ASR_KW_MAX_CNT	128 //最多128个关键词，512字节
#define ASR_MAX_PRED	5	//每次返回最多的预测结果数量

#define SIMILAR_N 		5  	//近音词数量，>=1

#define PNY_LOG_LEN  	16	//1s  //64ms为单位
#define DIGIT_LOG_LEN  	64	//4s
/*****************************************************************************/
// Enums
/*****************************************************************************/


/*****************************************************************************/
// Types
/*****************************************************************************/
typedef void (*decoder_cb_t)(void* data, int cnt);



/*****************************************************************************/
// Functions
/*****************************************************************************/
int  decoder_dig_init(decoder_cb_t decoder_cb, size_t* decoder_args, int decoder_argc);
void decoder_dig_deinit(void);
void decoder_dig_run(pnyp_t* pnyp_list);
void decoder_dig_clear(void);

#endif

