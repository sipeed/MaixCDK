#ifndef _SIMPLE_DECODER_H_
#define _SIMPLE_DECODER_H_ 

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "dict.h"
#include "sfst.h"
#include "producer.h"


#define ARC_LEN 256

typedef struct _Token{
    uint8_t* arc_;
    struct _Token* prev_;
    int32_t     ref_count_;
    float       cost_;
}Token;

typedef uint32_t StateId;

Token* tok_new(sfst_arc_t* arc, float acoustic_cost, Token* prev); //新建
void tok_del(Token* tok); //删除，直到最近的上游交汇点

typedef struct {
    uint32_t arcs[ARC_LEN];
    uint32_t arcs_len;
    uint32_t sym_in[ARC_LEN];
    uint32_t sym_in_len;
    uint32_t sym_out[ARC_LEN];
    uint32_t sym_out_len;
}decode_result_t;

#ifdef __cplusplus
extern "C"{
#endif

void* decoder_Init(float beam, uint8_t* sfst_buf, uint32_t* sym_buf);
void decoder_Deinit(void* decoder);
void decoder_PrintArcs(uint32_t* arcs, uint32_t arcs_len);
void decoder_PrintSymbols(uint32_t* syms, uint32_t syms_len, dict_t* dict);

int decoder_Decode(void* decoder, producer_t* producer);   //一次性解码完
int decoder_Decoding(void* decoder, producer_t* producer);//持续解码
void decoder_Clear(void* decoder);   //清空tok配置，重新开始解码
int decoder_Decode(void* decoder, producer_t* producer);
int decoder_ReachedFinal(void* decoder);
int decoder_GetBestPath(void* decoder, decode_result_t* decode_result);

#ifdef __cplusplus
}
#endif

#endif