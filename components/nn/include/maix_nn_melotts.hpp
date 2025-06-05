/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2025.5.20: Add melotts support.
 */

#pragma once
#ifdef PLATFORM_MAIXCAM2
#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>
#include "librosa_simple.hpp"
#include "maix_audio.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "onnxruntime_cxx_api.h"

namespace maix::nn
{
    /**
     * MeloTTS class
     * @maixpy maix.nn.MeloTTS
     */
    class MeloTTS
    {
        class Lexicon {
        private:
            std::unordered_map<std::string, std::pair<std::vector<int>, std::vector<int>>> lexicon;

        public:
            Lexicon(const std::string& lexicon_filename, const std::string& tokens_filename) {
                std::unordered_map<std::string, int> tokens;
                std::ifstream ifs(tokens_filename);
                assert(ifs.is_open());

                std::string line;
                while ( std::getline(ifs, line) ) {
                    auto splitted_line = split(line, ' ');
                    tokens.insert({splitted_line[0], std::stoi(splitted_line[1])});
                }
                ifs.close();

                ifs.open(lexicon_filename);
                assert(ifs.is_open());
                while ( std::getline(ifs, line) ) {
                    auto splitted_line = split(line, ' ');
                    std::string word_or_phrase = splitted_line[0];
                    size_t phone_tone_len = splitted_line.size() - 1;
                    size_t half_len = phone_tone_len / 2;
                    std::vector<int> phones, tones;
                    for (size_t i = 0; i < phone_tone_len; i++) {
                        auto phone_or_tone = splitted_line[i + 1];
                        if (i < half_len) {
                            phones.push_back(tokens[phone_or_tone]);
                        } else {
                            tones.push_back(std::stoi(phone_or_tone));
                        }
                    }

                    lexicon.insert({word_or_phrase, std::make_pair(phones, tones)});
                }

                lexicon["呣"] = lexicon["母"];
                lexicon["嗯"] = lexicon["恩"];

                const std::vector<std::string> punctuation{"!", "?", "…", ",", ".", "'", "-"};
                for (auto p : punctuation) {
                    int i = tokens[p];
                    int tone = 0;
                    lexicon[p] = std::make_pair(std::vector<int>{i}, std::vector<int>{tone});
                }
                lexicon[" "] = std::make_pair(std::vector<int>{tokens["_"]}, std::vector<int>{0});
            }

            std::vector<std::string> split (const std::string &s, char delim) {
                std::vector<std::string> result;
                std::stringstream ss (s);
                std::string item;
                while (getline (ss, item, delim)) {
                    result.push_back (item);
                }
                return result;
            }

            std::vector<std::string> splitEachChar(const std::string& text)
            {
                std::vector<std::string> words;
                std::string input(text);
                int len = input.length();
                int i = 0;

                while (i < len) {
                int next = 1;
                if ((input[i] & 0x80) == 0x00) {
                    // std::cout << "one character: " << input[i] << std::endl;
                } else if ((input[i] & 0xE0) == 0xC0) {
                    next = 2;
                    // std::cout << "two character: " << input.substr(i, next) << std::endl;
                } else if ((input[i] & 0xF0) == 0xE0) {
                    next = 3;
                    // std::cout << "three character: " << input.substr(i, next) << std::endl;
                } else if ((input[i] & 0xF8) == 0xF0) {
                    next = 4;
                    // std::cout << "four character: " << input.substr(i, next) << std::endl;
                }
                words.push_back(input.substr(i, next));
                i += next;
                }
                return words;
            }

