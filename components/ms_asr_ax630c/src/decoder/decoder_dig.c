#include <float.h>
#include "math.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include "decoder_dig.h"
#include "ms_asr.h"
#include "ms_asr_cfg.h"

/*****************************************************************************/
// Macro
/*****************************************************************************/
#define DEBUG_LINE()                              \
    do                                             \
    {                                              \
        printf("[decode] L%d\r\n", __LINE__); \
    } while(0)


/*****************************************************************************/
// Constants
/*****************************************************************************/
//数字枚举 转 拼音下标
/*
["ling2", "lin2", "ren2"],
["yi1","yao1",],
["er4",],
["san1", "shan1",],
["si4", "shi4",],
["wu3",],
["liu4",],
["qi1",],
["ba1",],
["jiu3",],
["shi2",],
["bai3",],
["qian1",],
["wan4",],
["dian3"]
*/
#define SIMILAR_N 5
const uint16_t digit_cvt_tbl[16][SIMILAR_N]= {
	{ 130,  281,   23, 2000, 2000, },
	{   8,  337, 2000, 2000, 2000, },
	{  78, 2000, 2000, 2000, 2000, },
	{  88,  255, 2000, 2000, 2000, },
	{ 145,    2, 2000, 2000, 2000, },
	{  60, 2000, 2000, 2000, 2000, },
	{ 161, 2000, 2000, 2000, 2000, },
	{ 113, 2000, 2000, 2000, 2000, },
	{ 137, 2000, 2000, 2000, 2000, },
	{ 105, 2000, 2000, 2000, 2000, },
	{  11, 2000, 2000, 2000, 2000, },
	{ 211, 2000, 2000, 2000, 2000, },
	{ 343, 2000, 2000, 2000, 2000, },
	{ 362, 2000, 2000, 2000, 2000, },
	{  38, 2000, 2000, 2000, 2000, },
};

//需要注意的近音词
//百<->八，七<->千，点
const float digit_cvt_gate[16]= {
	0.20, //0
	0.20, //1
	0.20, //2
	0.20, //3
	0.20, //4
	0.20, //5
	0.20, //6
	0.20, //7
	0.20, //8
	0.20, //9
	0.20, //十
	0.20, //百
	0.20, //千
	0.20, //万
	0.20, //.
	0.20, //None
};


static decoder_cb_t decoder_cb=NULL;

/******************* 数字解码 **********************/
static int is_in_mono(uint32_t idx, uint16_t* idx_list, uint8_t idx_cnt)
{
	int res = 0;
	for(int i=0; i < idx_cnt; i++) {
		if(idx == idx_list[i]) {
			res = 1;
			break;
		}
	}
	return res;
}

//当前时刻的前五预测值，计算指定单音节字的概率
static float cal_mono_p(pnyp_t* pnyp_list, uint8_t pny_cnt, uint16_t* idx_list, uint8_t idx_cnt)
{
	float    p = 0;
	uint32_t pny_idx;
	float    pny_p;
	for(int pny_i=0; pny_i < pny_cnt; pny_i++) {
		pny_idx = pnyp_list[pny_i].idx;
		pny_p   = pnyp_list[pny_i].p;
		if(is_in_mono(pny_idx, idx_list, idx_cnt)) {
			p += pny_p;
		}
	}
	return p;
}	

static int _get_maxf(float* val_list, int cnt)
{
	float max = FLT_MIN;
	int max_i=0;
	for (int i = 0; i < cnt; i++)  //10us  
    {
		if(max < val_list[i]) {
			max = val_list[i];
			max_i = i;
		}
    }
	return max_i;
}

