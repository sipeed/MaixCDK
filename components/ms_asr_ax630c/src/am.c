#include "am.h"
#include "ms_asr.h"
#include "ms_asr_cfg.h"
#include "ms_asr_utils.h"
#include <sys/time.h>
#include "math.h"

#include "string.h"

//以下抽象函数由宏定义选择编译实际的平台所使用的

extern int am_init_model(char* name, int* quant_type);
extern void am_deinit_model(void);
extern int am_infer_model(uint8_t* mel_buf, void** result, uint32_t* size);
extern int am_infer_clean(void);

extern int ms_asr_dbg_flag;

//[MODEL_OUT_LEN*BEAM_CNT];//每帧的计算结果(已排序，每格保留前5个结果)
static pnyp_t*  l_pnyp_res;    
static int l_out_type = QUANT_NONE; //0 float, 1 int8, 2 uint8
/***************************** AM private **********************************/

int uint8_comp_down(const void*p1, const void*p2) {
	//<0, 元素1排在元素2之前；即降序
	int tmp = (((pnyp_t*)p2)->p) - (((pnyp_t*)p1)->p);
	return tmp;  
}

int float_comp_down(const void*p1, const void*p2) {
	//<0, 元素1排在元素2之前；即降序
	//return ((*(pnyp_t*)p2).p-(*(pnyp_t*)p1).p)*10000; //使用乘法速度不高，使用下面的trick加速
	float tmp = ((*(pnyp_t*)p2).p-(*(pnyp_t*)p1).p);
	return *(int*)(&tmp);  
	//强转为int，并利用float32最高位定义与int32一致的原理，返回int
}

static int tick = 0;
static void dump_frame_res(pnyp_t* res)
{
    printf("===================================\n");
    for(int t=0; t<asrp.model_out_len-0; t++) {
        pnyp_t* pp = res+BEAM_CNT*t;
        if(1){ //(pp[0].idx != (asrp.vocab_cnt-1)) && (pp[0].p > 1)) {
            if(t<asrp.strip_l || t >= asrp.model_out_len-asrp.strip_r) {
                printf("T=%04d ----:", tick);
            } else {
                printf("T=%04d ====:", tick);
                tick += 1;  //全局时刻+1
            }
            for(int i=0; i < 3; i++) { //BEAM_CNT
                printf("  %4d %-6s: %.3f;", pp[i].idx, am_vocab[pp[i].idx], ((float)(pp[i].p)));
            }
        }
        printf("\n");
    }
    printf("####\n");
    return;
}

static void _softmax(int8_t* result, float* res_fp, int tlen, int vocab_cnt)
{
    for(int t=0; t<tlen; t++) {
        int max_val = -1000;
        float sum=0;
        int8_t* p = result+vocab_cnt*t;
        float* pdst = res_fp+vocab_cnt*t;
        for(int i=0; i<vocab_cnt; i++) {
            if(p[i]>max_val) max_val=p[i];
        }
#if 1
        for(int i=0; i<vocab_cnt; i++) {
            pdst[i] = p[i]-max_val;
        }
        for(int i=0; i<vocab_cnt; i++) {
            pdst[i] = exp(pdst[i]);
            sum += pdst[i];
        }
        for(int i=0; i<vocab_cnt; i++) {
            pdst[i] = pdst[i]/sum;
        }
#else
        for(int i=0; i<vocab_cnt; i++) {
            pdst[i] = p[i]-max_val;
            if(pdst[i] < -1) pdst[i] = 0;
            else pdst[i] = exp(pdst[i]);
            sum += pdst[i];
        }
        for(int i=0; i<vocab_cnt; i++) {
            pdst[i] = pdst[i]/sum;
        }
#endif      
    }
    return;
}

static void quickSort_int8(int8_t* array, int start, int end, int* indexArr, int Nmax) {
    if(end<=start) return ;
    int low = start;
    int high = end;
    int key = array[low];
    int curIndex = indexArr[low];
    while (low < high) {
        while (key >= array[high] && low < high) high--;
        array[low] = array[high];
        indexArr[low] = indexArr[high];
        while (key <= array[low] && low < high) low++;
        array[high] = array[low];
        indexArr[high] = indexArr[low];
    }
    array[high] = key;
    indexArr[high] = curIndex;

    if (low - 1 > start) quickSort_int8(array, start, low - 1, indexArr, Nmax);
    if (high + 1 < Nmax) quickSort_int8(array, high + 1, end, indexArr, Nmax);
    return ;
}

static void quickSort_float(float* array, int start, int end, int* indexArr, int Nmax) {
    if(end<=start) return ;
    int low = start;
    int high = end;
    float key = array[low];
    int curIndex = indexArr[low];
    while (low < high) {
        while (key >= array[high] && low < high) high--;
        array[low] = array[high];
        indexArr[low] = indexArr[high];
        while (key <= array[low] && low < high) low++;
        array[high] = array[low];
        indexArr[high] = indexArr[low];
    }
    array[high] = key;
    indexArr[high] = curIndex;

    if (low - 1 > start) quickSort_float(array, start, low - 1, indexArr, Nmax);
    if (high + 1 < Nmax) quickSort_float(array, high + 1, end, indexArr, Nmax);
    return ;
}

