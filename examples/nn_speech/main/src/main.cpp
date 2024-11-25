
#include "maix_basic.hpp"
#include "maix_nn_speech.hpp"
#include "main.h"

using namespace maix;

void digit_callback(char* data, int len)
{
    char* digit_data = (char*) data;
    log::info("digit_output: %s", digit_data);
    return;
}

void kws_callback(std::vector<float> data, int len)
{
    int maxp = -1;
    for(int i=0; i<len; i++){
        log::info0("\tkw%d: %.3f;", i, data[i]);
        if(data[i] > maxp){
            maxp = data[i];
        }
    }
    log::info0("\n");
    return;
}

void lvcsr_callback(std::pair<char*, char*> data, int len)
{
    log::info("PNYS: %s", data.second);
    log::info("HANS: %s", data.first);
    return;
}

int _main(int argc, char* argv[])
{
    nn::Speech speech("/root/models/am_3332_192_int8.mud");

    speech.init(nn::SpeechDevice::DEVICE_MIC);                      // use mic device
    // speech.init(nn::SpeechDevice::DEVICE_WAV, "test.wav");       // use wav file
    // speech.init(nn::SpeechDevice::DEVICE_PCM, "test.pcm");       // use pcm file

    speech.digit(640, digit_callback);      // init digit decoder

    /* init kws decoder */
    // std::vector<string> kw_tbl = {
    //     "xiao3 ai4 tong2 xue2",
    //     "tian1 mao1 jing1 ling2",
    //     "tian1 qi4 zen3 me yang4",
    // };
    // std::vector<float> kw_gate = {0.1, 0.1, 0.1};
    // speech.kws(kw_tbl, kw_gate, kws_callback);
    // std::vector<std::string> similar_char = {"xin1", "ting1", "jin1"};
    // speech.similar("jing1", similar_char);

    /* init lvcsr decoder */
    // const std::string lm_path = "/root/models/lmS/";
    // speech.lvcsr(lm_path + "lg_6m.sfst", lm_path + "lg_6m.sym", 
    //              lm_path + "phones.bin", lm_path + "words_utf.bin", 
    //              lvcsr_callback);

    while(!app::need_exit())
    {
        int frames = speech.run(1);
        if(frames < 1) {
            log::info("run out");
            break;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