//输入参数：
//每个时刻的拼音预测前五值：pnyp_t pnyp_list[PNY_LOG_LEN*5]; 
//每个时刻的数字预测：uint8_t digits[PNY_LOG_LEN];
static void decode_pny2digit(pnyp_t* pnyp_list, int t_cnt, uint8_t* digits)
{
	uint8_t t, i;
	float digit_p[ASR_DIGIT_NONE+1];
	for(t=0; t < t_cnt; t++ ) {
		for(i=0; i <= ASR_DIGIT_NONE; i++ ) { //逐个数字计算概率
			digit_p[i] = cal_mono_p(pnyp_list+BEAM_CNT*t, BEAM_CNT, (uint16_t*)digit_cvt_tbl[i], SIMILAR_N);
			//printf("%c:%.3f, ", digit_char[i], digit_p[i]);
		}
		//printf("\r\n");
		int maxi = _get_maxf(digit_p, ASR_DIGIT_NONE); //这里排除掉 ASR_DIGIT_NONE进行统计
		if(digit_p[maxi]>digit_cvt_gate[maxi]) {
			//printf("##t[%d]: %c, %.3f\r\n", t, digit_char[maxi], digit_p[maxi]);
			digits[t] = maxi;
		} else {
			//printf("##t[%d]: -, %.3f\r\n", t, digit_p[ASR_DIGIT_NONE]);
			digits[t] = ASR_DIGIT_NONE;
		}
	}
	return;
}


static uint8_t digit_log[DIGIT_LOG_LEN]={0}; //记录最近4s的数字记录
static char digit_res[DIGIT_LOG_LEN/2]={0}; //记录最近4s的数字记录
static uint8_t pause_len = 10;				//视为停顿的间隙长度，单位64ms
void decode_digit(pnyp_t* pnyp_list, int t_cnt, char** res, uint8_t** orignal_res)
{
	uint8_t digits[16]; 	//这里预留16个拼音，实际是4个拼音
	decode_pny2digit(pnyp_list, t_cnt, digits);	//解码当前4格的数字
	memmove(digit_log, digit_log+t_cnt, DIGIT_LOG_LEN-t_cnt);	//前移t_cnt格
	memcpy(digit_log+DIGIT_LOG_LEN-t_cnt, digits, t_cnt); //拷贝入新结果
	
	//预处理
	int log_i, res_i;
	uint8_t last_d0 = ASR_DIGIT_NONE;
	//uint8_t last_d1 = ASR_DIGIT_NONE; //更早
	uint8_t continue_cnt = 0;
	uint8_t digit;
	memset(digit_res, 0, DIGIT_LOG_LEN/2);
	for(log_i=0, res_i=0; log_i < DIGIT_LOG_LEN; log_i++) {
		digit = digit_log[log_i];
		if(digit == ASR_DIGIT_NONE) {	//无声音
			continue_cnt += 1;
		} else {	//有识别结果
			if(continue_cnt>pause_len) {	//超过最大空白次数，则记录一个间隔
				digit_res[res_i] = '_';
				res_i += 1;
			}
			continue_cnt = 0;
			//合并相邻相同数字
			if(digit == last_d0){ //|| 
				//(digit == last_d1 && last_d0 == ASR_DIGIT_NONE)) { //只间隔1个_的相同数字也合并
			} else {	//新数字记录
				digit_res[res_i] = digit_char[digit];
				res_i += 1;
			}
		}
		//last_d1 = last_d0;
		last_d0 = digit;
	}
	if(continue_cnt>pause_len) {	//结尾的空白
		digit_res[res_i] = '_';
		res_i += 1;
	}
	//printf("RESULT:  %s\r\n", digit_res);
	*orignal_res = digit_log;	//原始结果，记录最近4s
	*res         = digit_res;	//预处理的数字结果
	//res[0];
	return;
}
/*****************************Decoder DIG public**********************************/

//args: int pasue_ms
int  decoder_dig_init(decoder_cb_t _decoder_cb, size_t* decoder_args, int decoder_argc)
{
    decoder_cb = _decoder_cb;
    int arg0 = (int)decoder_args[0];
    printf("decoder_dig_init get arg0 %d\n", arg0);
    pause_len = arg0/64;
    decoder_dig_clear();
    return 0;
}

void decoder_dig_deinit(void)
{
    decoder_dig_clear();
    decoder_cb = NULL;
}

void decoder_dig_run(pnyp_t* pnyp_list)
{
	static char*    _digit_res;	//因为在下次循环才使用，所以需要static
	static uint8_t* _orignal_res;
    
    if(decoder_cb) {
        //数字解码使用独立的更长的历史buf
        decode_digit(pnyp_list, asrp.model_core_len, &_digit_res, &_orignal_res);	
        decoder_cb(_digit_res, strlen(_digit_res));//数字回调
    }
    return;
}

void decoder_dig_clear(void)
{
    memset(digit_log, ASR_DIGIT_NONE, DIGIT_LOG_LEN);
    return;
}


