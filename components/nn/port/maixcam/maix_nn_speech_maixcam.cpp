/**
 * @author 916BGAI
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.12.23: Add framework, create this file.
 */

#include "maix_nn_speech.hpp"

namespace maix::nn
{
    static bool _is_skip_frames {false};

    static void _signal_handle(int signal)
    {
        switch (signal) {
        case SIGINT:
            maix::app::set_exit_flag(true);
        break;
        default: break;
        }
    }

    static void digit_callback_wrapper(void* data, int cnt) {
        if (digit_callback && _is_skip_frames == false) {
            digit_callback(static_cast<char*>(data), cnt);
        }
    }

    static void kws_callback_wrapper(void* data, int cnt) {
        if (kws_callback && _is_skip_frames == false) {
            std::vector<float> kws_data;
            float* p = (float*) data;
            for(int i=0; i<cnt; i++){
                kws_data.push_back(p[i]);
            }
            kws_callback(kws_data, cnt);
        }
    }

    static void raw_callback_wrapper(void* data, int cnt) {
        if (raw_callback && _is_skip_frames == false) {
            std::vector<std::pair<int, float>> raw_data;
            pnyp_t* res = (pnyp_t*)data;
            for(int t=0; t<cnt; t++) {
                pnyp_t* pp = res+BEAM_CNT*t;
                raw_data.push_back({pp->idx, pp->p});
            }
            raw_callback(raw_data, cnt);
        }
    }

    static void lvcsr_callback_wrapper(void* data, int cnt) {
        if (lvcsr_callback && _is_skip_frames == false) {
            char* words = ((char**)data)[0];
            char* pnys = ((char**)data)[1];
            lvcsr_callback({words, pnys}, cnt);
        }
    }

    Speech::Speech(const string &model)
    {
		_model = nullptr;
        if (!model.empty())
        {
            err::Err e = load(model);
            if (e != err::ERR_NONE)
            {
                throw err::Exception(e, "load model failed");
            }
        }
        signal(SIGINT, _signal_handle);
	}

    Speech::~Speech()
    {
        if (_dev_type != SpeechDevice::DEVICE_NONE) {
            this->deinit();
        }

        if (_model)
        {
            delete _model;
            _model = nullptr;
        }
    }

    err::Err Speech::load(const string &model)
    {
        if (_model)
        {
            delete _model;
            _model = nullptr;
        }
        _model = new nn::NN(model);
        if (!_model)
        {
            return err::ERR_NO_MEM;
        }
        _extra_info = _model->extra_info();
        if (_extra_info.find("model_type") != _extra_info.end())
        {
            if (_extra_info["model_type"] != "speech")
            {
                log::error("model_type not match, expect 'speech', but got '%s'.", _extra_info["model_type"].c_str());
                return err::ERR_ARGS;
            }
        }
        else
        {
            log::error("model_type key not found.");
            return err::ERR_ARGS;
        }
        if (_extra_info.find("mean") != _extra_info.end())
        {
            std::string mean_str = _extra_info["mean"];
            std::vector<std::string> mean_strs = split(mean_str, ",");
            for (auto &it : mean_strs)
            {
                try
                {
                    this->mean.push_back(std::stof(it));
                }
                catch (std::exception &e)
                {
                    log::error("mean value error, should float.");
                    return err::ERR_ARGS;
                }
            }
        }
        else
        {
            log::error("mean key not found.");
            return err::ERR_ARGS;
        }
        if (_extra_info.find("scale") != _extra_info.end())
        {
            std::string scale_str = _extra_info["scale"];
            std::vector<std::string> scale_strs = split(scale_str, ",");
            for (auto &it : scale_strs)
            {
                try
                {
                    this->scale.push_back(std::stof(it));
                }
                catch (std::exception &e)
                {
                    log::error("scale value error, should float.");
                    return err::ERR_ARGS;
                }
            }
        }
        else
        {
            log::error("scale key not found.");
            return err::ERR_ARGS;
        }
        _inputs = _model->inputs_info();
        _input_size = image::Size(_inputs[0].shape[2], _inputs[0].shape[1]);
        _model_path = model;
        return err::ERR_NONE;
    }

