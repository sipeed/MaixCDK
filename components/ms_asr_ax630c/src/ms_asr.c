#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include "device.h"
#include "pp.h"
#include "am.h"
#include "decoder.h"

//PLATFORM_MAIXCAM
#define DEVICE_PCM    0
#define DEVICE_MIC    1
#define DEVICE_WAV    2
#define DEVICE_MIC2   3
#define DEVICE_MIC4   4
#define DEVICE_CUSTOM 5

#define DECODER_RAW   1
#define DECODER_DIG   2
#define DECODER_LVCSR 4
#define DECODER_KWS   8
#define DECODER_ALL   0xffff

#define DBG_PARAM 0
//全局变量
asr_param_t asrp;

/***************************** ASR private **********************************/
//预计算相关参数到全局变量里（asr lib 内部使用）
static void cal_asr_param(am_args_t* am_args)
{
    asrp.model_name    = am_args->model_name;
    asrp.model_in_len  = am_args->model_in_len;
    asrp.strip_l       = am_args->strip_l;
    asrp.strip_r       = am_args->strip_r;
    asrp.agc           = am_args->agc;
    asrp.phone_type    = am_args->phone_type;
    asrp.vocab_cnt     = -1; //在am初始化后填充
    //计算strip参数
    asrp.strip_out     = (asrp.strip_l+asrp.strip_r);
    asrp.strip_mel     = (asrp.strip_out*8);    //模型里div了8
    asrp.strip_p       = (asrp.strip_mel*FFT_MOVE); //2048点，128ms原始数据
    //计算model参数
    asrp.model_in_time = (asrp.model_in_len*8); //512ms, 一个len=8ms
    asrp.model_in_p    = (asrp.model_in_len*FFT_MOVE+FFT_N-FFT_MOVE); //8K点+额外
    asrp.model_out_len = (asrp.model_in_len/8);
    asrp.model_core_len= (asrp.model_out_len-asrp.strip_l-asrp.strip_r); //输出中心可用的内容
    //计算wav参数
    asrp.wav_buf_len   = (asrp.model_in_p);    //这么多点数计算出来的MEL给模型
    asrp.wav_core_len  = ((asrp.model_out_len-asrp.strip_l-asrp.strip_r)*8*FFT_MOVE);  //每次新入的buf点数
    asrp.wav_rest_len  = ((asrp.strip_l+asrp.strip_r)*8*FFT_MOVE+FFT_N-FFT_MOVE);
#if DBG_PARAM   
    printf("DBG ASR PARAM:\n");
    printf("AUDIO_RATE=%d, FFT_N=%d, FFT_MOVE=%d\n", AUDIO_RATE, FFT_N, FFT_MOVE);
    printf("STRIP_OUT=%d, STRIP_MEL=%d, STRIP_P=%d\n", asrp.strip_out, asrp.strip_mel, asrp.strip_p);
    printf("MODEL: IN_LEN=%d, IN_TIME=%dms, IN_P=%d, OUT_LEN=%d\n", \
        asrp.model_in_len, asrp.model_in_time, asrp.model_in_p, asrp.model_out_len);
    printf("WAV: BUF_LEN=%d, CORE_LEN=%d, REST_LEN=%d\n", \
        asrp.wav_buf_len, asrp.wav_core_len, asrp.wav_rest_len);
    printf("MEL_N=%d, BEAM_CNT=%d\n\n", MEL_N, BEAM_CNT);    
#endif
    return;
}


/***************************** ASR public **********************************/
//decoder: init, deinit, push_deocde
int ms_asr_dbg_flag = 0;
int ms_asr_init(int device_type, char* device_name, am_args_t* am_args, int dbg_flag)
{
    ms_asr_dbg_flag = dbg_flag;
    int res = 0;
    asr_device_t* asr_dev = NULL;
    cal_asr_param(am_args);

    //初始化设备
    if(device_type == DEVICE_PCM) {
        asr_dev = &dev_pcm;        
    } else if(device_type == DEVICE_MIC) {
        asr_dev = &dev_mic;   
    } else if(device_type == DEVICE_WAV) {
        asr_dev = &dev_wav;  
    } else if(device_type == DEVICE_CUSTOM) {
        asr_dev = (asr_device_t*)device_name;  
    } else {
        printf("error device type %d\n", device_type);
        return -1;
    }
    //初始化preprocess, pp为am提供数据
    res = pp_init(asr_dev, device_name);
    if(res != 0) {
        printf("pp_init error!\n");
        return -1;
    }
    //初始化am
    res = am_init(asrp.model_name, asrp.phone_type);
    if(res != 0) {
        printf("am_init error!\n");
        goto free_pp;
    }

    //decoder另外初始化
    return 0;
    
free_pp:
    pp_deinit();
    return res;
}

void ms_asr_deinit(void)
{   
    decoder_deinit(DECODER_ALL);
    
    am_deinit();
    pp_deinit();
    return;
}

//解码器勾选，默认有 DECODER_RAW,DECODER_DIG, DECODER_KWS, DECODER_LVCSR
//cb设置为有值即为使能该decoder，否则跳过该decoder
int  ms_asr_decoder_cfg(int decoder_type, decoder_cb_t decoder_cb, void* decoder_args, int decoder_argc)
{
    if(decoder_cb != NULL) {
        decoder_init(decoder_type, decoder_cb, decoder_args, decoder_argc);
    } else {
        decoder_deinit(decoder_type);
    }
}

//每次clear会重置内部buf状态，和decoder状态
//但是不释放资源，以便下次start的时候快速加载功能
void ms_asr_clear(void)
{
    pp_clear();
    decoder_clear();
    return;
}

int ms_asr_set_dev(int device_type, char* device_name)
{
    asr_device_t* asr_dev = NULL;
    pp_deinit();
    //初始化设备
    if(device_type == DEVICE_PCM) {
        asr_dev = &dev_pcm;        
    } else if(device_type == DEVICE_MIC) {
        asr_dev = &dev_mic;   
    } else if(device_type == DEVICE_WAV) {
        asr_dev = &dev_wav;  
    } else {
        printf("error device type %d\n", device_type);
        return -1;
    }
    //初始化preprocess, pp为am提供数据
    int res = pp_init(asr_dev, device_name);
    if(res != 0) {
        printf("pp_init error!\n");
        return -1;
    }
    decoder_clear();
    return 0;
}

//运行计算frames帧, 返回实际运行的帧数
int ms_asr_run(int frames)
{
    int i = 0;
    uint8_t* mel_buf;
    pnyp_t* pnyp_list;
    for(i= 0; i<frames; i++) {
        mel_buf=pp_get();
        if(mel_buf==NULL) break;
        pnyp_list = am_run(mel_buf);
        if(pnyp_list==NULL) break;
        decoder_run(pnyp_list);
    }
    return i;
}

//run一帧对应的ms数
int ms_asr_get_frame_time(void)
{
    return (int)(asrp.wav_core_len*1000/AUDIO_RATE);
}

int ms_asr_kws_reg_similar(char* pny, char** similar_pnys, int similar_cnt)
{
    return decoder_kws_reg_similar(pny, similar_pnys, similar_cnt);
}

void ms_asr_get_am_vocab(char** vocab, int* cnt)
{
    *vocab = (char*) am_vocab;
    *cnt = asrp.vocab_cnt;
    return;
}

void ms_asr_wfst_run(pnyp_t* pnyp_list)
{
    decoder_run(pnyp_list);
    return;
}


























