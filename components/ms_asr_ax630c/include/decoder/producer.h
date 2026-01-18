#ifndef _PRODUCER_H_
#define _PRODUCER_H_ 

#ifdef __cplusplus   
extern "C" {         
#endif

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ms_asr.h"
#include "ms_asr_cfg.h"

typedef struct {
    float* prob_buf;
    int mat_w;
    int mat_h;
}producer_mat_private_t;

typedef struct {
    float* prob_buf;
    int pny_n;
    int beam;
    int phone_n;
    int frame_n;
    int max_frames;
    float bg_prob;  //小概率的背景概率,-5,-10
    float scale;    //prob_buf = ln(prob)*scale
}producer_am_private_t;


typedef struct {
    //int (*init)(void* arg);         //初始化
    float (*LogLikelihood)(void* producer, int frame, int index); //当前帧的对数概率，自然对数?
    int (*IsLastFrame)(void* producer, int frame);  //是否是最后一帧了
    int (*NumFramesReady)(void* producer);    //总共的frame数量
    int (*NumIndices)(void* producer);        //声学模型里的状态数
    void* pdata;             //当前方法的私有数据
}producer_t;

int producer_mat_init(producer_t* producer, char* mat_name, int w, int h);
void producer_mat_deinit(producer_t* producer);
int producer_am_init(producer_t* producer, int pny_n, int beam, int phone_n, int max_frames, float bg_prob, float scale);
void producer_am_deinit(producer_t* producer);
int producer_am_push(producer_t* producer, pnyp_t* pnyp_list, int frames);

#ifdef __cplusplus
}
#endif

#endif