    err::Err Speech::init(nn::SpeechDevice dev_type, const string &device_name)
    {
        string _device_name = device_name;

        if (_model_path == "") {
            log::error("please load am model first.");
            throw err::Exception(err::ERR_NOT_IMPL);
        }

        am_args_t am_args = {(char*)_model_path.c_str(), 192, 6, 6, CN_PNYTONE, 1};

        if (this->dev_type() != SpeechDevice::DEVICE_NONE) {
            log::error("device has been initialized, please use maix.nn.Speech.devive to reset devive.");
            return err::ERR_RUNTIME;
        }

        if((int)dev_type > 2) {
            log::error("not support device %d.", dev_type);
            throw err::Exception(err::ERR_NOT_IMPL);
        } else if (dev_type == SpeechDevice::DEVICE_MIC && device_name == "") {
            _dev_type = dev_type;
            _device_name = "hw:0,0";
        } else if (dev_type == SpeechDevice::DEVICE_PCM && device_name == "") {
            log::error("please enter the correct path to the PCM file.");
            return err::ERR_ARGS;
        } else if (dev_type == SpeechDevice::DEVICE_WAV && device_name == "") {
            log::error("please enter the correct path to the WAV file.");
            return err::ERR_ARGS;
        } else {
            _dev_type = dev_type;
            _device_name = device_name;
        }

        int ret = ms_asr_init((int)_dev_type, (char*)_device_name.c_str(), &am_args, 0);
        if(ret) {
            log::error("asr init error!");
            _dev_type = SpeechDevice::DEVICE_NONE;
            return err::ERR_NOT_IMPL;
        }

        return err::ERR_NONE;
    }

    err::Err Speech::devive(nn::SpeechDevice dev_type, const string &device_name)
    {
        if((int)dev_type > 2) {
            log::error("not support device %d.", dev_type);
            throw err::Exception(err::ERR_NOT_IMPL);
        } else {
            _dev_type = dev_type;
        }

        int ret = ms_asr_set_dev((int)_dev_type, (char*)device_name.c_str());
        if(ret) {
            log::error("set devive error!\n");
            _dev_type = SpeechDevice::DEVICE_NONE;
            return err::ERR_NOT_IMPL;
        }

        return err::ERR_NONE;
    }

    void Speech::dec_deinit(nn::SpeechDecoder decoder)
    {
        ms_asr_decoder_cfg((int)decoder, NULL , NULL, 0);
        switch (decoder)
        {
            case nn::SpeechDecoder::DECODER_RAW:
                _decoder_raw = false;
                break;
            case nn::SpeechDecoder::DECODER_DIG:
                _decoder_dig = false;
                break;
            case nn::SpeechDecoder::DECODER_LVCSR:
                _decoder_lvcsr = false;
                break;
            case nn::SpeechDecoder::DECODER_KWS:
                _decoder_kws = false;
                break;
            case nn::SpeechDecoder::DECODER_ALL:
                _decoder_raw = false;
                _decoder_dig = false;
                _decoder_lvcsr = false;
                _decoder_kws = false;
                break;
            default:
                log::error("not support decoder %d.", decoder);
                throw err::Exception(err::ERR_NOT_IMPL);
        }
    }

    err::Err Speech::raw(std::function<void(std::vector<std::pair<int, float>>, int)> callback)
    {
        if (this->dev_type() == SpeechDevice::DEVICE_NONE) {
            log::error("please init a type of audio device first.");
            return err::ERR_NOT_INIT;
        }

        _raw_callback = callback;
        int ret = ms_asr_decoder_cfg((int)nn::SpeechDecoder::DECODER_RAW, raw_callback_wrapper , NULL, 0);
        if (ret != 0) {
            log::error("raw decoder init error.");
            return err::ERR_RUNTIME;
        } else {
            _decoder_raw = true;
            return err::ERR_NONE;
        }
    }

    bool Speech::raw() { return _decoder_raw; }

    err::Err Speech::digit(int blank, std::function<void(char*, int)> callback) 
    {
        if (this->dev_type() == SpeechDevice::DEVICE_NONE) {
            log::error("please init a type of audio device first.");
            return err::ERR_NOT_INIT;
        }

        size_t decoder_args[10];
        decoder_args[0] = blank;

        _digit_callback = callback;

        int ret = ms_asr_decoder_cfg((int)nn::SpeechDecoder::DECODER_DIG, digit_callback_wrapper, &decoder_args, 1);
        if (ret != 0) {
            log::error("digit decoder init error.");
            return err::ERR_RUNTIME;
        } else {
            _decoder_dig = true;
            return err::ERR_NONE;
        }
    }

    bool Speech::digit() { return _decoder_dig; }

