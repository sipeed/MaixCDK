#include "producer.h"
#include "ms_asr_tbl.h"
#include "math.h"
#include <unistd.h>

extern int ms_asr_dbg_flag;

/**********************producer_mat private*********************************/
static float _producer_mat_LogLikelihood(void* _producer, int frame, int index)
{
    producer_t* producer = (producer_t*)_producer;
    producer_mat_private_t* pdata = (producer_mat_private_t*)(producer->pdata);
    float* prob_buf = pdata->prob_buf;
    int mat_w = pdata->mat_w;
    int mat_h = pdata->mat_h;
    return prob_buf[frame*mat_w+index];
}

static int _producer_mat_IsLastFrame(void* _producer, int frame)
{
    producer_t* producer = (producer_t*)_producer;
    producer_mat_private_t* pdata = (producer_mat_private_t*)(producer->pdata);
    int mat_h = pdata->mat_h;
    return (frame == mat_h-1)?1:0;
}

static int _producer_mat_NumFramesReady(void* _producer)
{
    producer_t* producer = (producer_t*)_producer;
    producer_mat_private_t* pdata = (producer_mat_private_t*)(producer->pdata);
    int mat_h = pdata->mat_h;
    return mat_h;
}

static int _producer_mat_NumIndices(void* _producer)
{
    producer_t* producer = (producer_t*)_producer;
    producer_mat_private_t* pdata = (producer_mat_private_t*)(producer->pdata);
    int mat_w = pdata->mat_w;
    return mat_w;
}


/**********************producer_mat public*********************************/

int producer_mat_init(producer_t* producer, char* mat_name, int w, int h)
{
    producer->LogLikelihood  = _producer_mat_LogLikelihood;
    producer->IsLastFrame    = _producer_mat_IsLastFrame;
    producer->NumFramesReady = _producer_mat_NumFramesReady;
    producer->NumIndices     = _producer_mat_NumIndices;
    producer_mat_private_t* pdata = (producer_mat_private_t*)malloc(sizeof(producer_mat_private_t));
    if(pdata == NULL) {
        printf("alloc data err!\n");
        return -1;
    }
    producer->pdata         = (void*)pdata;
    pdata->mat_w = w;
    pdata->mat_h = h;
    
    FILE* fp = fopen(mat_name, "r");
    if(fp == NULL) {
        printf("file %s open failed!\n", mat_name);
        goto free_private;
    }
    pdata->prob_buf = (float*)malloc(sizeof(float)*w*h);
    if(pdata->prob_buf == NULL) {
        printf("prob_buf alloc failed!\n");
        goto close_fp;
    }
    int cnt = fread (pdata->prob_buf, 1, sizeof(float)*w*h, fp);

    //打印mat里逐帧的最大值路径
    for(int j=0; j<h; j++) {
        int max = -1000000;
        int maxi= -1;
        for(int i=0; i<w; i++) {
            if(pdata->prob_buf[j*w+i]>max) {
                max = pdata->prob_buf[j*w+i];
                maxi = i;
            }
        }
        printf("%d, ", maxi);
    }
    printf("\n%d\n", cnt);
    
    fclose(fp);
    return 0;
    
close_fp:
    fclose(fp);

free_private:
    free(pdata);
    
    return -1;
}

void producer_mat_deinit(producer_t* producer)
{
    producer_mat_private_t* pdata = (producer_mat_private_t*)(producer->pdata);
    float* prob_buf = pdata->prob_buf;
    free(prob_buf);
    free(pdata);
    return;
}

/**********************producer_am private*********************************/
static float _producer_am_LogLikelihood(void* _producer, int frame, int index)
{
    producer_t* producer = (producer_t*)_producer;
    producer_am_private_t* pdata = (producer_am_private_t*)(producer->pdata);
    float* prob_buf = pdata->prob_buf;
    int mat_w = pdata->phone_n;
    int mat_h = pdata->frame_n;
    return prob_buf[frame*mat_w+index];
}

