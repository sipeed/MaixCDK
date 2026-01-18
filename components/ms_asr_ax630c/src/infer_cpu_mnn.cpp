#include "ms_asr.h"


#include "am.h"
#include "ms_asr_cfg.h"
#include "ms_asr_utils.h"
#include <sys/time.h>
#include "math.h"
#include "string.h"

#include <stdio.h>
#include <MNN/ImageProcess.hpp>
#include <MNN/Interpreter.hpp>
#define MNN_OPEN_TIME_TRACE
#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <vector>
#include <MNN/AutoTime.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "imageHelper/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "imageHelper/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "imageHelper/stb_image_resize.h"

using namespace MNN;
using namespace MNN::CV;

#define DBG(format, ...) //DBG(format, __VA_ARGS__)

extern asr_param_t asrp;
extern uint8_t am_test_data[512][80];

typedef struct{
    char* name;
    std::shared_ptr<Interpreter> net;
    Session* session;
    Tensor* inputTensor;
    Tensor* outputTensor;
    Tensor* inputTensorUser;
    Tensor* outputTensorUser;
    int in_h;
    int in_w;
    int in_ch;
    int out_h;
    int out_w;
    int out_ch;
} model_t;

#define DEC_KEY     "Shenzhen Sipeed Technology Co. Ltd."  //注意不满32字节，需使用空格补齐到32字节
#define DEC_SIZE    32

static model_t mdl_am;
static float* l_output = NULL;

static void read_mdl(char* name, uint8_t** mdl_buf, int* mdl_size)
{
    FILE* fp = fopen(name, "r");
    if(!fp) {
        printf("open %s failed\n", name);
        return ;
    }
    fseek(fp,0L,SEEK_END);
    size_t m_size = ftell(fp);
    uint8_t* m_buf = (uint8_t*)malloc(m_size);
    if(m_buf==NULL) {
        printf("alloc buf %ld bytes failed\n", (long int)m_size);
        return ;
    }
    fseek(fp,0L,SEEK_SET);
    size_t read_cnt = fread(m_buf, 1, m_size, fp);
    if (read_cnt != m_size) {
        printf("fread %ld bytes but get %ld bytes\n", (long int)m_size, (long int)read_cnt);
        return ;
    }
    fclose(fp);
    *mdl_size = (size_t)m_size;
    //判读magic
    char* magic = (char*)m_buf;
    if(magic[2]==0 && magic[3]==0) { //未加密的mnn
        *mdl_buf = m_buf;
    } else {
        //解密文件
        uint8_t* key = (uint8_t*)DEC_KEY;
        for(int i=0; i+DEC_SIZE<=m_size; i+=DEC_SIZE){
            for(int j=0; j<DEC_SIZE; j++){
                m_buf[i+j] ^= key[j];
            }
            key = m_buf+i; //key 更新为已解密的上一段内容
        }
        *mdl_buf = m_buf;
        /*FILE* fw = fopen("dump.sm", "w");
        if(fw == NULL) {
            free(m_buf);
            return;
        }
        fwrite(*mdl_buf, 1, *mdl_size, fw);
        fclose(fw); */
        //0x53 0x68 0x65 0x6e
    }
    return;
}


static int parse_mdl(model_t* model, char* name, char* in_name, char* out_name)
{
    uint8_t* mdl_buf;
    int mdl_size;
    read_mdl(name, &mdl_buf, &mdl_size);
    if(mdl_buf == NULL || mdl_size ==0){
        printf("parse mdl err!\n");
        return -1;
    }
    //std::shared_ptr<Interpreter> net(Interpreter::createFromFile(name)); //createFromBuffer(const void* buffer, size_t size);
    std::shared_ptr<Interpreter> net(Interpreter::createFromBuffer((const void*)mdl_buf, mdl_size));
    
    ScheduleConfig config;
    config.type  = MNN_FORWARD_CPU;
    config.numThread = 1;

    MNN::BackendConfig backendConfig;
    backendConfig.precision = MNN::BackendConfig::Precision_High;
    config.backendConfig = &backendConfig;
    
    Session* session = net->createSession(config);

    Tensor* inputTensor  = net->getSessionInput(session, in_name);
    Tensor* outputTensor = net->getSessionOutput(session, out_name);

    net->resizeTensor(inputTensor, {1, 1, asrp.model_in_len, MEL_N});
    net->resizeSession(session);

    Tensor inputTensorUser(inputTensor, Tensor::DimensionType::TENSORFLOW);
    Tensor outputTensorUser(outputTensor, outputTensor->getDimensionType());   

    int netInputHeight = inputTensorUser.height();
    int netInputWidth  = inputTensorUser.width();
    int netInputChannel = inputTensorUser.channel();
    int netOutputHeight = outputTensorUser.height();
    int netOutputWidth  = outputTensorUser.width();
    int netOutputChannel = outputTensorUser.channel();
    
    model->name = name;
    model->net = net;
    model->session = session;
    model->inputTensor = inputTensor;
    model->outputTensor = outputTensor;
    model->inputTensorUser = NULL;  //inputTensorUser 是局部变量，只能在外面填充
    model->outputTensorUser = NULL;
    model->in_h = netInputHeight;
    model->in_w = netInputWidth;
    model->in_ch = netInputChannel;
    model->out_h = netOutputHeight;
    model->out_w = netOutputWidth;
    model->out_ch = netOutputChannel;

    DBG("model %s, in (%d, %d, %d), out (%d, %d, %d)\n", name, \
        model->in_w, model->in_h, model->in_ch, model->out_w, model->out_h, model->out_ch);
    mdl_am.net->releaseModel();
    if(mdl_buf) free(mdl_buf);
   
    return 0;
}

