#include "decoder_wfst.h"
#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include "ms_asr_utils.h"

#include <stdio.h>
#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "sfst.h"  
#include "dict.h"
#include "simple_decoder.h"
#include "producer.h"
#include "ms_asr_tbl.h"


#define DUMP_SFST   0
#define DUMP_PHONES 0
#define DUMP_WORDS  0

#define MAX_RES_LEN (8192)  //8192/2=4096, 1s 4字的话，可以支持约1000s

static producer_t producer;
static void* decoder;
static dict_t phones_dict;
static dict_t words_dict;
static uint8_t* sfst_buf;
static uint32_t* sym_buf;
static decode_result_t decode_result;
static decoder_cb_t decoder_cb=NULL;

/***************************** WFST private**********************************/
//process phones to uint32_t lm_tbl_cnt; const char* lm_tbl[2000]; uint16_t am2lm[2000];
static void _preprocess_phones(dict_t* lm_dict)
{
    lm_tbl_cnt = lm_dict->cnt;
    //填充lm phone tbl
    for(int i=0; i<lm_tbl_cnt; i++) {
        lm_tbl[i] = dict_get(lm_dict, i);
    }
    //填充 am2lm phone 映射表
    for(int i = 0; i < asrp.vocab_cnt; i++) {
        char* phone = (char*)am_vocab[i];
        int idx = -1;
        for(int j=0; j<lm_tbl_cnt; j++) {
            if(strcmp(phone, lm_tbl[j])==0){
                idx = j;
                break;
            }
        }
        if(idx<0){ //同时也包括了_
            am2lm[i] = 1;
        } else {
            am2lm[i] = idx;
        }
    }
    /*for(int i=0; i<asrp.vocab_cnt; i++){
        printf("%4d,", am2lm[i]);
        if(i%20==19)printf("\n");
    }*/
    return;
}

//存储到上一个停顿,隔离上下文干扰
static char words_stage[MAX_RES_LEN]={0};    
static char pnys_stage[MAX_RES_LEN]={0};
//对外输出的结果
static char words_out[MAX_RES_LEN]={0};    //1K字节，约500汉字
static char pnys_out[MAX_RES_LEN]={0};
static void _gen_result(char* words, char* pnys, char stop_ch)
{    
    int len;
    //添加结果到上一次停顿的末尾处
    strcpy(words_out, words_stage);
    for(int i = 0; i < decode_result.sym_out_len; i++) {
        char* word = dict_get(&words_dict, decode_result.sym_out[i]);
        if(strlen(words)+strlen(word)+1 < MAX_RES_LEN) {
            strcat(words, word);
            strcat(words, " ");
        }
    }
    strcpy(pnys_out, pnys_stage);
    for(int i = 0; i < decode_result.sym_in_len; i++) {
        char* pny = dict_get(&phones_dict, decode_result.sym_in[i]);
        if(strlen(pnys)+strlen(pny)+1 < MAX_RES_LEN) {
            strcat(pnys, pny);
            strcat(pnys, " ");
        }
    }
    //如果有停顿则更新stage字符串
    if(stop_ch != 0) {
        len = strlen(words);
        words[len-1] = stop_ch;
        len = strlen(pnys);
        pnys[len-1] = stop_ch;
        strcpy(words_stage, words);
        strcpy(pnys_stage, pnys);
        decoder_Clear(decoder);     //清除到目前为止的解码结果，防止上下文干扰
    }
    return ;
}


