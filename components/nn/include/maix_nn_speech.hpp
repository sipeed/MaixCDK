/**
 * @author 916BGAI
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.9.18: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_nn_F.hpp"

#include "speech/ms_asr.h"
#include <iostream>
#include <cstring>
#include <signal.h>

namespace maix::nn
{

static std::function<void(std::vector<std::pair<int, float>>, int)> raw_callback;
static std::function<void(char*, int)> digit_callback;
static std::function<void(std::vector<float>, int)> kws_callback;
static std::function<void(std::pair<char*, char*>, int)> lvcsr_callback;

/**
 * @brief speech device
 * @maixpy maix.nn.SpeechDevice
 */
enum class SpeechDevice {
    DEVICE_NONE = -1,
    DEVICE_PCM,
    DEVICE_MIC,
    DEVICE_WAV,
};

/**
 * @brief speech decoder type
 * @maixpy maix.nn.SpeechDecoder
 */
enum class SpeechDecoder {
    DECODER_RAW = 1,
    DECODER_DIG = 2,
    DECODER_LVCSR = 4,
    DECODER_KWS = 8,
    DECODER_ALL = 65535,
};

#ifdef PLATFORM_MAIXCAM

    static bool _is_skip_frames {false};

    /**
     * Speech
     * @maixpy maix.nn.Speech
     */
    class Speech
    {
    public:
        /**
         * Construct a new Speech object
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.Speech.__init__
         * @maixcdk maix.nn.Speech.Speech
         */
        Speech(const string &model = "")
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

        ~Speech()
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

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.Speech.load
        */
        err::Err load(const string &model)
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

        /**
         * Init the ASR library and select the type and name of the audio device.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @throw If am model is not loaded, will throw err::ERR_NOT_IMPL.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.init
         */
        err::Err init(nn::SpeechDevice dev_type, const string &device_name = "")
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

        /**
         * Reset the device, usually used for PCM/WAV recognition,
         * such as identifying the next WAV file.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.devive
         */
        err::Err devive(nn::SpeechDevice dev_type, const string &device_name)
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

        /**
         * Deinit the decoder.
         * @param decoder decoder type want to deinit
         * can choose between DECODER_RAW, DECODER_DIG, DECODER_LVCSR, DECODER_KWS or DECODER_ALL.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @maixpy maix.nn.Speech.dec_deinit
         */
        void dec_deinit(nn::SpeechDecoder decoder)
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

        /**
         * Init raw decoder, it will output the prediction results of the original AM.
         * @param callback raw decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.raw
         */
        err::Err raw(std::function<void(std::vector<std::pair<int, float>>, int)> callback)
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

        /**
         * Get raw decoder status
         * @return bool, raw decoder status
         * @maixpy maix.nn.Speech.raw
         */
        bool raw() { return _decoder_raw; }

        /**
         * Init digit decoder, it will output the Chinese digit recognition results within the last 4 seconds.
         * @param blank If it exceeds this value, insert a '_' in the output result to indicate idle mute.
         * @param callback digit decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.digit
         */
        err::Err digit(int blank, std::function<void(char*, int)> callback) 
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

        /**
         * Get digit decoder status
         * @return bool, digit decoder status
         * @maixpy maix.nn.Speech.digit
         */
        bool digit() { return _decoder_dig; }

        /**
         * Init kws decoder, it will output a probability list of all registered keywords in the latest frame,
         * users can set their own thresholds for wake-up.
         * @param kw_tbl Keyword list, filled in with spaces separated by pinyin, for example: xiao3 ai4 tong2 xue2
         * @param kw_gate kw_gate, keyword probability gate table, the number should be the same as kw_tbl
         * @param auto_similar Whether to perform automatic homophone processing,
         * setting it to true will automatically calculate the probability by using pinyin with different tones as homophones 
         * @param callback digit decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.kws
         */
        err::Err kws(std::vector<string> kw_tbl, std::vector<float> kw_gate, std::function<void(std::vector<float>, int)> callback, bool auto_similar = true) 
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

        /**
         * Get kws decoder status
         * @return bool, kws decoder status
         * @maixpy maix.nn.Speech.kws
         */
        bool kws() { return _decoder_kws; }

        /**
         * Init lvcsr decoder, it will output continuous speech recognition results (less than 1024 Chinese characters).
         * @param sfst_name Sfst file path.
         * @param sym_name Sym file path (output symbol table).
         * @param phones_txt Path to phones.bin (pinyin table).
         * @param words_txt Path to words.bin (dictionary table).
         * @param callback lvcsr decoder user callback.
         * @param beam The beam size for WFST search is set to 8 by default, and it is recommended to be between 3 and 9.
         * The larger the size, the larger the search space, and the more accurate but slower the search.
         * @param bg_prob The absolute value of the natural logarithm of the default probability value for background pinyin
         * outside of BEAM-CNT is set to 10 by default.
         * @param scale acoustics_cost = log(pny_prob)*scale.
         * @param mmap use mmap to load the WFST decoding image,
         * If set to true, the beam should be less than 5.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.lvcsr
         */
        err::Err lvcsr(const string &sfst_name, const string &sym_name,
                       const string &phones_txt, const string &words_txt, 
                       std::function<void(std::pair<char*, char*>, int)> callback,
                       float beam = 8, float bg_prob = 10, float scale = 0.5, bool mmap = false) 
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

        /**
         * Get lvcsr decoder status
         * @return bool, lvcsr decoder status
         * @maixpy maix.nn.Speech.lvcsr
         */
        bool lvcsr() { return _decoder_lvcsr; }

        /**
         * Run speech recognition, user can run 1 frame at a time and do other processing after running,
         * or it can run continuously within a thread and be stopped by an external thread.
         * @param frame The number of frames per run.
         * @return int type, return actual number of frames in the run.
         * @maixpy maix.nn.Speech.run
         */
        int run(int frame)
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

        /**
         * Reset internal cache operation
         * @maixpy maix.nn.Speech.clear
         */
        void clear()
        {
            ms_asr_clear();
        }

        /**
         * Get the time of one frame.
         * @return int type, return the time of one frame.
         * @maixpy maix.nn.Speech.frame_time
         */
        int frame_time()
        {
            return ms_asr_get_frame_time();
        }

        /**
         * Manually register mute words, and each pinyin can register up to 10 homophones,
         * please note that using this interface to register homophones will overwrite,
         * the homophone table automatically generated in the "automatic homophone processing" feature.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.similar
         */
        err::Err similar(const string &pny, std::vector<std::string> similar_pnys)
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

        /**
         * Run some frames and drop, this can be used to avoid
         * incorrect recognition results when switching decoders.
         * @param num number of frames to run and drop
         * @maixpy maix.nn.Speech.skip_frames
        */
        void skip_frames(int num) {
            _is_skip_frames = true;
            this->run(num);
            _is_skip_frames = false;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.Speech.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.Speech.scale
         */
        std::vector<float> scale;

        /**
         * get device type
         * @return nn::SpeechDevice type, see SpeechDevice of this module
         * @maixpy maix.nn.Speech.dev_type
        */
        nn::SpeechDevice dev_type() { return _dev_type; }

        static void _signal_handle(int signal)
        {
            switch (signal) {
            case SIGINT:
                maix::app::set_exit_flag(true);
            break;
            default: break;
            }
        }

    private:
        nn::NN *_model;
        std::string _model_path = "";
        std::map<string, string> _extra_info;
        image::Size _input_size;
        std::vector<nn::LayerInfo> _inputs;
        nn::SpeechDevice _dev_type = SpeechDevice::DEVICE_NONE;
        bool _decoder_raw = false;
        bool _decoder_dig = false;
        bool _decoder_kws = false;
        bool _decoder_lvcsr = false;

        std::function<void(std::vector<std::pair<int, float>>, int)> _raw_callback;
        std::function<void(char*, int)> _digit_callback;
        std::function<void(std::vector<float>, int)> _kws_callback;
        std::function<void(std::pair<char*, char*>, int)> _lvcsr_callback;

        void deinit()
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

        static void split0(std::vector<std::string> &items, const std::string &s, const std::string &delimiter)
        {
            items.clear();
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                items.push_back(token);
            }

            items.push_back(s.substr(pos_start));
        }

        static std::vector<std::string> split(const std::string &s, const std::string &delimiter)
        {
            std::vector<std::string> tokens;
            split0(tokens, s, delimiter);
            return tokens;
        }
    };

} // namespace maix::nn
#endif

