#include "decoder_raw.h"
#include "ms_asr.h"
#include "ms_asr_cfg.h"

static decoder_cb_t decoder_cb = NULL;

int  decoder_raw_init(decoder_cb_t _decoder_cb, size_t* decoder_args, int decoder_argc)
{
    decoder_cb = _decoder_cb;
    return 0;
}

void decoder_raw_deinit(void)
{
    decoder_cb = NULL;
    return;
}

void decoder_raw_run(pnyp_t* pnyp_buf)
{
    if(decoder_cb != NULL) {
        decoder_cb(pnyp_buf, asrp.model_core_len);
    };
    return;
}

void decoder_raw_clear(void)
{
    if(decoder_cb != NULL) {
        //do nothing
    }
    return;
}