#ifndef _SFST_H_
#define _SFST_H_ 

#ifdef __cplusplus   
extern "C" {         
#endif

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>


#define INIT_ARC_IDX (0xffffffff)
#define DUMMY_IDX (0xffffffff)
#define COST_INF (1000000)
#define ARC_SIZE (6)  //默认使用6字节

#if ARC_SIZE==5
#define SYM_FINAL (0x7ff)
#elif ARC_SIZE==6
#define SYM_FINAL (0xfff)
#else
#define SYM_FINAL (0xfff)
#endif

/*
S0S1S2 S3|C0|T0 T1	5byte  S3:2|C0:3|T0:3  S:26bit
S表示state_out, 总共分配 24+2 = 26bit = 67M个 (测试lg8 state数小于40M，lg7 state数小于6M)
T表示sym_in,  总共分配 8+3 = 11bit = 2048个
C表示cost，总共分配3bit，最大8个状态，0,1,2,3,4,5,6,7   0.5压缩，即最大截断到14
*/

/*
# 6字节版
S0S1S2 S3|T0 T1 C	6byte  S3:2|T0:6  C:8bit
state 总共28bit, 256M=2.56亿状态
sym_in总共12bit, 4096种输入
cost 8bit量化
*/

/*
# 9字节版
S0S1S2 S3|T0 T1 C0C1C2C3	9byte  S3:2|T0:6  C:8bit
state 总共26bit, 64M=0.64亿状态 （考虑到最大4GB大小，64*6=3.84GB，够用）
sym_in总共14bit, 16384种输入
cost 原始4bit float
*/


typedef struct {
    char magic[4];
    uint32_t state_cnt;
    uint32_t arc_cnt;
    uint32_t state_oft;
    uint32_t arc_oft;
    uint32_t version; //压缩版version=1
    uint32_t res1;
    uint32_t res2;
}sfst_head_t;

typedef struct {
    uint8_t* arc_addr;
    uint32_t state_out;
    uint32_t sym_in;
    uint32_t sym_out;
    float    cost;
}sfst_arc_t;

typedef struct {
    uint32_t state_in;
    uint8_t* start_oft;
    uint8_t* end_oft;
    uint8_t* cur_oft;
}sfst_iter_t;

extern sfst_arc_t sfst_first_arc; 

//buf的偏移是字节为单位，即sfst文件最大4GB
//载入sfst和sym文件，可选是否以mmap形式
int sfst_open(char* sfst_name, char* sym_name, int is_mmap, uint8_t** _sfst_buf, uint32_t** _sym_buf, size_t* _sfst_size, size_t* _sym_size);       
//释放sfst和sym
void sfst_close(int is_mmap, uint8_t* sfst_addr, uint32_t* sym_addr, size_t sfst_size, size_t sym_size);        

//iter all arcs in the state
int sfst_iter_state_init(uint8_t* sfst_buf, uint32_t state_in, sfst_iter_t* sfst_iter);
int sfst_iter_state(sfst_iter_t* sfst_iter, sfst_arc_t* sfst_arc);

uint32_t sfst_get_arc_idx(uint8_t* sfst_buf, uint8_t* arc_addr);    //从arc地址查询idx
int sfst_get_arc(uint8_t* sfst_buf, uint32_t* sym_buf, uint32_t arc_idx, sfst_arc_t* sfst_arc); //返回对应arc_idx的arc_buf
float sfst_get_finalcost(uint8_t* sfst_buf, uint32_t state_in); //查找某状态的终止cost 
uint32_t sfst_get_state_cnt(uint8_t* sfst_buf);         //获取状态数
uint32_t sfst_get_arc_cnt(uint8_t* sfst_buf);           //获取总arc数

int sfst_print_state(uint8_t* sfst_buf);                //打印当前sfst的状态偏移表
int sfst_print_state_arc(uint8_t* sfst_buf, uint32_t* sym_buf, uint32_t state_in);             //打印当前state的arc表

#ifdef __cplusplus
}
#endif

#endif