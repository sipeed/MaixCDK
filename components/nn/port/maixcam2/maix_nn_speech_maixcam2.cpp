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
        (void)model;
		err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented.");
	}

    Speech::~Speech()
    {
        err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented.");
    }

    err::Err Speech::load(const string &model)
    {
        (void)model;
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    err::Err Speech::init(nn::SpeechDevice dev_type, const string &device_name)
    {
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    err::Err Speech::devive(nn::SpeechDevice dev_type, const string &device_name)
    {
        (void)dev_type;
        (void)device_name;
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    void Speech::dec_deinit(nn::SpeechDecoder decoder)
    {
        (void)decoder;
        err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented.");
    }

    err::Err Speech::raw(std::function<void(std::vector<std::pair<int, float>>, int)> callback)
    {
        (void)callback;
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    bool Speech::raw() { return _decoder_raw; }

    err::Err Speech::digit(int blank, std::function<void(char*, int)> callback) 
    {
        (void)blank;
        (void)callback;
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    bool Speech::digit() { return _decoder_dig; }

    err::Err Speech::kws(std::vector<string> kw_tbl, std::vector<float> kw_gate, std::function<void(std::vector<float>, int)> callback, bool auto_similar) 
    {
        (void)kw_tbl;
        (void)kw_gate;
        (void)callback;
        (void)auto_similar;
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    bool Speech::kws() { return _decoder_kws; }

    err::Err Speech::lvcsr(const string &sfst_name, const string &sym_name,
                    const string &phones_txt, const string &words_txt, 
                    std::function<void(std::pair<char*, char*>, int)> callback,
                    float beam, float bg_prob, float scale, bool mmap) 
    {
        (void)sfst_name;
        (void)sym_name;
        (void)phones_txt;
        (void)words_txt;
        (void)callback;
        (void)beam;
        (void)bg_prob;
        (void)scale;
        (void)mmap;
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    bool Speech::lvcsr() { return _decoder_lvcsr; }

    int Speech::run(int frame)
    {
        (void)frame;
        log::error("This function is not implemented.");
        return 0;
    }

    void Speech::clear()
    {
        err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented.");
    }

    int Speech::frame_time()
    {
        log::error("This function is not implemented.");
        return 0;
    }

    err::Err Speech::similar(const string &pny, std::vector<std::string> similar_pnys)
    {
        log::error("This function is not implemented.");
        return err::ERR_NOT_IMPL;
    }

    void Speech::skip_frames(int num) {
        (void)num;
        err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented.");
    }

    void Speech::deinit()
    {
        err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented.");
    }
}