static int _producer_am_IsLastFrame(void* _producer, int frame)
{
    producer_t* producer = (producer_t*)_producer;
    producer_am_private_t* pdata = (producer_am_private_t*)(producer->pdata);
    int mat_h = pdata->frame_n;
    return (frame == mat_h-1)?1:0;
}

static int _producer_am_NumFramesReady(void* _producer)
{
    producer_t* producer = (producer_t*)_producer;
    producer_am_private_t* pdata = (producer_am_private_t*)(producer->pdata);
    int mat_h = pdata->frame_n;
    return mat_h;
}

static int _producer_am_NumIndices(void* _producer)
{
    producer_t* producer = (producer_t*)_producer;
    producer_am_private_t* pdata = (producer_am_private_t*)(producer->pdata);
    int mat_w = pdata->phone_n;
    return mat_w;
}

//返回当前格解析出来的pny格数
#define SLIENT_GATE (0.85)
int _gen_prob_from_pnyp(pnyp_t* pnyp_list, float* prob_buf, producer_am_private_t* pdata)
{
    int blank_idx = pdata->pny_n-1;
    int beam      = pdata->beam;
    int phone_n   = pdata->phone_n;
    float bg_prob = pdata->bg_prob;
    float* _prob_buf = prob_buf;
    
    for(int i = 0; i < phone_n; i++) {
        _prob_buf[i] = bg_prob; //背景全填充相同的背景概率值，这里直接填充2倍，理论上不会越界
    }
    //模型的pny idx 转 lg的pny idx
    pnyp_t* pnyp = pnyp_list+0;
    if((pnyp->idx == blank_idx) && (pnyp->p > SLIENT_GATE)) {
        return 0;    //blank 跳过
    }
    if(ms_asr_dbg_flag&DBG_LVCSR)printf("    ");
    for(int i = 0; i < beam; i++) { 
        pnyp = pnyp_list+i;
        uint16_t lm_idx =  am2lm[pnyp->idx];
        if(lm_idx == 1) continue; //去掉sil
        if(pnyp->p==0) continue;  //防止inf
        _prob_buf[lm_idx] = log(pnyp->p)*pdata->scale;
        if(ms_asr_dbg_flag&DBG_LVCSR)printf("%7s:%-2.1f;", lm_tbl[lm_idx], -1.0*log(pnyp->p)*pdata->scale);
    }
    if(ms_asr_dbg_flag&DBG_LVCSR)printf("\n");
    return 1;
}

