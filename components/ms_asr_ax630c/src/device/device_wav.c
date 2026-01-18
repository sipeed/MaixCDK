#include "device.h"
#include <sys/stat.h>  
#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include "string.h"
#include "dr_wav.h"

static size_t wav_points;         //total wav points
static int16_t* l_wav_buf;     //暂存wav文件数据的buf
static int wav_i;

/*
//处理流程
1. 缓冲区共 512+384=896ms, 每次滑动 256ms
2. 每次计算出 64格 mel，即 64H*80W
3. 64x80 输入模型，计算得 8x408的输出
4. 去头去尾，取中间4格内容为当前帧预测结果
5. 缓冲区向前移动256ms，即4K点，再补充入新数据，再继续
*/
/***************************** wav private **********************************/
static void dump_all_wav(void)
{
    for(int i=0; i*128<wav_points; i++) {
        printf("%5d,", l_wav_buf[i*128]);
        if(i%32==31)printf("\r\n");
    }
    printf("\r\n");
    return;
}



/***************************** wav public **********************************/

//初始化音频设备/文件
int  wav_init(char* wav_name)
{
    l_wav_buf = NULL;
    wav_points = 0;

    //音频采样率
    uint32_t sampleRate = 0;
    unsigned int channels = 0;
    l_wav_buf = drwav_open_and_read_file_s16(wav_name, &channels, &sampleRate, (drwav_uint64*)&wav_points);
    //如果加载成功
    if (l_wav_buf == NULL || sampleRate != 16000 || channels!= 1)
    {
        printf("open wav failed or fs,ch error!\n");
        return -1;
    }
    //printf("Open %s ok, total %d points\n", wav_name, wav_points);
    wav_i = 0;
	return 0;
}    

//返回读取到的数量    
int  wav_read(int16_t* data_buf, int len)
{
    int cnt = 0;
    if(wav_i+len <= wav_points) {
        cnt = len;
        memcpy(data_buf, l_wav_buf+wav_i, sizeof(int16_t)*cnt);
    } else {
        cnt = wav_points-wav_i;
        if(cnt<=0){
            cnt = 0;
        } else {
            memcpy(data_buf, l_wav_buf+wav_i, sizeof(int16_t)*cnt);
            memset(data_buf+cnt, 0, sizeof(int16_t)*(len-cnt)); //末尾填0
        }
    }
    
    wav_i += cnt;
    return cnt;
}    
    
//清缓存
void wav_clear(void)
{
    wav_i = 0;
    return;
}    

//清理    
void wav_deinit(void)
{
    free(l_wav_buf);
    return;
}


asr_device_t dev_wav ={
    .init   = wav_init,
    .read   = wav_read,
    .clear  = wav_clear,
    .deinit = wav_deinit,
};
