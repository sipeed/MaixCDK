#include "decoder.h"

static int decoder_flag = 0;


int  decoder_init(int decoder_type, decoder_cb_t decoder_cb, void* decoder_args, int decoder_argc)
{
    int res = 0;
    switch(decoder_type) {
        case DECODER_RAW:
            res = decoder_raw_init(decoder_cb, decoder_args, decoder_argc);
            break;
        case DECODER_DIG:
            res = decoder_dig_init(decoder_cb, decoder_args, decoder_argc);
            break;
        case DECODER_LVCSR:
            res = decoder_wfst_init(decoder_cb, decoder_args, decoder_argc);
            break;
        case DECODER_KWS:
            res = decoder_kws_init(decoder_cb, decoder_args, decoder_argc);
            break;
        default:
            printf("decoder type %d not support\n", decoder_type);
            res = -1;
            break;
    }
    return res;
}

void decoder_deinit(int decoder_type)
{
    if(decoder_type&DECODER_RAW)   decoder_raw_deinit();
    if(decoder_type&DECODER_DIG)   decoder_dig_deinit();
    if(decoder_type&DECODER_LVCSR) decoder_wfst_deinit();
    if(decoder_type&DECODER_KWS)   decoder_kws_deinit();
    return;
}

void decoder_run(pnyp_t* pnyp_list)
{   //内部自动判断是否需要执行
    decoder_raw_run(pnyp_list);
    decoder_dig_run(pnyp_list);    
    decoder_wfst_run(pnyp_list);    
    decoder_kws_run(pnyp_list);  
    return;
}

void decoder_clear(void)
{
    decoder_raw_clear();
    decoder_dig_clear();    
    decoder_wfst_clear();    
    decoder_kws_clear(); 
    return;
}