/***************************** WFST public**********************************/
static int is_mmap;
static size_t sfst_file_size;
static size_t sym_file_size;
static int l_init_flag = 0;
extern int ms_asr_dbg_flag;
int  decoder_wfst_init(decoder_cb_t _decoder_cb, size_t* decoder_args, int decoder_argc)
{
    DBG_TIME_INIT();
    int res = 0;
    decoder_cb = _decoder_cb;
    char* sfst_name = (char*)(decoder_args[0]);
    char* sym_name = (char*)(decoder_args[1]);
    char* phones_txt = (char*)(decoder_args[2]);
    char* words_txt = (char*)(decoder_args[3]);
    float beam = *((float*)(&decoder_args[4]));
    float bg_prob = *((float*)(&decoder_args[5]));
    float scale = *((float*)(&decoder_args[6]));
    is_mmap = (float)(decoder_args[7]);
    printf("decoder_wfst_init get args: %s, %s, %s, %s, %.3f, %.3f, %.3f, %d\n",\
        sfst_name, sym_name, phones_txt, words_txt, beam, bg_prob, scale, is_mmap);
    //read sfst
    res = sfst_open(sfst_name, sym_name, is_mmap, &sfst_buf, &sym_buf, &sfst_file_size, &sym_file_size);
    if(res != 0) {
        goto out;
    }
    DBG_TIME("Read FST");
    uint32_t state_cnt = sfst_get_state_cnt(sfst_buf);
    uint32_t arc_cnt = sfst_get_arc_cnt(sfst_buf);
    printf("Read Fst: state cnt %d, arc cnt %d\n", state_cnt, arc_cnt);    
#if DUMP_SFST      
    sfst_print_state(sfst_buf); 
    for(uint32_t state = 0; state < 1; state++) { //state_cnt
        sfst_print_state_arc(sfst_buf, sym_buf, state); 
    }
#endif

    //read phones.txt
    res = dict_open(phones_txt, &phones_dict);
    if(res != 0) {
        printf("load %s error!\n", phones_txt);
        res = -2;
        goto free_sfst;
    }
    DBG_TIME("Read phones");
    //dict_dump(&phones_dict);
    _preprocess_phones(&phones_dict);

    //read words.txt
    res = dict_open(words_txt, &words_dict);
    if(res != 0) {
        printf("load %s error!\n", words_txt);
        res = -3;
        goto free_phones;
    }
    DBG_TIME("Read words");
    //dict_dump(&words_dict);

    //init decoder
    decoder = decoder_Init(beam, sfst_buf, sym_buf); 
    if(decoder == NULL) {
        printf("decoder_Init failed!\n");
        res = -4;
        goto free_words;
    }
    //init data producer
    int phone_n = lm_tbl_cnt;
    res = producer_am_init(&producer, asrp.vocab_cnt, BEAM_CNT, phone_n, asrp.model_core_len, -1*bg_prob, scale);
    if(res != 0) {
        res = -5;
        printf("producer_am_init failed!\n");
        goto free_decoder;
    }
    l_init_flag = 1;
    decoder_wfst_clear();
    return 0;
    //free resource
free_mat:
    producer_am_deinit(&producer);
free_decoder:
    decoder_Deinit(decoder);
free_words:
    dict_close(&words_dict); 
free_phones:
    dict_close(&phones_dict);
free_sfst:
    sfst_close(is_mmap, sfst_buf, sym_buf, sfst_file_size, sym_file_size);
out:
    return res; 
}

void decoder_wfst_deinit(void)
{
    if(l_init_flag){
        producer_am_deinit(&producer);
        decoder_Deinit(decoder);
        dict_close(&words_dict); 
        dict_close(&phones_dict);
        sfst_close(is_mmap, sfst_buf, sym_buf, sfst_file_size, sym_file_size);
        decoder_cb = NULL;
        l_init_flag = 0;
    }
    return;
}


int decoder_wfst_run(pnyp_t* pnyp_list)
{
    int res;
    char stop_ch = 0;
    if(decoder_cb) {
        //producer 压入新数据
        res = producer_am_push(&producer, pnyp_list, asrp.model_core_len);
        if(res<0) {
            printf("producer_am_push error!\n");
            return -1;
        } else if (res == 1) {
            printf("### SIL to clear decoder!\n");
            stop_ch = ',';
        } else if (res == 2) {
            printf("### Long SIL to clear decoder!\n");
            stop_ch = '.';
        }

        //WFST解码器 持续解码
        res = decoder_Decoding(decoder, &producer);
        if(res!=0) {
            printf("cur frame no decoder result\n");
            return -1;
        }
        //WFST解码器最佳路径求解
        res = decoder_GetBestPath(decoder, &decode_result);
        if(ms_asr_dbg_flag&DBG_LVCSR) {
            if(res == 0) {
                printf("========Print Decode ARC Sequence\n");
                decoder_PrintArcs(decode_result.arcs, decode_result.arcs_len);
                printf("========Print Decode Symbol In Sequence\n");
                decoder_PrintSymbols(decode_result.sym_in, decode_result.sym_in_len, &phones_dict);
                printf("========Print Decode Symbol Out Sequence\n");
                decoder_PrintSymbols(decode_result.sym_out, decode_result.sym_out_len, &words_dict);
            } else {
                printf("decoder_GetBestPath failed!\n");
            }
        }
        //调用回调函数
        char* data[2] = {words_out, pnys_out};
        _gen_result(words_out, pnys_out, stop_ch);

        decoder_cb(data, 2);//数字回调
    }
    return 0;
}

void decoder_wfst_clear(void)
{
    if(l_init_flag){
        words_stage[0] = 0;
        pnys_stage[0] = 0;
        decoder_Clear(decoder);
    }
    return;
}