static int run_model(model_t* model, uint8_t* pic, int picw, int pich, float** output, uint32_t* size)
{
    Tensor inputTensorUser(model->inputTensor, Tensor::DimensionType::TENSORFLOW);
    Tensor outputTensorUser(model->outputTensor, model->outputTensor->getDimensionType());  
    //预处理图片
    {
        //归一化图片
        ImageProcess::Config config;
        config.filterType = BILINEAR;
        float mean[3]     = {0, 0, 0};  
        float normals[3]  = {1/255.0, 1/255.0, 1/255.0};
        ::memcpy(config.mean, mean, sizeof(mean));
        ::memcpy(config.normal, normals, sizeof(normals));
        config.sourceFormat = GRAY;
        config.destFormat = GRAY;
        std::shared_ptr<ImageProcess> pretreat(ImageProcess::create(config));
        pretreat->convert((const uint8_t*)pic, (int)picw, (int)pich, 0, &inputTensorUser);
    }
    //run
    {
        //AUTOTIME;
        model->inputTensor->copyFromHostTensor(&inputTensorUser);
        model->net->runSession(model->session);
        model->outputTensor->copyToHostTensor(&outputTensorUser);
    }
    //get result
    {
        DBG("output size:%d\n", outputTensorUser.elementSize());
        auto type = outputTensorUser.getType();
        auto _size = outputTensorUser.elementSize();
        std::vector<std::pair<int, float>> tempValues(_size);
        if (type.code == halide_type_float) {
            float* _output = outputTensorUser.host<float>();
            memcpy(l_output, _output, _size*sizeof(float));
            *output = l_output;
            *size = (uint32_t)_size;
            return 0;
        }
        /*if (type.code == halide_type_uint && type.bytes() == 1) {
            auto values = outputTensorUser.host<uint8_t>();
            for (int i = 0; i < size; ++i) {
                tempValues[i] = std::make_pair(i, values[i]);
            }
        }*/
        else{
            DBG("wrong type! only support float!\n");
            return -1;
        }
        
    }
}

static int _am_init_model(char* model_name, int* quant_type){
    int res = parse_mdl(&mdl_am, model_name, NULL, NULL); //单输入单输出时无需填写节点名
    if(res != 0) return res;
    Tensor outputTensorUser(mdl_am.outputTensor, mdl_am.outputTensor->getDimensionType());  
    auto type = outputTensorUser.getType();
    if (type.code == halide_type_float) {
        *quant_type = QUANT_NONE;
        l_output = (float*)malloc(sizeof(float)*asrp.vocab_cnt*asrp.model_out_len);
        if(l_output == NULL){
            printf("am output alloc error!\n");
            return -1;
        }
    } else if(type.code == halide_type_uint && type.bytes() == 1) {
        printf("not support cpu uint8 yet!\n");
        return -1;
    }
    return 0;
}

static void _am_deinit_model(void){
    mdl_am.net->releaseSession(mdl_am.session);
    //mdl_am.net->releaseModel();
    //delete &mdl_am.net;
    free(l_output);
    return;
}

static int _am_infer_model(uint8_t* mel_buf, void** result, uint32_t* size) {
    return run_model(&mdl_am, mel_buf, MEL_N, asrp.model_in_len, (float**)result, size);
}

static int _am_infer_clean(void){
    return 0;
}


extern "C"{
int am_init_model(char* model_name, int* quant_type){
    return _am_init_model(model_name, quant_type);
}

void am_deinit_model(void){
    return _am_deinit_model();
}

int am_infer_model(uint8_t* mel_buf, void** result, uint32_t* size) {
    return _am_infer_model(mel_buf, result, size);
}
int am_infer_clean(void){
    return _am_infer_clean();
}


/********************** test code *************************/
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

static void _post_sort_fp(float* result, int tlen, int vocab_cnt)
{
    int idx_buf[1250];
    float pdst[BEAM_CNT];
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
            printf("%d:idx=%04d,p=%.3f;  ", i, idx_buf[i], p[i]);
        }
        printf("\n");
    }
    return;
}

void am_infer_test(void){
    float* result;
    uint32_t size;
    DBG_TIME_INIT();DBG_TIME_START();
    _am_infer_model((uint8_t*)am_test_data, (void**)&result, &size);
    DBG_TIME("AM Model");
    printf("size=%d, refer to %d\n", size, asrp.model_out_len*asrp.vocab_cnt);
    printf("layout type: %d\n", mdl_am.outputTensor->getDimensionType());
    /*for(int i=0; i<asrp.model_out_len; i++){
        printf("[");
        for(int j=0; j<asrp.vocab_cnt; j++){
            printf("%.3f,", result[i*asrp.vocab_cnt+j]);
        }
        printf("]\n");
    }
    printf("\n");*/
    _post_sort_fp(result, 24, 1250);

    return;
}

/*
T=006: idx=  8 (yi1   )	prob=0.600
T=010: idx= 38 (dian3 )	prob=0.971
T=015: idx= 78 (er4   )	prob=0.815
T=020: idx= 15 (shen2 )	prob=0.444
T=027: idx=145 (si4   )	prob=0.334
T=030: idx= 60 (wu3   )	prob=0.768
T=034: idx=161 (liu4  )	prob=0.918
T=041: idx=113 (qi1   )	prob=0.255
T=045: idx=137 (ba1   )	prob=0.649
T=050: idx=105 (jiu3  )	prob=0.949
*/

}