    err::Err Speech::kws(std::vector<string> kw_tbl, std::vector<float> kw_gate, std::function<void(std::vector<float>, int)> callback, bool auto_similar) 
    {
        if (this->dev_type() == SpeechDevice::DEVICE_NONE) {
            log::error("please init a type of audio device first.");
            return err::ERR_NOT_INIT;
        }

        if (kw_tbl.size() != kw_gate.size()) {
            log::error("kw_tbl num must equal to kw_gate num.");
            return err::ERR_ARGS;
        }

        size_t decoder_args[10];
        char** _kw_tbl = new char*[kw_tbl.size()];
        for (size_t i = 0; i < kw_tbl.size(); ++i) {
            _kw_tbl[i] = new char[kw_tbl[i].size() + 1];
            std::strcpy(_kw_tbl[i], kw_tbl[i].c_str());
        }
        float* _kw_gate = new float[kw_gate.size()];
        for (size_t i = 0; i < kw_gate.size(); ++i) {
            _kw_gate[i] = kw_gate[i];
        }

        decoder_args[0] = (size_t)_kw_tbl;
        decoder_args[1] = (size_t)_kw_gate;
        decoder_args[2] = kw_tbl.size();
        decoder_args[3] = auto_similar;
        _kws_callback = callback;
        int ret = ms_asr_decoder_cfg((int)nn::SpeechDecoder::DECODER_KWS, kws_callback_wrapper, &decoder_args, 3);

        delete[] _kw_gate;
        for (size_t i = 0; i < kw_tbl.size(); ++i) {
            delete[] _kw_tbl[i];
        }
        delete[] _kw_tbl;

        if (ret != 0) {
            log::error("kws decoder init error.");
            return err::ERR_RUNTIME;
        } else {
            _decoder_kws = true;
            return err::ERR_NONE;
        }
    }

    bool Speech::kws() { return _decoder_kws; }

    err::Err Speech::lvcsr(const string &sfst_name, const string &sym_name,
                    const string &phones_txt, const string &words_txt, 
                    std::function<void(std::pair<char*, char*>, int)> callback,
                    float beam, float bg_prob, float scale, bool mmap) 
    {
        if (this->dev_type() == SpeechDevice::DEVICE_NONE) {
            log::error("please init a type of audio device first.");
            return err::ERR_NOT_INIT;
        }

        size_t decoder_args[10];
        decoder_args[0] = (size_t)sfst_name.c_str();
        decoder_args[1] = (size_t)sym_name.c_str();
        decoder_args[2] = (size_t)phones_txt.c_str();
        decoder_args[3] = (size_t)words_txt.c_str();
        memcpy(&decoder_args[4], &(beam), sizeof(float));
        memcpy(&decoder_args[5], &(bg_prob), sizeof(float));
        memcpy(&decoder_args[6], &(scale), sizeof(float));
        decoder_args[7] = mmap;
        _lvcsr_callback = callback;

        int ret = ms_asr_decoder_cfg((int)nn::SpeechDecoder::DECODER_LVCSR, lvcsr_callback_wrapper, &decoder_args, 8);
        if (ret != 0) {
            log::error("lvcsr decoder init error.");
            return err::ERR_RUNTIME;
        } else {
            _decoder_lvcsr = true;
            return err::ERR_NONE;
        }
    }

    bool Speech::lvcsr() { return _decoder_lvcsr; }

    int Speech::run(int frame)
    {
        if (!(raw() || digit() || kws() || lvcsr())) {
            log::error("please init at least one decoder before running.");
            return 0;
        }

        raw_callback = this->_raw_callback;
        digit_callback = this->_digit_callback;
        kws_callback = this->_kws_callback;
        lvcsr_callback = this->_lvcsr_callback;

        int frames = ms_asr_run(frame);

        // Set it to nullptr, otherwise MaixPy cannot exit properly.
        raw_callback = nullptr;
        digit_callback = nullptr;
        kws_callback = nullptr;
        lvcsr_callback = nullptr;

        return frames;
    }

    void Speech::clear()
    {
        ms_asr_clear();
    }

    int Speech::frame_time()
    {
        return ms_asr_get_frame_time();
    }

    err::Err Speech::similar(const string &pny, std::vector<std::string> similar_pnys)
    {
        if (this->kws() != true) {
            log::error("please init kws decoder first.");
            return err::ERR_RUNTIME;
        }

        char** _similar_pnys = new char*[similar_pnys.size()];
        for (size_t i = 0; i < similar_pnys.size(); ++i) {
            _similar_pnys[i] = new char[similar_pnys[i].size() + 1];
            std::strcpy(_similar_pnys[i], similar_pnys[i].c_str());
        }

        int ret = ms_asr_kws_reg_similar((char*)pny.c_str(), _similar_pnys, similar_pnys.size());

        for (size_t i = 0; i < similar_pnys.size(); ++i) {
            delete[] _similar_pnys[i];
        }
        delete[] _similar_pnys;

        if (ret != 0) {
            log::error("set similar pny error.");
            return err::ERR_RUNTIME;
        } else {
            return err::ERR_NONE;
        }
    }

    void Speech::skip_frames(int num) {
        _is_skip_frames = true;
        this->run(num);
        _is_skip_frames = false;
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
        ms_asr_deinit();
        sys::register_default_signal_handle();
    }
}