            bool is_english(std::string s) {
                if (s.size() == 1)
                    return (s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z');
                else
                    return false;
            }

            std::vector<std::string> merge_english(const std::vector<std::string>& splitted_text) {
                std::vector<std::string> words;
                size_t i = 0;
                while (i < splitted_text.size()) {
                    std::string s;
                    if (is_english(splitted_text[i])) {
                        while (i < splitted_text.size()) {
                            if (!is_english(splitted_text[i])) {
                                break;
                            }
                            s += splitted_text[i];
                            i++;
                        }
                        // to lowercase
                        std::transform(s.begin(), s.end(), s.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                        words.push_back(s);
                        if (i >= splitted_text.size())
                            break;
                    }
                    else {
                        words.push_back(splitted_text[i]);
                        i++;
                    }
                }
                return words;
            }

            void convert(const std::string& text, std::vector<int>& phones, std::vector<int>& tones, std::vector<int>& word2ph) {
                auto splitted_text = splitEachChar(text);
                auto zh_mix_en = merge_english(splitted_text);
                for (auto c : zh_mix_en) {
                    std::string s{c};
                    if (s == "，")
                        s = ",";
                    else if (s == "。")
                        s = ".";
                    else if (s == "！")
                        s = "!";
                    else if (s == "？")
                        s = "?";

                    auto phones_and_tones = lexicon[" "];
                    if (lexicon.find(s) != lexicon.end()) {
                        phones_and_tones = lexicon[s];
                    }
                    phones.insert(phones.end(), phones_and_tones.first.begin(), phones_and_tones.first.end());
                    tones.insert(tones.end(), phones_and_tones.second.begin(), phones_and_tones.second.end());
                    word2ph.push_back(phones_and_tones.first.size());
                }
            }
        };

        class OnnxWrapper {
        public:
            OnnxWrapper():
                m_session(nullptr) {

            }
            ~OnnxWrapper() {
                Release();
            }

            int Init(const std::string& model_file);
            std::vector<Ort::Value> Run(std::vector<int>& phone,
                                        std::vector<int>& tones,
                                        std::vector<int>& langids,
                                        std::vector<float>& g,

                                        float noise_scale,
                                        float length_scale,
                                        float noise_scale_w,
                                        float sdp_ratio);

            inline int GetInputSize(int index) const {
                return m_input_sizes[index];
            }

            inline int GetOutputSize(int index) const {
                return m_output_sizes[index];
            }

            int Release() {
                if (m_session) {
                    delete m_session;
                    m_session = nullptr;
                }
                return 0;
            }

        private:
            Ort::Env m_ort_env;
            Ort::Session* m_session;
            int m_input_num, m_output_num;
            std::vector<std::string> m_input_names, m_output_names;
            std::vector<int> m_input_sizes, m_output_sizes;
        };

    public:
        /**
         * Constructor of MeloTTS class
         * @param model model path, default empty, you can load model later by load function.
         * @param language language code, default "zh", supported language code: "zh"
         * @param speed the speech rate of the audio is controlled by this value,lower values result in slower reading speed. default is 0.8
         * @param noise_scale this parameter controls the randomness in speech. increasing the value results in more varied and less deterministic speech output.default is 0.3
         * @param noise_scale_w this parameter controls the randomness in speech alignment. while a higher value can enhance naturalness, overly high values may introduce instability or distortion in the audio. default is 0.6
         * @param sdp_ratio the higher the alignment weight, the more natural the speech sounds, but excessive values may result in instability. default is 0.2
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.MeloTTS.__init__
         * @maixcdk maix.nn.MeloTTS.MeloTTS
         */
        MeloTTS(const string &model = "", std::string language = "zh", double speed = 0.8f, double noise_scale = 0.3f, double noise_scale_w = 0.6f, double sdp_ratio = 0.2f) {
            _decoder_model = nullptr;
            _dual_buff = false;
            _speed = speed;
            _language = language;
            _noise_scale = noise_scale;
            _noise_scale_w = noise_scale_w;
            _sdp_ratio = sdp_ratio;
            for (char& c : _language) {
                c = std::toupper(static_cast<unsigned char>(c));
            }

            if (!model.empty())
            {
                err::Err e = load(model);
                if (e != err::ERR_NONE)
                {
                    throw err::Exception(e, "load model failed");
                }
            }
        }

        ~MeloTTS() {
            unload();
        }

        std::string type()
        {
            return _extra_info["type"];
        }

        static err::Err __load_value_from_map(std::map<string, string> extra_info, std::string key, int &value) {
            if (extra_info.find(key) != extra_info.end()) {
                value = std::atoi(extra_info[key].c_str());
            } else {
                log::error("%s key not found", key.c_str());
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }

        static err::Err __load_value_from_map(std::map<string, string> extra_info, std::string key, std::string &value) {
            if (extra_info.find(key) != extra_info.end()) {
                value = extra_info[key];
            } else {
                log::error("%s key not found", key.c_str());
                return err::ERR_ARGS;
            }

            return err::ERR_NONE;
        }

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.MeloTTS.load
         */
        err::Err load(const string &model) {
            err::Err res = err::ERR_NONE;
            std::string value_string;
            int value_int;

            this->unload();
            uint64_t start = time::time_ms();
            // load decoder model
            _decoder_model = std::make_unique<nn::NN>(model, _dual_buff);
            if (!_decoder_model) {
                return err::ERR_NO_MEM;
            }
            log::info("load decoder model cost %ld ms", time::time_ms() - start), start = time::time_ms();

            // load extra info
            _extra_info = _decoder_model->extra_info();
            res = __load_value_from_map(_extra_info, "model_type", value_string);
            if (res != err::ERR_NONE) {
                this->unload();
                return res;
            }

            if (value_string != this->type_str) {
                log::error("model_type not match, expect '%s', but got '%s'", this->type_str.c_str(), value_string.c_str());
                this->unload();
                return err::ERR_ARGS;
            }

            // load sample rate
            res = __load_value_from_map(_extra_info, "sample_rate", value_int);
            if (res != err::ERR_NONE) {
                this->unload();
                return res;
            }
            _sample_rate = value_int;

            start = time::ticks_ms();
            // load tokens
            res = __load_value_from_map(_extra_info, "tokens", value_string);
            if (res != err::ERR_NONE) {
                this->unload();
                return res;
            }
            std::string token_file = fs::dirname(model) + "/" + value_string;

            // load lexicon
            res = __load_value_from_map(_extra_info, "lexicon", value_string);
            if (res != err::ERR_NONE) {
                this->unload();
                return res;
            }
            std::string lexicon_file = fs::dirname(model) + "/" + value_string;
            _lexicon = std::make_unique<Lexicon>(lexicon_file, token_file);
            log::info("load token and lexicon cost %ld ms", time::ticks_ms() - start);

            start = time::ticks_ms();
            // load static input
            res = __load_value_from_map(_extra_info, "static_input", value_string);
            if (res != err::ERR_NONE) {
                this->unload();
                return res;
            }
            std::string g_file = fs::dirname(model) + "/" + value_string;
            _g.resize(256, 0);
            FILE* fp = fopen(g_file.c_str(), "rb");
            if (!fp) {
                printf("Open %s failed!\n", g_file.c_str());
                return err::ERR_RUNTIME;
            }
            fread(_g.data(), sizeof(float), _g.size(), fp);
            fclose(fp);
            log::info("load g.bin cost %ld ms", time::ticks_ms() - start);

            start = time::ticks_ms();
            // load encoder model
            res = __load_value_from_map(_extra_info, "encoder", value_string);
            if (res != err::ERR_NONE) {
                this->unload();
                return res;
            }
            std::string encoder_file = fs::dirname(model) + "/" + value_string;
            if (0 != _encoder.Init(encoder_file)) {
                log::error("mellotts encoder init failed!\n");
                return err::ERR_RUNTIME;
            }
            log::info("load encoder cost %ld ms", time::ticks_ms() - start);
            return err::ERR_NONE;
        }

        /**
         * Unload model from memory
         * @return ERR_NONE if success
        */
        err::Err unload() {
            this->_decoder_model = nullptr;
            return err::ERR_NONE;
        }

        // static void print_test(char *name, int *data, int len, int oft = 50) {
        //     printf(" ============ %s ============ size:%d\r\n", name, len);
        //     oft = oft > len ? len : oft;
        //     for (size_t i = 0; i < oft; i ++) {
        //         printf("%d ", data[i]);
        //     }
        //     printf("\r\n");

        //     size_t start = len - oft > 0 ? len - oft : 0;
        //     for (size_t i = start; i < len; i ++) {
        //         printf("%d ", data[i]);
        //     }printf("\r\n");
        // }

        // static void print_test(char *name, float *data, int len, int oft = 50) {
        //     printf(" ============ %s ============ size:%d\r\n", name, len);
        //     oft = oft > len ? len : oft;
        //     for (size_t i = 0; i < oft; i ++) {
        //         printf("%f ", data[i]);
        //     }
        //     printf("\r\n");

        //     size_t start = len - oft > 0 ? len - oft : 0;
        //     for (size_t i = start; i < len; i ++) {
        //         printf("%f ", data[i]);
        //     }printf("\r\n");
        // }

        // static void print_test(char *name, uint8_t *data, int len, int oft = 50) {
        //     printf(" ============ %s ============ size:%d\r\n", name, len);
        //     oft = oft > len ? len : oft;
        //     for (size_t i = 0; i < oft; i ++) {
        //         printf("%#x ", data[i]);
        //     }
        //     printf("\r\n");

        //     size_t start = len - oft > 0 ? len - oft : 0;
        //     for (size_t i = start; i < len; i ++) {
        //         printf("%#x ", data[i]);
        //     }printf("\r\n");
        // }

        /**
         * Text to speech
         * @param text input text
         * @param path The output path of the voice file, the default sampling rate is 44100,
         * the number of channels is 1, and the number of sampling bits is 16. default is empty.
         * @param output_pcm Enable or disable the output of raw PCM data. The default output sampling rate is 44100,
         * the number of channels is 1, and the sampling depth is 16 bits. default is false.
         * @return raw PCM data
         * @maixpy maix.nn.MeloTTS.forward
        */
        Bytes *forward(std::string text, std::string path = "", bool output_pcm = false) {
            err::Err err = err::ERR_NONE;
            float noise_scale   = _noise_scale;
            float length_scale  = 1.0 / _speed;
            float noise_scale_w = _noise_scale_w;
            float sdp_ratio     = _sdp_ratio;

            auto sens = split_sentence(text, 10, _language);
            std::vector<float> wavlist;
            for (auto& se : sens) {

                // Convert sentence to phones and tones
                std::vector<int> phones_bef, tones_bef, word2ph;
                _lexicon->convert(se, phones_bef, tones_bef, word2ph);

                // Add blank between words
                auto phones = intersperse(phones_bef, 0);
                auto tones = intersperse(tones_bef, 0);
                for (int& i : word2ph) {
                    i *= 2;
                }
                if (!word2ph.empty())
                    word2ph[0] += 1;

                int phone_len = phones.size();

                std::vector<int> langids(phone_len, 3);

                // Run encoder
                auto encoder_output = _encoder.Run(phones, tones, langids, _g, noise_scale, noise_scale_w, length_scale, sdp_ratio);
                float* zp_data = encoder_output.at(0).GetTensorMutableData<float>();
                int* pronoun_lens_data = encoder_output.at(1).GetTensorMutableData<int>();
                // int audio_len = encoder_output.at(2).GetTensorMutableData<int>()[0];
                auto zp_info = encoder_output.at(0).GetTensorTypeAndShapeInfo();
                auto zp_shape = zp_info.GetShape();
                std::vector<int> pronoun_lens(pronoun_lens_data, pronoun_lens_data + phone_len);

                auto inputs_info = _decoder_model->inputs_info();
                auto outputs_info = _decoder_model->outputs_info();
                int zp_size = inputs_info[0].shape_int();
                int dec_len = zp_size / zp_shape[1];
                int audio_slice_len = outputs_info[0].shape_int();

                // Generate pronoun slices for better effect
                auto word2pronoun = calc_word2pronoun(word2ph, pronoun_lens);
                auto dec_slices = generate_slices(word2pronoun, dec_len);

                // int dec_slice_num = int(std::ceil(zp_shape[2] * 1.0 / dec_len));
                size_t dec_slice_num = dec_slices.first.size();

                // Iteratively run decoder
                for (size_t i = 0; i < dec_slice_num; i++) {
                    const Slice& ps = dec_slices.first[i];
                    const Slice& zs = dec_slices.second[i];

                    std::vector<float> zp_slice(zp_size, 0);
                    int actual_size = std::min(zs.end - zs.start, dec_len);
                    for (int n = 0; n < zp_shape[1]; n++) {
                        memcpy(zp_slice.data() + n * dec_len, zp_data + n * zp_shape[2] + zs.start, sizeof(float) * actual_size);
                    }

                    // 输出音频的长度
                    int sub_audio_len = 512 * actual_size;
                    tensor::Tensors input_tensors, output_tensors;
                    tensor::Tensor *input_tensor0 = new tensor::Tensor(inputs_info[0].shape, inputs_info[0].dtype, zp_slice.data(), false);
                    tensor::Tensor *input_tensor1 = new tensor::Tensor(inputs_info[1].shape, inputs_info[1].dtype, _g.data(), false);
                    input_tensors.add_tensor(inputs_info[0].name, input_tensor0, false, true);
                    input_tensors.add_tensor(inputs_info[1].name, input_tensor1, false, true);
                    if (err::ERR_NONE != (err = _decoder_model->forward(input_tensors, output_tensors, false, true))) {
                        log::error("decoder forward failed! err:%d", err);
                        return nullptr;
                    }
                    auto output_tensor = output_tensors.get_tensor(outputs_info[0].name);
                    if (output_tensor.size_int() != audio_slice_len) {
                        log::error("decoder output size error! %d != %d", output_tensor.size_int(), audio_slice_len);
                        return nullptr;
                    }

                    // 处理overlap
                    int audio_start = 0;
                    if (!wavlist.empty()) {
                        if (dec_slices.first[i - 1].end > ps.start) {
                            // 去掉第一个字
                            audio_start = 512 * word2pronoun[ps.start];
                        }
                    }

                    int audio_end = sub_audio_len;
                    if (i < dec_slices.first.size() - 1) {
                        if (ps.end > dec_slices.first[i + 1].start) {
                            // 去掉最后一个字
                            audio_end = sub_audio_len - 512 * word2pronoun[ps.end - 1];
                        }
                    }

                    wavlist.insert(wavlist.end(), (float *)output_tensor.data() + audio_start, (float *)output_tensor.data() + audio_end);
                }
            }

            Bytes *pcm = nullptr;
            if (path.size() > 0 || output_pcm) {
                audio::File file;
                file.float_to_pcm_bytes(wavlist.data(), wavlist.size(), 16, &pcm);
                if (path.size() > 0) {
                    file.sample_bits(16);
                    file.sample_rate(_sample_rate);
                    file.channels(1);
                    file.set_pcm(pcm);
                    if (err::ERR_NONE != file.save(path)) {
                        log::error("melotts save file error");
                    }
                }

                if (!output_pcm) {
                    delete pcm;
                    pcm = nullptr;
                }
            }
            return pcm;
        }

        /**
         * Get pcm samplerate
         * @return pcm samplerate
         * @maixpy maix.nn.MeloTTS.samplerate
        */
        int samplerate() {
            return _sample_rate;
        }

        /**
         * Get the speed of the text
         * @return text speed
         * @maixpy maix.nn.MeloTTS.speed
        */
        double speed() {
            return _speed;
        }
    private:
        std::unique_ptr<nn::NN> _decoder_model;
        double _speed;
        double _noise_scale;
        double _noise_scale_w;
        double _sdp_ratio;
        std::string _language;
        int _sample_rate;
        bool _dual_buff;
        std::map<string, string> _extra_info;
        std::unique_ptr<Lexicon> _lexicon;
        OnnxWrapper _encoder;
        std::vector<float> _g;

        // 判断是否是UTF-8字符的后续字节
        inline bool is_utf8_continuation_byte(unsigned char c) {
            return (c & 0xC0) == 0x80;
        }

        // 计算UTF-8字符串的字符数（非字节数）
        size_t utf8_strlen(const string& str) {
            size_t len = 0;
            for (size_t i = 0; i < str.size(); ) {
                unsigned char c = str[i];
                if ((c & 0x80) == 0) { // ASCII字符
                    i += 1;
                } else if ((c & 0xE0) == 0xC0) { // 2字节UTF-8
                    i += 2;
                } else if ((c & 0xF0) == 0xE0) { // 3字节UTF-8（包括大部分中文）
                    i += 3;
                } else if ((c & 0xF8) == 0xF0) { // 4字节UTF-8
                    i += 4;
                } else {
                    i++; // 无效UTF-8，跳过
                }
                len++;
            }
            return len;
        }

        // 合并短句的英文版本
        vector<string> merge_short_sentences_en(const vector<string>& sens) {
            vector<string> sens_out;
            for (const auto& s : sens) {
                // 如果前一个句子太短（<=2个单词），就与当前句子合并
                if (!sens_out.empty()) {
                    istringstream iss(sens_out.back());
                    int word_count = distance(istream_iterator<string>(iss), istream_iterator<string>());
                    if (word_count <= 2) {
                        sens_out.back() += " " + s;
                        continue;
                    }
                }
                sens_out.push_back(s);
            }

            // 处理最后一个句子如果太短的情况
            if (!sens_out.empty() && sens_out.size() > 1) {
                istringstream iss(sens_out.back());
                int word_count = distance(istream_iterator<string>(iss), istream_iterator<string>());
                if (word_count <= 2) {
                    sens_out[sens_out.size()-2] += " " + sens_out.back();
                    sens_out.pop_back();
                }
            }

            return sens_out;
        }

        // 合并短句的中文版本
        vector<string> merge_short_sentences_zh(const vector<string>& sens) {
            vector<string> sens_out;
            for (const auto& s : sens) {
                // 如果前一个句子太短（<=2个字符），就与当前句子合并
                if (!sens_out.empty() && utf8_strlen(sens_out.back()) <= 2) {
                    sens_out.back() += " " + s;
                } else {
                    sens_out.push_back(s);
                }
            }

            // 处理最后一个句子如果太短的情况
            if (!sens_out.empty() && sens_out.size() > 1 && utf8_strlen(sens_out.back()) <= 2) {
                sens_out[sens_out.size()-2] += " " + sens_out.back();
                sens_out.pop_back();
            }

            return sens_out;
        }

        // 替换字符串中的子串
        string replace_all(const string& input, const string& from, const string& to) {
            string result = input;
            size_t pos = 0;
            while ((pos = result.find(from, pos)) != string::npos) {
                result.replace(pos, from.length(), to);
                pos += to.length();
            }
            return result;
        }

        // 分割拉丁语系文本（英文、法文、西班牙文等）
        vector<string> split_sentences_latin(const string& text, int min_len = 10) {
            string processed = text;

            // 替换中文标点为英文标点
            processed = replace_all(processed, "。", ".");
            processed = replace_all(processed, "！", ".");
            processed = replace_all(processed, "？", ".");
            processed = replace_all(processed, "；", ".");
            processed = replace_all(processed, "，", ",");
            processed = replace_all(processed, "“", "\"");
            processed = replace_all(processed, "”", "\"");
            processed = replace_all(processed, "‘", "'");
            processed = replace_all(processed, "’", "'");

            // 移除特定字符
            string chars_to_remove = "<>()[]\"«»";
            for (char c : chars_to_remove) {
                processed.erase(remove(processed.begin(), processed.end(), c), processed.end());
            }

            // 分割句子（简化版，按句号分割）
            vector<string> sentences;
            size_t start = 0;
            size_t end = processed.find('.');

            while (end != string::npos) {
                string sentence = processed.substr(start, end - start);
                // 去除前后空白
                sentence.erase(sentence.begin(), find_if(sentence.begin(), sentence.end(), [](int ch) { return !isspace(ch); }));
                sentence.erase(find_if(sentence.rbegin(), sentence.rend(), [](int ch) { return !isspace(ch); }).base(), sentence.end());
                if (!sentence.empty()) {
                    sentences.push_back(sentence);
                }
                start = end + 1;
                end = processed.find('.', start);
            }

            // 添加最后一部分
            if (start < processed.size()) {
                string sentence = processed.substr(start);
                sentence.erase(sentence.begin(), find_if(sentence.begin(), sentence.end(), [](int ch) { return !isspace(ch); }));
                sentence.erase(find_if(sentence.rbegin(), sentence.rend(), [](int ch) { return !isspace(ch); }).base(), sentence.end());
                if (!sentence.empty()) {
                    sentences.push_back(sentence);
                }
            }

            return merge_short_sentences_en(sentences);
        }

        // 分割中文文本
        vector<string> split_sentences_zh(const string& text, int min_len = 10) {
            string processed = text;

            // 替换中文标点为英文标点
            processed = replace_all(processed, "。", ".");
            processed = replace_all(processed, "！", ".");
            processed = replace_all(processed, "？", ".");
            processed = replace_all(processed, "；", ".");
            processed = replace_all(processed, "，", ",");

            // 将文本中的换行符、空格和制表符替换为空格
            processed = replace_all(processed, "\n", " ");
            processed = replace_all(processed, "\t", " ");
            processed = replace_all(processed, "  ", " "); // 多个空格合并为一个

            // 在标点符号后添加一个特殊标记用于分割
            string punctuation = ".,!?;";
            for (char c : punctuation) {
                string from(1, c);
                string to = from + " $#!";
                processed = replace_all(processed, from, to);
            }

            // 分割句子
            vector<string> sentences;
            size_t start = 0;
            size_t end = processed.find("$#!");

            while (end != string::npos) {
                string sentence = processed.substr(start, end - start);
                // 去除前后空白
                sentence.erase(sentence.begin(), find_if(sentence.begin(), sentence.end(), [](int ch) { return !isspace(ch); }));
                sentence.erase(find_if(sentence.rbegin(), sentence.rend(), [](int ch) { return !isspace(ch); }).base(), sentence.end());
                if (!sentence.empty()) {
                    sentences.push_back(sentence);
                }
                start = end + 3; // "$#!" 长度为3
                end = processed.find("$#!", start);
            }

            // 添加最后一部分
            if (start < processed.size()) {
                string sentence = processed.substr(start);
                sentence.erase(sentence.begin(), find_if(sentence.begin(), sentence.end(), [](int ch) { return !isspace(ch); }));
                sentence.erase(find_if(sentence.rbegin(), sentence.rend(), [](int ch) { return !isspace(ch); }).base(), sentence.end());
                if (!sentence.empty()) {
                    sentences.push_back(sentence);
                }
            }

            // 按最小长度合并句子
            vector<string> new_sentences;
            vector<string> new_sent;
            int count_len = 0;

            for (size_t i = 0; i < sentences.size(); ++i) {
                new_sent.push_back(sentences[i]);
                count_len += utf8_strlen(sentences[i]);
                if (count_len > min_len || i == sentences.size() - 1) {
                    count_len = 0;
                    ostringstream oss;
                    for (size_t j = 0; j < new_sent.size(); ++j) {
                        if (j != 0) oss << " ";
                        oss << new_sent[j];
                    }
                    new_sentences.push_back(oss.str());
                    new_sent.clear();
                }
            }

            return merge_short_sentences_zh(new_sentences);
        }

        // 主分割函数
        vector<string> split_sentence(const string& text, int min_len = 10, const string& language_str = "EN") {
            if (language_str == "EN" || language_str == "FR" || language_str == "ES" || language_str == "SP") {
                return split_sentences_latin(text, min_len);
            } else {
                return split_sentences_zh(text, min_len);
            }
        }

        static std::vector<int> intersperse(const std::vector<int>& lst, int item) {
            std::vector<int> result(lst.size() * 2 + 1, item);
            for (size_t i = 1; i < result.size(); i+=2) {
                result[i] = lst[i / 2];
            }
            return result;
        }

        // 计算每个词的发音时长
        vector<int> calc_word2pronoun(const vector<int>& word2ph, const vector<int>& pronoun_lens) {
            vector<int> indice = {0};
            for (size_t i = 0; i < word2ph.size() - 1; ++i) {
                indice.push_back(indice.back() + word2ph[i]);
            }

            vector<int> word2pronoun;
            for (size_t i = 0; i < word2ph.size(); ++i) {
                int start = indice[i];
                int end = start + word2ph[i];
                int sum = accumulate(pronoun_lens.begin() + start, pronoun_lens.begin() + end, 0);
                word2pronoun.push_back(sum);
            }
            return word2pronoun;
        }

        struct Slice {
            int start;
            int end;
            Slice(int s, int e) : start(s), end(e) {}
        };

        // 生成有overlap的slice，slice索引是对于zp的
        pair<vector<Slice>, vector<Slice>> generate_slices(const vector<int>& word2pronoun, int dec_len) {
            int pn_start = 0, pn_end = 0;
            int zp_start = 0, zp_end = 0;
            int zp_len = 0;
            vector<Slice> pn_slices;
            vector<Slice> zp_slices;

            while (pn_end < static_cast<int>(word2pronoun.size())) {
                // 检查是否可以向前overlap两个字
                if (pn_end - pn_start > 2 &&
                    accumulate(word2pronoun.begin() + pn_end - 2, word2pronoun.begin() + pn_end + 1, 0) <= dec_len) {
                    zp_len = accumulate(word2pronoun.begin() + pn_end - 2, word2pronoun.begin() + pn_end, 0);
                    zp_start = zp_end - zp_len;
                    pn_start = pn_end - 2;
                } else {
                    zp_len = 0;
                    zp_start = zp_end;
                    pn_start = pn_end;
                }

                while (pn_end < static_cast<int>(word2pronoun.size()) &&
                    zp_len + word2pronoun[pn_end] <= dec_len) {
                    zp_len += word2pronoun[pn_end];
                    pn_end++;
                }

                zp_end = zp_start + zp_len;
                pn_slices.emplace_back(pn_start, pn_end);
                zp_slices.emplace_back(zp_start, zp_end);
            }

            return make_pair(pn_slices, zp_slices);
        }
    protected:
        std::string type_str = "melotts";
    };
} // namespace maix::nn
#endif // PLATFORM_MAIXCAM2