#ifdef PLATFORM_LINUX
    /**
     * Speech
     * @maixpy maix.nn.Speech
     */
    class Speech
    {
    public:
        /**
         * Construct a new Speech object
         * @param model model path, default empty, you can load model later by load function.
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.Speech.__init__
         * @maixcdk maix.nn.Speech.Speech
         */
        Speech(const string &model = "")
        {

        }

        ~Speech()
        {

        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.Speech.load
        */
        err::Err load(const string &model)
        {
            return err::ERR_NONE;
        }

        /**
         * Init the ASR library and select the type and name of the audio device.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @throw If am model is not loaded, will throw err::ERR_NOT_IMPL.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.init
         */
        err::Err init(nn::SpeechDevice dev_type, const string &device_name = "")
        {
            return err::ERR_NONE;
        }

        /**
         * Reset the device, usually used for PCM/WAV recognition,
         * such as identifying the next WAV file.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.devive
         */
        err::Err devive(nn::SpeechDevice dev_type, const string &device_name)
        {
            return err::ERR_NONE;
        }

        /**
         * Deinit the decoder.
         * @param decoder decoder type want to deinit
         * can choose between DECODER_RAW, DECODER_DIG, DECODER_LVCSR, DECODER_KWS or DECODER_ALL.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @maixpy maix.nn.Speech.dec_deinit
         */
        void dec_deinit(nn::SpeechDecoder decoder)
        {
            
        }

        /**
         * Init raw decoder, it will output the prediction results of the original AM.
         * @param callback raw decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.raw
         */
        err::Err raw(std::function<void(std::vector<std::pair<int, float>>, int)> callback)
        {
            return err::ERR_NONE;
        }

        /**
         * Get raw decoder status
         * @return bool, raw decoder status
         * @maixpy maix.nn.Speech.raw
         */
        bool raw() { return _decoder_raw; }

        /**
         * Init digit decoder, it will output the Chinese digit recognition results within the last 4 seconds.
         * @param blank If it exceeds this value, insert a '_' in the output result to indicate idle mute.
         * @param callback digit decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.digit
         */
        err::Err digit(int blank, std::function<void(char*, int)> callback) 
        {
            return err::ERR_NONE;
        }

        /**
         * Get digit decoder status
         * @return bool, digit decoder status
         * @maixpy maix.nn.Speech.digit
         */
        bool digit() { return _decoder_dig; }

        /**
         * Init kws decoder, it will output a probability list of all registered keywords in the latest frame,
         * users can set their own thresholds for wake-up.
         * @param kw_tbl Keyword list, filled in with spaces separated by pinyin, for example: xiao3 ai4 tong2 xue2
         * @param kw_gate kw_gate, keyword probability gate table, the number should be the same as kw_tbl
         * @param auto_similar Whether to perform automatic homophone processing,
         * setting it to true will automatically calculate the probability by using pinyin with different tones as homophones 
         * @param callback digit decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.kws
         */
        err::Err kws(std::vector<string> kw_tbl, std::vector<float> kw_gate, std::function<void(std::vector<float>, int)> callback, bool auto_similar = true) 
        {
            return err::ERR_NONE;
        }

        /**
         * Get kws decoder status
         * @return bool, kws decoder status
         * @maixpy maix.nn.Speech.kws
         */
        bool kws() { return _decoder_kws; }

        /**
         * Init lvcsr decoder, it will output continuous speech recognition results (less than 1024 Chinese characters).
         * @param sfst_name Sfst file path.
         * @param sym_name Sym file path (output symbol table).
         * @param phones_txt Path to phones.bin (pinyin table).
         * @param words_txt Path to words.bin (dictionary table).
         * @param callback lvcsr decoder user callback.
         * @param beam The beam size for WFST search is set to 8 by default, and it is recommended to be between 3 and 9.
         * The larger the size, the larger the search space, and the more accurate but slower the search.
         * @param bg_prob The absolute value of the natural logarithm of the default probability value for background pinyin
         * outside of BEAM-CNT is set to 10 by default.
         * @param scale acoustics_cost = log(pny_prob)*scale.
         * @param mmap use mmap to load the WFST decoding image,
         * If set to true, the beam should be less than 5.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.lvcsr
         */
        err::Err lvcsr(const string &sfst_name, const string &sym_name,
                       const string &phones_txt, const string &words_txt, 
                       std::function<void(std::pair<char*, char*>, int)> callback,
                       float beam = 8, float bg_prob = 10, float scale = 0.5, bool mmap = false) 
        {
            return err::ERR_NONE;
        }

        /**
         * Get lvcsr decoder status
         * @return bool, lvcsr decoder status
         * @maixpy maix.nn.Speech.lvcsr
         */
        bool lvcsr() { return _decoder_lvcsr; }

        /**
         * Run speech recognition, user can run 1 frame at a time and do other processing after running,
         * or it can run continuously within a thread and be stopped by an external thread.
         * @param frame The number of frames per run.
         * @return int type, return actual number of frames in the run.
         * @maixpy maix.nn.Speech.run
         */
        int run(int frame)
        {
            return 0;
        }

        /**
         * Reset internal cache operation
         * @maixpy maix.nn.Speech.clear
         */
        void clear()
        {

        }

        /**
         * Get the time of one frame.
         * @return int type, return the time of one frame.
         * @maixpy maix.nn.Speech.frame_time
         */
        int frame_time()
        {
            return 0;
        }

        /**
         * Manually register mute words, and each pinyin can register up to 10 homophones,
         * please note that using this interface to register homophones will overwrite,
         * the homophone table automatically generated in the "automatic homophone processing" feature.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.similar
         */
        err::Err similar(const string &pny, std::vector<std::string> similar_pnys)
        {
            return err::ERR_NONE;
        }

        /**
         * Run some frames and drop, this can be used to avoid
         * incorrect recognition results when switching decoders.
         * @param num number of frames to run and drop
         * @maixpy maix.nn.Speech.skip_frames
        */
        void skip_frames(int num) {
            return;
        }

    public:
        /**
         * Get mean value, list type
         * @maixpy maix.nn.Speech.mean
         */
        std::vector<float> mean;

        /**
         * Get scale value, list type
         * @maixpy maix.nn.Speech.scale
         */
        std::vector<float> scale;

        /**
         * get device type
         * @return nn::SpeechDevice type, see SpeechDevice of this module
         * @maixpy maix.nn.Speech.dev_type
        */
        nn::SpeechDevice dev_type() { return _dev_type; }

    private:
        nn::NN *_model;
        std::string _model_path = "";
        std::map<string, string> _extra_info;
        image::Size _input_size;
        std::vector<nn::LayerInfo> _inputs;
        nn::SpeechDevice _dev_type = SpeechDevice::DEVICE_NONE;
        bool _decoder_raw = false;
        bool _decoder_dig = false;
        bool _decoder_kws = false;
        bool _decoder_lvcsr = false;

        void deinit()
        {
            return ;
        }

        static void digit_callback_wrapper(void* data, int cnt) {

        }

        static void kws_callback_wrapper(void* data, int cnt) {

        }

        static void raw_callback_wrapper(void* data, int cnt) {

        }

        static void lvcsr_callback_wrapper(void* data, int cnt) {

        }

        static void split0(std::vector<std::string> &items, const std::string &s, const std::string &delimiter)
        {
            items.clear();
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                items.push_back(token);
            }

            items.push_back(s.substr(pos_start));
        }

        static std::vector<std::string> split(const std::string &s, const std::string &delimiter)
        {
            std::vector<std::string> tokens;
            split0(tokens, s, delimiter);
            return tokens;
        }
    };

} // namespace maix::nn
#endif