/*
typedef struct
{
	uint32_t idx;  //pny的下标
	float p;
}pnyp_t;
*/
// assume beam <= 20
static pnyp_t prev_pnyps[20];
static pnyp_t merge_pnyps[40];
static pnyp_t pend_pnyps[20];
/*
pre cur out
0   0   cur
0   1   pend(blank)
1   0   pre
1   1   merge
*/
static int float_comp_down(const void*p1, const void*p2) {
	//<0, 元素1排在元素2之前；即降序
	//return ((*(pnyp_t*)p2).p-(*(pnyp_t*)p1).p)*10000; //使用乘法速度不高，使用下面的trick加速
	float tmp = ((*(pnyp_t*)p2).p-(*(pnyp_t*)p1).p);
	return *(int*)(&tmp);  
	//强转为int，并利用float32最高位定义与int32一致的原理，返回int
}
pnyp_t* _do_merge_frame(pnyp_t* prev, pnyp_t* cur, int beam)
{
    //step1: merge
    memcpy(&merge_pnyps[0], prev, sizeof(pnyp_t)*beam);
    memcpy(&merge_pnyps[beam], cur, sizeof(pnyp_t)*beam);
    int merge_i=1;
    for(int scan_i=1; scan_i<beam*2; scan_i++) {
        int merge_flag = 0;
        for(int i = 0; i<merge_i; i++){
            if(merge_pnyps[i].idx == merge_pnyps[scan_i].idx){
                merge_pnyps[i].p += merge_pnyps[scan_i].p;
                merge_flag = 1;
                break;
            }
        }
        if(merge_flag == 0){
            merge_pnyps[merge_i].idx = merge_pnyps[scan_i].idx;
            merge_pnyps[merge_i].p   = merge_pnyps[scan_i].p;
            merge_i += 1;
        }
        /*printf("scan_i=%d, merge_i=%d\n", scan_i, merge_i);
        for(int i=0; i<merge_i; i++){
            printf("%2d: %7s:%.3f;", i, am_vocab[merge_pnyps[i].idx], merge_pnyps[i].p);
        }
        printf("\n");*/
    }
    memset(&merge_pnyps[merge_i], 0, sizeof(pnyp_t)*(2*beam-merge_i));
    /*printf("\n");
    for(int i=0; i<merge_i; i++){
        printf("%2d: %7s:%.3f;", i, am_vocab[merge_pnyps[i].idx], merge_pnyps[i].p);
    }
    printf("\n");*/
    //step2: sort
    qsort(merge_pnyps, merge_i, sizeof(pnyp_t), float_comp_down);
    /*for(int i=0; i<merge_i; i++){
        printf("%2d: %7s:%.3f;", i, am_vocab[merge_pnyps[i].idx], merge_pnyps[i].p);
    }
    printf("\n");*/

    return merge_pnyps;
}

#define FRAME_BLANK(x) ((x)->idx == (asrp.vocab_cnt-1) && (x)->p > SLIENT_GATE)
pnyp_t* _merge_frame(pnyp_t* pnyp_list, producer_am_private_t* pdata)
{
    int blank_idx = pdata->pny_n-1;
    int beam      = pdata->beam;
    int phone_n   = pdata->phone_n;
    float bg_prob = pdata->bg_prob;

    pnyp_t* prev_top = &(prev_pnyps[0]);
    pnyp_t* res; 
    if(FRAME_BLANK(prev_top) && FRAME_BLANK(pnyp_list)){//printf("cur_pnyps!\n");
        res = pnyp_list;
        memcpy(prev_pnyps, pnyp_list, sizeof(pnyp_t)*beam);
    } else if(FRAME_BLANK(prev_top) && (!FRAME_BLANK(pnyp_list))){//printf("pend_pnyps!\n");
        res = pend_pnyps;
        memcpy(prev_pnyps, pnyp_list, sizeof(pnyp_t)*beam);
    } else if((!FRAME_BLANK(prev_top)) && (FRAME_BLANK(pnyp_list))){//printf("prev_pnyps!\n");
        memcpy(merge_pnyps, prev_pnyps, sizeof(pnyp_t)*beam);
        res = merge_pnyps;  //借用 merge_pnys 存储之前的帧来返回
        memcpy(prev_pnyps, pnyp_list, sizeof(pnyp_t)*beam);
    } else if((!FRAME_BLANK(prev_top)) && (!FRAME_BLANK(pnyp_list))){//printf("Merge!\n");
        res = _do_merge_frame(prev_pnyps, pnyp_list, beam);
        memcpy(prev_pnyps, pend_pnyps, sizeof(pnyp_t)*beam);    //merge后当前帧就清空
    }
    return res;
}