static int idx_buf[MAX_VOCAB_CNT];
//int8 输出，则需要后处理手工进行softmax（因为softmax直接int8的话误差太大）
static void _post_sort_int8(int8_t* result, int tlen, int vocab_cnt, pnyp_t* res)
{
    float pdst[BEAM_CNT];
    for(int t=0; t<tlen; t++) {
        int8_t* p = result+vocab_cnt*t;
        /*printf("t=%d, vocab_cnt=%d\n", t, vocab_cnt);
        for(int i=0; i<asrp.vocab_cnt; i++){
            printf("%4d,", p[i]);
        }
        printf("\n");
        */
        for(int i=0; i<asrp.vocab_cnt; i++){
            idx_buf[i] = i;
        }
        quickSort_int8(p, 0, asrp.vocab_cnt-1, idx_buf, BEAM_CNT);

        int8_t pmax = p[0];
        float sum = 0;
        for(int i=0; i<BEAM_CNT; i++) {
            pdst[i] = p[i]-pmax;
            pdst[i] = exp(pdst[i]);
            sum += pdst[i];
        }

        for(int i=0; i<BEAM_CNT; i++) {
            pdst[i] = pdst[i]/sum;
        }

        for(int i=0; i<BEAM_CNT; i++) {
            res[BEAM_CNT*t+i].idx = idx_buf[i];
            res[BEAM_CNT*t+i].p = pdst[i] ;
        }
    }
    return;
}

//float输出，则无需再softmax
static void _post_sort_fp(float* result, int tlen, int vocab_cnt, pnyp_t* res)
{
    for(int t=0; t<tlen; t++) {
        float* p = result+vocab_cnt*t;
        /*printf("t=%d, vocab_cnt=%d\n", t, vocab_cnt);
        for(int i=0; i<asrp.vocab_cnt; i++){
            
        }*/
        for(int i=0; i<asrp.vocab_cnt; i++){
            idx_buf[i] = i;
        }
        quickSort_float(p, 0, asrp.vocab_cnt-1, idx_buf, BEAM_CNT);     

        for(int i=0; i<BEAM_CNT; i++) {
            res[BEAM_CNT*t+i].idx = idx_buf[i];
            res[BEAM_CNT*t+i].p = p[i] ;
            //printf("%d:idx=%04d,p=%.3f;  ", i, idx_buf[i], p[i]);
        }
        //printf("\n");
    }
    return;
}


/***************************** AM public **********************************/
int am_init(char* model_name, int phone_type)
{   //结果缓存
    l_pnyp_res= (pnyp_t*)malloc(asrp.model_out_len*BEAM_CNT*sizeof(pnyp_t));
    if(l_pnyp_res == NULL) return -1;
    //选择对应的am_voab
    switch(phone_type){
        case CN_PNY:
            am_vocab = (char**)&am_pny_vocab;
            asrp.vocab_cnt = sizeof(am_pny_vocab)/sizeof(char*);
            break;
        case CN_PNYTONE:
            am_vocab = (char**)&am_pnytone_vocab;
            asrp.vocab_cnt = sizeof(am_pnytone_vocab)/sizeof(char*);
            break;
        default:
            printf("phone type %d not support yet!\n", phone_type);
            free(l_pnyp_res);
            return -1;
            break;
    }
    printf("load am vocab ok, vocab_cnt=%d\n", asrp.vocab_cnt);
    int res = am_init_model(model_name, &l_out_type);
    if(res != 0) {
        free(l_pnyp_res);
        return -1;
    }
    return 0;
}

void am_deinit(void)
{
    am_deinit_model();
    if (l_pnyp_res != NULL) {
        free(l_pnyp_res);
        l_pnyp_res = NULL;
    }
    return;
}


pnyp_t* am_run(uint8_t* mel_buf)
{
    int ret = 0;
    DBG_TIME_INIT();DBG_TIME_START();
    void *result;
    uint32_t size;
    ret = am_infer_model(mel_buf, (void**)&result, &size);  //result need HWC layout
    if(ms_asr_dbg_flag&DBGT_AM)DBG_TIME("AM Model");
    if (ret == 0) {
        if(l_out_type == QUANT_NONE){
            if(ms_asr_dbg_flag&DBG_STRIP)_post_sort_fp(result, asrp.model_out_len,asrp.vocab_cnt, l_pnyp_res);
            else _post_sort_fp(((float*)result)+asrp.vocab_cnt*asrp.strip_l, asrp.model_core_len,asrp.vocab_cnt, l_pnyp_res); //only decode core
        }else if(l_out_type == QUANT_INT8){
            if(ms_asr_dbg_flag&DBG_STRIP)_post_sort_int8(result, asrp.model_out_len,asrp.vocab_cnt, l_pnyp_res);
            else _post_sort_int8(((int8_t*)result)+asrp.vocab_cnt*asrp.strip_l, asrp.model_core_len,asrp.vocab_cnt, l_pnyp_res); //only decode core
        }else {
            printf("Wrong output type %d!\n", l_out_type);
            goto out1;
        }
        if(ms_asr_dbg_flag&DBG_STRIP){
            dump_frame_res(l_pnyp_res);
        }
        if(ms_asr_dbg_flag&DBGT_AM)DBG_TIME("decode_result");
    } else {
        goto out1;
    }
    
    ret = am_infer_clean();
    if(ret != 0) {
        goto out1;
    }

    pnyp_t* res;
    if(ms_asr_dbg_flag&DBG_STRIP) res = l_pnyp_res+BEAM_CNT*asrp.strip_l; //调试strip的时候要手工加回去
    else res = l_pnyp_res;
    return res;
out1:
    return NULL;
}


