/**
 * @author 916BGAI
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.12.23: Add framework, create this file.
 */

#include "maix_nn_speech.hpp"

namespace maix::nn
{
    Speech::Speech(const string &model)
    {

	}

    Speech::~Speech()
    {

    }

    err::Err Speech::load(const string &model)
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err Speech::init(nn::SpeechDevice dev_type, const string &device_name)
    {
        return err::ERR_NOT_IMPL;
    }

    err::Err Speech::devive(nn::SpeechDevice dev_type, const string &device_name)
    {
        return err::ERR_NOT_IMPL;
    }

    void Speech::dec_deinit(nn::SpeechDecoder decoder)
    {
        
    }

    err::Err Speech::raw(std::function<void(std::vector<std::pair<int, float>>, int)> callback)
    {
        return err::ERR_NOT_IMPL;
    }

    bool Speech::raw() { return _decoder_raw; }

    err::Err Speech::digit(int blank, std::function<void(char*, int)> callback) 
    {
        return err::ERR_NOT_IMPL;
    }

    bool Speech::digit() { return _decoder_dig; }

    err::Err Speech::kws(std::vector<string> kw_tbl, std::vector<float> kw_gate, std::function<void(std::vector<float>, int)> callback, bool auto_similar) 
    {
        return err::ERR_NOT_IMPL;
    }

    bool Speech::kws() { return _decoder_kws; }

    err::Err Speech::lvcsr(const string &sfst_name, const string &sym_name,
                    const string &phones_txt, const string &words_txt, 
                    std::function<void(std::pair<char*, char*>, int)> callback,
                    float beam, float bg_prob, float scale, bool mmap) 
    {
        return err::ERR_NOT_IMPL;
    }

    bool Speech::lvcsr() { return _decoder_lvcsr; }

    int Speech::run(int frame)
    {
        return -1;
    }

    void Speech::clear()
    {

    }

    int Speech::frame_time()
    {
        return -1;
    }

    err::Err Speech::similar(const string &pny, std::vector<std::string> similar_pnys)
    {
        return err::ERR_NOT_IMPL;
    }

    void Speech::skip_frames(int num) {
        return;
    }

    void Speech::deinit()
    {
        _dev_type = SpeechDevice::DEVICE_NONE;
        _decoder_raw = false;
        _decoder_dig = false;
        _decoder_lvcsr = false;
        _decoder_kws = false;
        _raw_callback = nullptr;
        _digit_callback = nullptr;
        _kws_callback = nullptr;
        _lvcsr_callback = nullptr;
    }
}