/**********************producer_am private*********************************/
int producer_am_init(producer_t* producer, int pny_n, int beam, int phone_n, int max_frames, float bg_prob, float scale)
{
    producer->LogLikelihood  = _producer_am_LogLikelihood;
    producer->IsLastFrame    = _producer_am_IsLastFrame;
    producer->NumFramesReady = _producer_am_NumFramesReady;
    producer->NumIndices     = _producer_am_NumIndices;
    producer_am_private_t* pdata = (producer_am_private_t*)malloc(sizeof(producer_am_private_t));
    if(pdata == NULL) {
        printf("alloc data err!\n");
        return -1;
    }
    pdata->prob_buf = malloc(max_frames*phone_n*sizeof(float)); 
    if(pdata->prob_buf == NULL) {
        printf("prob_buf alloc failed!\n");
        goto free_pdata;
    }

    prev_pnyps[0].idx=asrp.vocab_cnt-1;
    prev_pnyps[0].p = 1.0;
    pend_pnyps[0].idx=asrp.vocab_cnt-1;
    pend_pnyps[0].p = 1.0;
    
    pdata->pny_n = pny_n;
    pdata->beam = beam;
    pdata->phone_n = phone_n;
    pdata->frame_n = 0;
    pdata->max_frames = max_frames;
    pdata->bg_prob = bg_prob;
    pdata->scale = scale;
    producer->pdata = pdata;
    
    return 0;
    
free_pdata:
    free(pdata);
    return -1;
}

void producer_am_deinit(producer_t* producer)
{
    producer_am_private_t* pdata = (producer_am_private_t*)(producer->pdata);
    float* prob_buf = pdata->prob_buf;
    free(prob_buf);
    free(pdata);
    return;
}

/*
===================================
T=0048 ====:     1 shi   : 0.820;   407 _     : 0.071;    13 si    : 0.027;
T=0049 ====:   407 _     : 1.000;     0 lv    : 0.000;     1 shi   : 0.000;
T=0050 ====:   407 _     : 1.000;     0 lv    : 0.000;     1 shi   : 0.000;
T=0051 ====:   407 _     : 1.000;     0 lv    : 0.000;     1 shi   : 0.000;
T=0052 ====:   322 che   : 0.498;    75 zuo   : 0.114;    79 ce    : 0.114;
T=0053 ====:   407 _     : 1.000;     0 lv    : 0.000;     1 shi   : 0.000;
T=0054 ====:   407 _     : 0.996;    10 de    : 0.004;     0 lv    : 0.000;
T=0055 ====:   407 _     : 1.000;     0 lv    : 0.000;     1 shi   : 0.000;
####
*/

static int sil_counter = 0;
#define SIL_CNT_GATE1 2  //短停顿1s 
#define SIL_CNT_GATE2 4  //长停顿2s
//assume beam <= 20


int producer_am_push(producer_t* producer, pnyp_t* pnyp_list, int frames)
{   //每个时刻保留beam个，总共frames个时刻
    producer_am_private_t* pdata = (producer_am_private_t*)(producer->pdata);
    float* prob_buf = pdata->prob_buf;
    if(frames > pdata->max_frames) {
        printf("push too many frames! %d>%d\n", frames*2, pdata->max_frames);
        return -1;
    }
    
    float*  _prob_buf  = prob_buf; // + pdata->vocab_n*t;
    int phone_cnt = 0;
    int phone_i;

    //从当前frames时刻的pnyp结果，生成decoder所需的phone_prob矩阵
    for(int t = 0; t < frames; t++) {
        pnyp_t* _pnyp_list = pnyp_list + pdata->beam*t;
        pnyp_t* _pnyp_list1;
        _pnyp_list1 = _merge_frame(_pnyp_list, pdata);    //合并相邻两帧结果
        //_pnyp_list1 = _pnyp_list;
        phone_i = _gen_prob_from_pnyp(_pnyp_list1, _prob_buf, pdata);   //am prob -> lm prob
        phone_cnt += phone_i;
        _prob_buf += (pdata->phone_n*phone_i);
    }
    pdata->frame_n = phone_cnt;
    //连续2帧（1s）无声音则sil
    if(phone_cnt == 0) {
        sil_counter += 1;
        if(sil_counter == SIL_CNT_GATE1) {          //短停顿，逗号
            return 1;
        } else if(sil_counter == SIL_CNT_GATE2) {   //长停顿，句号
            return 2;
        }
    } else {
        sil_counter = 0;    //reset counter
    }
    
    return 0;
}


