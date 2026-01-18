#include "device.h"
#include <sys/stat.h>  
#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include "string.h"

static int pcm_points;         //total pcm points
static int16_t* l_pcm_buf;     //暂存pcm文件数据的buf
static int pcm_i;

/*
//处理流程
1. 缓冲区共 512+384=896ms, 每次滑动 256ms
2. 每次计算出 64格 mel，即 64H*80W
3. 64x80 输入模型，计算得 8x408的输出
4. 去头去尾，取中间4格内容为当前帧预测结果
5. 缓冲区向前移动256ms，即4K点，再补充入新数据，再继续
*/
/***************************** PCM private **********************************/
static int get_file_size(char* filename)
{
    struct stat statbuf;
    stat(filename,&statbuf);
    int size=statbuf.st_size;
    return size;
}

static void dump_all_pcm(void)
{
    for(int i=0; i*128<pcm_points; i++) {
        printf("%5d,", l_pcm_buf[i*128]);
        if(i%32==31)printf("\r\n");
    }
    printf("\r\n");
    return;
}



/***************************** PCM public **********************************/

//初始化音频设备/文件
int  pcm_init(char* pcm_name)
{
    l_pcm_buf = NULL;
    pcm_points = 0;
	FILE* fp = fopen(pcm_name, "r");
    if(fp == NULL) {
        printf("%s open failed!\n", pcm_name);
        return -1;
    }
    int file_size = get_file_size(pcm_name);
    if(file_size <= 0) {
        printf("get file size error\n");
        return -2;
    }
    l_pcm_buf = (int16_t*)malloc(file_size);
    if(l_pcm_buf == NULL) {
        printf("pcm buf alloc failed\n");
        return -3;
    }
    
	fread(l_pcm_buf, file_size, 1, fp);	
	fclose(fp);
    pcm_points = file_size/sizeof(int16_t);
    printf("## pcm init ok! total %d points\n", pcm_points);
    pcm_i = 0;
	return 0;
}    

//返回读取到的数量    
int  pcm_read(int16_t* data_buf, int len)
{
    int cnt = 0;
    if(pcm_i+len <= pcm_points) {
        cnt = len;
        memcpy(data_buf, l_pcm_buf+pcm_i, sizeof(int16_t)*cnt);
    } else {
        cnt = pcm_points-pcm_i;
        if(cnt<=0){
            cnt = 0;
        } else {
            memcpy(data_buf, l_pcm_buf+pcm_i, sizeof(int16_t)*cnt);
            memset(data_buf+cnt, 0, sizeof(int16_t)*(len-cnt)); //末尾填0
        }
    }
    pcm_i += cnt;
    return cnt;
}    
    
//清缓存
void pcm_clear(void)
{
    pcm_i = 0;
    return;
}    

//清理    
void pcm_deinit(void)
{
    free(l_pcm_buf);
    return;
}


asr_device_t dev_pcm ={
    .init   = pcm_init,
    .read   = pcm_read,
    .clear  = pcm_clear,
    .deinit = pcm_deinit,
};
