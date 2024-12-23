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
    DECODER_RAW   = 1,
    DECODER_DIG   = 2,
    DECODER_LVCSR = 4,
    DECODER_KWS   = 8,
    DECODER_ALL   = 65535,
};

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
        Speech(const string &model = "");

        ~Speech();

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.Speech.load
        */
        err::Err load(const string &model);

        /**
         * Init the ASR library and select the type and name of the audio device.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @throw If am model is not loaded, will throw err::ERR_NOT_IMPL.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.init
         */
        err::Err init(nn::SpeechDevice dev_type, const string &device_name = "");

        /**
         * Reset the device, usually used for PCM/WAV recognition,
         * such as identifying the next WAV file.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.devive
         */
        err::Err devive(nn::SpeechDevice dev_type, const string &device_name);

        /**
         * Deinit the decoder.
         * @param decoder decoder type want to deinit
         * can choose between DECODER_RAW, DECODER_DIG, DECODER_LVCSR, DECODER_KWS or DECODER_ALL.
         * @throw If device is not supported, will throw err::ERR_NOT_IMPL.
         * @maixpy maix.nn.Speech.dec_deinit
         */
        void dec_deinit(nn::SpeechDecoder decoder);

        /**
         * Init raw decoder, it will output the prediction results of the original AM.
         * @param callback raw decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.raw
         */
        err::Err raw(std::function<void(std::vector<std::pair<int, float>>, int)> callback);

        /**
         * Get raw decoder status
         * @return bool, raw decoder status
         * @maixcdk maix.nn.Speech.raw
         */
        bool raw();

        /**
         * Init digit decoder, it will output the Chinese digit recognition results within the last 4 seconds.
         * @param blank If it exceeds this value, insert a '_' in the output result to indicate idle mute.
         * @param callback digit decoder user callback.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.digit
         */
        err::Err digit(int blank, std::function<void(char*, int)> callback);

        /**
         * Get digit decoder status
         * @return bool, digit decoder status
         * @maixcdk maix.nn.Speech.digit
         */
        bool digit();

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
        err::Err kws(std::vector<string> kw_tbl, std::vector<float> kw_gate, std::function<void(std::vector<float>, int)> callback, bool auto_similar = true);

        /**
         * Get kws decoder status
         * @return bool, kws decoder status
         * @maixcdk maix.nn.Speech.kws
         */
        bool kws();

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
                       float beam = 8, float bg_prob = 10, float scale = 0.5, bool mmap = false);

        /**
         * Get lvcsr decoder status
         * @return bool, lvcsr decoder status
         * @maixcdk maix.nn.Speech.lvcsr
         */
        bool lvcsr();

        /**
         * Run speech recognition, user can run 1 frame at a time and do other processing after running,
         * or it can run continuously within a thread and be stopped by an external thread.
         * @param frame The number of frames per run.
         * @return int type, return actual number of frames in the run.
         * @maixpy maix.nn.Speech.run
         */
        int run(int frame);

        /**
         * Reset internal cache operation
         * @maixpy maix.nn.Speech.clear
         */
        void clear();

        /**
         * Get the time of one frame.
         * @return int type, return the time of one frame.
         * @maixpy maix.nn.Speech.frame_time
         */
        int frame_time();

        /**
         * Manually register mute words, and each pinyin can register up to 10 homophones,
         * please note that using this interface to register homophones will overwrite,
         * the homophone table automatically generated in the "automatic homophone processing" feature.
         * @param dev_type device type want to detect, can choose between WAV, PCM, or MIC.
         * @param device_name device name want to detect, can choose a WAV file, a PCM file, or a MIC device name.
         * @return err::Err type, if init success, return err::ERR_NONE
         * @maixpy maix.nn.Speech.similar
         */
        err::Err similar(const string &pny, std::vector<std::string> similar_pnys);

        /**
         * Run some frames and drop, this can be used to avoid
         * incorrect recognition results when switching decoders.
         * @param num number of frames to run and drop
         * @maixpy maix.nn.Speech.skip_frames
        */
        void skip_frames(int num);

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

        std::function<void(std::vector<std::pair<int, float>>, int)> _raw_callback;
        std::function<void(char*, int)> _digit_callback;
        std::function<void(std::vector<float>, int)> _kws_callback;
        std::function<void(std::pair<char*, char*>, int)> _lvcsr_callback;

        void deinit();

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
