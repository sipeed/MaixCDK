#ifndef _DICT_H_
#define _DICT_H_ 

#ifdef __cplusplus   
extern "C" {         
#endif

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define DICT_TXT 0
#define DICT_BIN 1

typedef struct {
    char magic[4];
    uint32_t word_cnt;
    uint32_t word_cnt1;
    uint32_t idx_oft;
    uint32_t word_oft;
    uint32_t res0;
    uint32_t res1;
    uint32_t res2;
}dict_head_t;

typedef struct {
    int type;
    uint32_t  cnt;
    uint32_t* idx_buf;
    char* dict_buf;
    size_t size;
    char* bin_buf;
}dict_t;

//注意字典最后一行必须为空行（以\n计数）
int dict_open(char* dict_name, dict_t* dict);
void dict_close(dict_t* dict);
int dict_openbin(char* dict_name, dict_t* dict);
void dict_closebin(dict_t* dict);
char* dict_get(dict_t* dict, uint32_t idx);
void dict_dump(dict_t* dict);

#ifdef __cplusplus
}
#endif

#endif