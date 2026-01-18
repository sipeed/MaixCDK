#include "pp.h"
#include "stft.h"
#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include "ms_asr_utils.h"
#include "string.h"
#include <sys/time.h>
#include <time.h>

extern asr_param_t asrp;
extern int ms_asr_dbg_flag;
asr_device_t* asr_dev;

int16_t* l_wav_buf;     //[WAV_BUF_LEN];             //每帧wav缓冲
int16_t* l_wav_buf_agc;    //AGC后的缓冲
uint8_t* l_mel_buf;     //[MODEL_IN_LEN*MEL_N];     //每帧计算出来的mel谱，即模型输入

/*****************************PP private**********************************/
static void dump_cur_frame(void)
{
    printf("#######\r\n");
    for(int i=0; i*128<asrp.wav_buf_len; i++) {
        printf("%5d,", l_wav_buf[i*128]);
        if(i%32==31)printf("\r\n");
    }
    printf("\r\n");
    return;
}

static int16_t  g_d0=0;		//存储前一时刻的wav数据
void cal_mel_buf(int16_t* wav_buf, uint8_t* mel_buf, int len)
{
	//一次调用计算4格，每格间距8ms
    for (int i = 0; i < len ; i++) //asrp.model_in_len
    {	//这里计算一个滑动窗，512点，实际频点256点
		//计算得到80维mel fbank
		//l_mel_features即不停在刷新的声谱图
        mel_compute(wav_buf+FFT_MOVE*i, mel_buf+MEL_N*i, g_d0);
		g_d0 = wav_buf[FFT_MOVE*(i+1) - 1];
    }
	return;
}

/*****************************PP public**********************************/
FILE* pp_fw = NULL;
int  pp_init(asr_device_t* _asr_dev, char* device_name)
{
    asr_dev = _asr_dev;
    int res = asr_dev->init(device_name);
    if(res != 0) return res;
    
    l_wav_buf = (int16_t*)malloc(asrp.wav_buf_len*sizeof(int16_t));
    l_wav_buf_agc = (int16_t*)malloc(asrp.wav_buf_len*sizeof(int16_t));
    l_mel_buf = (uint8_t*)malloc(asrp.model_in_len*MEL_N*sizeof(uint8_t));
    if(l_wav_buf==NULL || l_mel_buf==NULL || l_wav_buf_agc==NULL) {
        res = -1;
        goto free_device;
    }
    pp_clear();
    enable_agc(asrp.agc);

    if(ms_asr_dbg_flag&DBG_MIC) {
        pp_fw = fopen("mic.pcm", "w");   
    }
    return 0;
    
free_device:
    asr_dev->deinit();
    return res;
}

void pp_deinit(void)
{
    if (l_mel_buf != NULL) {
        free(l_mel_buf);
        l_mel_buf = NULL;
    }
    if (l_wav_buf != NULL) {
        free(l_wav_buf);
        l_wav_buf = NULL;
    }
    if (l_wav_buf_agc != NULL) {
        free(l_wav_buf_agc);
        l_wav_buf_agc = NULL;
    }

    if (asr_dev != NULL) {
        asr_dev->deinit();
    }
    if((ms_asr_dbg_flag&DBG_MICRAW)  && pp_fw) {
        fclose(pp_fw);   
    }
    return;
}

extern int stft_pcm_idx;
void  pp_clear(void)
{
    asr_dev->clear();
    memset(l_wav_buf, 0, asrp.wav_buf_len*sizeof(int16_t)); 
    memset(l_mel_buf, 0, asrp.model_in_len*MEL_N*sizeof(uint8_t));
    if((ms_asr_dbg_flag&DBG_MICRAW) && pp_fw) {
        fclose(pp_fw);   
        pp_fw = fopen("mic.pcm", "w");   
    }
    stft_pcm_idx = 0;
    return;
}

uint8_t* pp_get(void)
{
    DBG_TIME_INIT();
    //上一帧前移
    memmove(l_wav_buf, l_wav_buf+asrp.wav_core_len, sizeof(int16_t)*asrp.wav_rest_len);
    //读入下一帧数据
    if(asr_dev->read(l_wav_buf+asrp.wav_rest_len, asrp.wav_core_len)==0) { //!=asrp.wav_core_len
        printf("read device error or runout!\n");
        return NULL;
    }
    //AGC操作
    DBG_TIME_START();
    memcpy(l_wav_buf_agc, l_wav_buf, asrp.wav_buf_len*sizeof(int16_t));
    agcProcess(l_wav_buf_agc, 16000, FFT_N, kAgcModeAdaptiveDigital);
    if(ms_asr_dbg_flag&DBGT_PP)DBG_TIME("AGC"); 
    
    //dump_cur_frame();
    //上一帧mel buf前移
    memmove(l_mel_buf, l_mel_buf+asrp.model_core_len*8*MEL_N, sizeof(uint8_t)*(asrp.strip_l+asrp.strip_r)*8*MEL_N);
    cal_mel_buf(l_wav_buf_agc+(asrp.strip_l+asrp.strip_r)*8*FFT_MOVE, \
        l_mel_buf+(asrp.strip_l+asrp.strip_r)*8*MEL_N, asrp.model_core_len*8);
    if(ms_asr_dbg_flag&DBGT_PP)DBG_TIME("CAL_MEL"); 
    
    return l_mel_buf;
}

