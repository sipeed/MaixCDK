/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.6.7: Add yolov8 support.
 */

#include "maix_basic.hpp"
#include "maix_nn.hpp"
#include "maix_image.hpp"
#include "maix_nn_F.hpp"
#include "maix_nn_object.hpp"
#include <math.h>
#include "maix_nn_yolo11.hpp"
#include "librosa_simple.hpp"
#include "maix_audio.hpp"
#include <fstream>
#include "opencc.h"
#include "maix_nn_whisper.hpp"

namespace maix::nn
{
    #define WHISPER_SAMPLE_RATE 16000
    #define WHISPER_N_FFT       400
    #define WHISPER_HOP_LENGTH  160
    #define WHISPER_CHUNK_SIZE  30
    #define WHISPER_N_MELS      80

    #define WHISPER_SOT         50258
    #define WHISPER_EOT         50257
    #define WHISPER_BLANK       220
    #define WHISPER_NO_TIMESTAMPS   50363
    #define WHISPER_NO_SPEECH   50362
    #define WHISPER_TRANSLATE   50358
    #define WHISPER_TRANSCRIBE  50359
    #define WHISPER_VOCAB_SIZE  51865
    #define WHISPER_N_TEXT_CTX  448
    #define NEG_INF             -std::numeric_limits<float>::infinity()

    static std::vector<int> WHISPER_LANG_CODES{
        50273,50303,50288,50261,50342,50299,50330,50302,50336,50267,50287,50292,50294,50323,50348,50291,50317,
        50326,50289,50356,50290,50282,50347,50331,50354,50264,50333,50296,50339,50318,50305,50293,50280,50322,
        50312,50306,50353,50285,50275,50340,50278,50268,50337,50316,50266,50307,50310,50338,50334,50313,50351,
        50260,50344,50283,50327,50272,50324,50276,50281,50301,50332,50300,50309,50343,50349,50335,50320,50259,
        50284,50304,50277,50311,50319,50314,50352,50328,50286,50274,50329,50270,50269,50350,50263,50345,50298,
        50279,50297,50262,50315,50321,50308,50355,50265,50346,50295,50271,50357,50341,50325
    };

    static std::vector<std::string> WHISPER_LANG_NAMES{
        "sv","sr","no","de","nn","te", "be","bn","lo","pt","ta","bg","la","km","tl","hr","sq","so","th","jw","ur","ms","bo",
        "tg","ha","ko","gu","ml","ht", "sw","sl","lt","uk","si","hy","kn","ln","da","id","ps","vi","tr","uz","kk","ja","et",
        "eu","fo","am","ne","tt","zh", "sa","cs","af","ar","sn","hi","el","lv","sd","fa","br","mt","mg","yi","mr","en","ro",
        "az","fi","is","gl","mn","haw","oc","hu","it","ka","ca","pl","as","ru","lb","sk","he","cy","es","bs","pa","mk","ba",
        "fr","my","mi","nl","su","tk", "yo"
    };

    static std::unordered_map<std::string, int> WHISPER_N_TEXT_STATE_MAP{
        {"tiny",    384},
        {"base",    512},
        {"small",   768}
    };

    static std::vector<int32_t> SOT_SEQUENCE{WHISPER_SOT,50260,WHISPER_TRANSCRIBE,WHISPER_NO_TIMESTAMPS};

    static void supress_tokens(std::vector<float>& logits, bool is_initial) {
        if (is_initial) {
            logits[WHISPER_EOT] = NEG_INF;
            logits[WHISPER_BLANK] = NEG_INF;
        }

        logits[WHISPER_NO_TIMESTAMPS] = NEG_INF;
        logits[WHISPER_SOT] = NEG_INF;
        logits[WHISPER_NO_SPEECH] = NEG_INF;
        logits[WHISPER_TRANSLATE] = NEG_INF;
    }

    static int argmax(const std::vector<float>& logits) {
        auto max_iter = std::max_element(logits.begin(), logits.end());
        return std::distance(logits.begin(), max_iter); // absolute index of max
    }

    static int detect_language(const std::string& language) {
        int i = 51; // zh
        for (int n = 0; n < (int)WHISPER_LANG_CODES.size(); n++) {
            if (language == WHISPER_LANG_NAMES[n]) {
                i = n;
                break;
            }
        }

        return WHISPER_LANG_CODES[i];
    }

    static uint8_t reverse_map[] =
    {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
        255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
    };

    static int base64_decode(const uint8_t* code, uint32_t code_len, char* str)
    {
        uint8_t plain[1024];
        assert((code_len & 0x03) == 0);  //如果它的条件返回错误，则终止程序执行。4的倍数。

        uint32_t i, j = 0;
        uint8_t quad[4];
        for (i = 0; i < code_len; i += 4)
        {
            for (uint32_t k = 0; k < 4; k++)
            {
                quad[k] = reverse_map[code[i + k]];//分组，每组四个分别依次转换为base64表内的十进制数
            }

            assert(quad[0] < 64 && quad[1] < 64);

            plain[j++] = (quad[0] << 2) | (quad[1] >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的前2位进行组合

            if (quad[2] >= 64)
                break;
            else if (quad[3] >= 64)
            {
                plain[j++] = (quad[1] << 4) | (quad[2] >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应base64表的十进制数的前4位进行组合
                break;
            }
            else
            {
                plain[j++] = (quad[1] << 4) | (quad[2] >> 2);
                plain[j++] = (quad[2] << 6) | quad[3];//取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
            }
        }
        plain[j] = 0;
        // char str[1024] = "";
        strcpy(str, (char*)plain);
        // strcpy_s(str, sizeof(plain), U2G(str));
        return j;
    }

    namespace {
        class WhisperParam {
        public:
            nn::NN *encoder_model;
            nn::NN *decoder_main_model;
            nn::NN *decoder_loop_model;
            std::vector<float> positional_embedding;
            std::vector<std::string> token_tables;
            std::string language;
            std::string whisper_type;
            bool dual_buff = true;
            int n_fft;
            int n_hop;
            int n_mels;
            int n_text_state;
            std::map<string, string> extra_info;
            std::unique_ptr<opencc::SimpleConverter> simple_converter;
        };
    }

    Whisper::Whisper(const string &model, std::string language) {
        _extra_param = new WhisperParam();
        WhisperParam *param = (WhisperParam *) _extra_param;
        param->encoder_model = nullptr;
        param->decoder_main_model = nullptr;
        param->decoder_loop_model = nullptr;
        param->dual_buff = false;
        param->language = language;

        if (!model.empty())
        {
            err::Err e = load(model);
            if (e != err::ERR_NONE)
            {
                throw err::Exception(e, "load model failed");
            }
        }
    }

    Whisper::~Whisper() {
        unload();
    }

    std::string Whisper::type()
    {
        WhisperParam *param = (WhisperParam *) _extra_param;
        return param->extra_info["type"];
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
     * Unload model from memory
     * @return ERR_NONE if success
    */
    err::Err Whisper::unload() {
        WhisperParam *param = (WhisperParam *) _extra_param;
        if (param->encoder_model) {
            delete param->encoder_model;
            param->encoder_model = NULL;
        }

        if (param->decoder_main_model) {
            delete param->decoder_main_model;
            param->decoder_main_model = NULL;
        }

        if (param->decoder_loop_model) {
            delete param->decoder_loop_model;
            param->decoder_loop_model = NULL;
        }
        return err::ERR_NONE;
    }

    /**
     * Load model from file
     * @param model Model path want to load
     * @return err::Err
     * @maixpy maix.nn.Whisper.load
     */
    err::Err Whisper::load(const string &model) {
        err::Err res = err::ERR_NONE;
        WhisperParam *param = (WhisperParam *) _extra_param;
        std::string value_string;
        int value_int;

        this->unload();

        param->encoder_model = new nn::NN(model, param->dual_buff);
        if (!param->encoder_model) {
            return err::ERR_NO_MEM;
        }

        param->extra_info = param->encoder_model->extra_info();
        res = __load_value_from_map(param->extra_info, "model_type", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }

        if (value_string != this->type_str) {
            log::error("model_type not match, expect '%s', but got '%s'", this->type_str.c_str(), value_string.c_str());
            this->unload();
            return err::ERR_ARGS;
        }

        res = __load_value_from_map(param->extra_info, "input_type", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        if (value_string != "pcm") {
            log::error("unknown input type: %s", value_string.c_str());
            this->unload();
            return err::ERR_ARGS;
        }

        res = __load_value_from_map(param->extra_info, "pcm_samplerate", value_int);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        _input_pcm_samplerate = value_int;

        // res = __load_value_from_map(param->extra_info, "pcm_channels", value_int);
        // if (res != err::ERR_NONE) {
        //     this->unload();
        //     return res;
        // }
        // _input_pcm_channels = value_int;

        // res = __load_value_from_map(param->extra_info, "pcm_bits_per_frame", value_int);
        // if (res != err::ERR_NONE) {
        //     this->unload();
        //     return res;
        // }
        // _input_pcm_bits_per_frame= value_int;

        res = __load_value_from_map(param->extra_info, "n_fft", value_int);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        param->n_fft = value_int;

        res = __load_value_from_map(param->extra_info, "n_hop", value_int);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        param->n_hop = value_int;

        res = __load_value_from_map(param->extra_info, "n_mels", value_int);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        param->n_mels = value_int;

        res = __load_value_from_map(param->extra_info, "whisper_type", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        param->whisper_type = value_string;

        // load decoder-main model
        res = __load_value_from_map(param->extra_info, "decoder-main", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        std::string model_file = fs::dirname(model) + "/" + value_string;
        param->decoder_main_model = new nn::NN(model_file);
        if (param->decoder_main_model == nullptr) {
            log::error("load decoder-main model failed");
            this->unload();
            return err::ERR_RUNTIME;
        }

        // load decoder-loop model
        res = __load_value_from_map(param->extra_info, "decoder-loop", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }
        model_file = fs::dirname(model) + "/" + value_string;
        param->decoder_loop_model = new nn::NN(model_file);
        if (param->decoder_loop_model == nullptr) {
            log::error("load decoder-loop model failed");
            this->unload();
            return err::ERR_RUNTIME;
        }

        // load pe
        res = __load_value_from_map(param->extra_info, "positional_embedding", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }

        if (param->whisper_type == "tiny") {
            param->n_text_state = 384;
        } else if (param->whisper_type == "base") {
            param->n_text_state = 512;
        } else if (param->whisper_type == "small") {
            param->n_text_state = 768;
        } else {
            log::error("invalid whisper type:%s", param->whisper_type.c_str());
            this->unload();
            return err::ERR_ARGS;
        }
        param->positional_embedding.resize(WHISPER_N_TEXT_CTX * param->n_text_state);
        auto positional_embedding_file = fs::dirname(model) + "/" + value_string;
        FILE* fp = fopen(positional_embedding_file.c_str(), "rb");
        if (!fp) {
            fprintf(stderr, "Can NOT open %s\n", value_string.c_str());
            this->unload();
            return err::ERR_ARGS;
        }
        fread(param->positional_embedding.data(), sizeof(float), WHISPER_N_TEXT_CTX * param->n_text_state, fp);
        fclose(fp);

        // load tokens
        res = __load_value_from_map(param->extra_info, "tokens", value_string);
        if (res != err::ERR_NONE) {
            this->unload();
            return res;
        }

        auto tokens_file = fs::dirname(model) + "/" + value_string;
        std::ifstream ifs(tokens_file);
        if (!ifs.is_open()) {
            fprintf(stderr, "Can NOT open %s\n", value_string.c_str());
            this->unload();
            return err::ERR_ARGS;
        }
        std::string line;
        while (std::getline(ifs, line)) {
            size_t i = line.find(' ');
            param->token_tables.push_back(line.substr(0, i));
        }

        // load tokens
        param->simple_converter = std::make_unique<opencc::SimpleConverter>("/opt/etc/t2s.json");
        return err::ERR_NONE;
    }

    /**
     * Transcribe audio file to text
     * @note If the wav file has multiple channels, only the first channel will be used.
     * @param file Pass in an audio file, supporting files in WAV format.
     * @return The output result after automatic speech recognition.
     * @maixpy maix.nn.Whisper.transcribe
    */
    std::string Whisper::transcribe(std::string &file) {
        if (fs::exists(file) == false) {
            return "";
        }

        auto f = fopen(file.c_str(), "rb+");
        if (f == NULL) {
            log::error("open file error");
            return "";
        }

        if (fs::splitext(file)[1] != ".wav") {
            log::error("file is not wav");
            fclose(f);
            return "";
        }

        audio::File audio_file;
        audio_file.load(file);
        auto pcm = audio_file.get_pcm(false);
        auto result = transcribe_raw(pcm, audio_file.sample_rate(), audio_file.channels(), audio_file.sample_bits());
        delete pcm;
        return result;
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
     * Transcribe pcm data to text
     * @param pcm RAW data
     * @return The output result after automatic speech recognition.
     * @maixpy maix.nn.Whisper.transcribe_raw
    */
    std::string Whisper::transcribe_raw(Bytes *pcm, int sample_rate, int channels, int bits_per_frame) {
        err::Err err = err::ERR_NONE;
        WhisperParam *param = (WhisperParam *) _extra_param;
        if (sample_rate != _input_pcm_samplerate) {
            log::error("wav sample rate not match, must be %d!", _input_pcm_samplerate);
            return "";
        }

        int bytes_per_frame = bits_per_frame * channels / 8;
        int num_of_samples = pcm->data_len / bytes_per_frame;
        std::vector<float> pcm_data(num_of_samples);
        for (int i = 0; i < num_of_samples; i ++) {
            uint8_t *data = pcm->data + i * bytes_per_frame;
            switch (bits_per_frame) {
            case 8:
            {
                pcm_data[i] = (float)(data[0] - 128) / 128.;
                break;
            }
            case 16:
            {
                int16_t result = ((int16_t)data[1] << 8) | data[0];
                pcm_data[i] = (float)result / 32768.;
                break;
            }
            case 24:
            {
                int32_t result = ((int32_t)data[2] << 16) | ((int32_t)data[1] << 8) | data[0];
                if (result & 0x800000) {
                    result = result | ~0xFFFFFF;
                }
                pcm_data[i] = (float)(result) / 8388608.;
                break;
            }
            case 32:
            {
                int32_t result = ((int32_t)data[3] << 24) | ((int32_t)data[2] << 16) | ((int32_t)data[1] << 8) | data[0];
                pcm_data[i] = (float)(result) / 2147483648.;
                break;
            }
            default:
                log::error("unsupported sample bit %d", bits_per_frame);
                return "";
            }
        }
        // print_test("pcm_data_F", (float *)pcm_data.data(), pcm_data.size(), 100);
        // print_test("pcm_data", (uint8_t *)pcm->data, pcm->data_len, 100);
        auto mel = librosa_simple::Feature::melspectrogram(pcm_data, _input_pcm_samplerate, param->n_fft, param->n_hop, "hann", true, "reflect", 2.0f, param->n_mels, 0.0f, _input_pcm_samplerate / 2);
        int n_mel = mel.size();
        int n_len = mel[0].size();

        // clamping and normalization
        double mmax = -1e20;
        for (int i = 0; i < WHISPER_N_MELS; i++) {
            for (int n = 0; n < n_len; n++) {
                mel[i][n] = std::log10(std::max(mel[i][n], 1e-10f));

                if (mel[i][n] > mmax) {
                    mmax = mel[i][n] ;
                }
            }
        }

        for (int i = 0; i < WHISPER_N_MELS; i++) {
            for (int n = 0; n < n_len; n++) {
                mel[i][n] = (std::max(mel[i][n], (float)(mmax - 8.0)) + 4.0)/4.0;
                mel[i].resize(3000);
            }
        }

        n_len = mel[0].size();

        int offset = 0;
        std::vector<float> logits(WHISPER_VOCAB_SIZE);
        int max_token_id = -1;
        std::vector<int> results;
        std::vector<int> tokens(1);

        auto encoder_outputs_info0 = param->encoder_model->outputs_info()[0];
        auto encoder_outputs_info1 = param->encoder_model->outputs_info()[1];
        auto encoder_ouptut_size0 = encoder_outputs_info0.shape_int() * tensor::dtype_size[encoder_outputs_info0.dtype];
        auto encoder_ouptut_size1 = encoder_outputs_info0.shape_int() * tensor::dtype_size[encoder_outputs_info1.dtype];

        auto decoder_main_outputs_info1 = param->decoder_main_model->outputs_info()[1];
        auto decoder_main_outputs_info2 = param->decoder_main_model->outputs_info()[2];
        auto decoder_main_ouptut_size1 = decoder_main_outputs_info1.shape_int() * tensor::dtype_size[decoder_main_outputs_info1.dtype];
        auto decoder_main_ouptut_size2 = decoder_main_outputs_info2.shape_int() * tensor::dtype_size[decoder_main_outputs_info2.dtype];

        std::vector<float> n_layer_cross_k(encoder_ouptut_size0 / sizeof(float));
        std::vector<float> n_layer_cross_v(encoder_ouptut_size1 / sizeof(float));

        std::vector<float> decoder_main_logits(4 * WHISPER_VOCAB_SIZE);
        std::vector<float> n_layer_self_k_cache(decoder_main_ouptut_size1 / sizeof(float));
        std::vector<float> n_layer_self_v_cache(decoder_main_ouptut_size2 / sizeof(float));

        // encoder
        std::vector<float> continous_mel(WHISPER_N_MELS * n_len);
        for (int i = 0; i < n_mel; i++) {
            memcpy(continous_mel.data() + i * n_len, mel[i].data(), sizeof(float) * n_len);
        }

        // print_test(">>>>>>>>>>>>>>>>> Encoder input 0", (float *)continous_mel.data(), continous_mel.size());
        auto encoder_input_tensor = new tensor::Tensor({1, 80, 3000}, tensor::DType::FLOAT32, continous_mel.data(), false);
        tensor::Tensors encoder_input_tensors, encoder_output_tensors;
        encoder_input_tensors.add_tensor("mel", encoder_input_tensor, true, true);
        if ( err::ERR_NONE != (err = param->encoder_model->forward(encoder_input_tensors, encoder_output_tensors, false, true))) {
            log::error("encoder forward failed! err:%d", err);
            return "";
        }

        SOT_SEQUENCE[1] = detect_language(param->language);

        // print_test(">>>>>>>>>>>>>>>>> Decoder main input 0", (int *)SOT_SEQUENCE.data(), SOT_SEQUENCE.size());
        // print_test(">>>>>>>>>>>>>>>>> Decoder main input 1", (float *)encoder_output_tensors[0].data(), encoder_output_tensors[0].size_int());
        // print_test(">>>>>>>>>>>>>>>>> Decoder main input 2", (float *)encoder_output_tensors[1].data(), encoder_output_tensors[1].size_int());
        auto decoder_main_input_tensor0 = new tensor::Tensor({1, 4}, tensor::DType::INT32, SOT_SEQUENCE.data(), false);
        auto decoder_main_input_tensor1 = new tensor::Tensor({6, 1, 1500, 512}, tensor::DType::FLOAT32, encoder_output_tensors[0].data(), false);
        auto decoder_main_input_tensor2 = new tensor::Tensor({6, 1, 1500, 512}, tensor::DType::FLOAT32, encoder_output_tensors[1].data(), false);
        tensor::Tensors decoder_main_input_tensors, decoder_main_input_tensors2,decoder_main_output_tensors;
        decoder_main_input_tensors.add_tensor("tokens", decoder_main_input_tensor0, false, true);
        decoder_main_input_tensors.add_tensor("n_layer_cross_k", decoder_main_input_tensor1, false, true);
        decoder_main_input_tensors.add_tensor("n_layer_cross_v", decoder_main_input_tensor2, false, true);
        if (err::ERR_NONE != (err = param->decoder_main_model->forward(decoder_main_input_tensors, decoder_main_output_tensors, false, true))) {
            log::error("decoder main forward failed! err:%d", err);
            return "";
        }

        memcpy(decoder_main_logits.data(), decoder_main_output_tensors[0].data(), decoder_main_output_tensors[0].size_int() * tensor::dtype_size[decoder_main_output_tensors[0].dtype()]);
        offset += SOT_SEQUENCE.size();
        std::copy(decoder_main_logits.begin() + 3 * WHISPER_VOCAB_SIZE, decoder_main_logits.end(), logits.begin());
        supress_tokens(logits, true);
        max_token_id = argmax(logits);

        std::vector<float> mask(WHISPER_N_TEXT_CTX);
        for (int n = 0; n < WHISPER_N_TEXT_CTX - offset - 1; n++) {
            mask[n] = NEG_INF;
        }

        int WHISPER_N_TEXT_STATE = WHISPER_N_TEXT_STATE_MAP[param->whisper_type];
        auto decoder_loop_input_tensor1 = new tensor::Tensor({6, 1, 448, 512}, tensor::DType::FLOAT32, decoder_main_output_tensors[1].data(), false);
        auto decoder_loop_input_tensor2 = new tensor::Tensor({6, 1, 448, 512}, tensor::DType::FLOAT32, decoder_main_output_tensors[2].data(), false);
        auto decoder_loop_input_tensor3 = new tensor::Tensor({6, 1, 1500, 512}, tensor::DType::FLOAT32, encoder_output_tensors[0].data(), false);
        auto decoder_loop_input_tensor4 = new tensor::Tensor({6, 1, 1500, 512}, tensor::DType::FLOAT32, encoder_output_tensors[1].data(), false);
        tensor::Tensors decoder_loop_input_tensors, decoder_loop_output_tensors;
        bool first_forward_decoder_loop = true;
        decoder_loop_input_tensors.add_tensor("in_n_layer_self_k_cache", decoder_loop_input_tensor1, false, true);
        decoder_loop_input_tensors.add_tensor("in_n_layer_self_v_cache", decoder_loop_input_tensor2, false, true);
        decoder_loop_input_tensors.add_tensor("n_layer_cross_k", decoder_loop_input_tensor3, false, true);
        decoder_loop_input_tensors.add_tensor("n_layer_cross_v", decoder_loop_input_tensor4, false, true);
        for (size_t i = 0; i < WHISPER_N_TEXT_CTX - SOT_SEQUENCE.size(); i++) {
            if (max_token_id == WHISPER_EOT) {
                break;
            }

            results.push_back(max_token_id);
            tokens[0] = results.back();

            auto decoder_loop_input_tensor0 = new tensor::Tensor({1, 1}, tensor::DType::INT32, tokens.data(), false);
            auto decoder_loop_input_tensor5 = new tensor::Tensor({1, 512}, tensor::DType::FLOAT32, param->positional_embedding.data() + offset * WHISPER_N_TEXT_STATE, false);
            auto decoder_loop_input_tensor6 = new tensor::Tensor({448}, tensor::DType::FLOAT32, mask.data(), false);
            decoder_loop_input_tensors.add_tensor("tokens", decoder_loop_input_tensor0, false, true);
            decoder_loop_input_tensors.add_tensor("positional_embedding", decoder_loop_input_tensor5, false, true);
            decoder_loop_input_tensors.add_tensor("mask", decoder_loop_input_tensor6, false, true);

            if (err::ERR_NONE != (err = param->decoder_loop_model->forward(decoder_loop_input_tensors, decoder_loop_output_tensors, false, true))) {
                log::error("decoder loop forward failed! err:%d", err);
                return "";
            }

            decoder_loop_input_tensors.rm_tensor("tokens");
            decoder_loop_input_tensors.rm_tensor("positional_embedding");
            decoder_loop_input_tensors.rm_tensor("mask");
            decoder_loop_input_tensors.rm_tensor("in_n_layer_self_k_cache");
            decoder_loop_input_tensors.rm_tensor("in_n_layer_self_v_cache");
            auto decoder_loop_input_tensor1 = new tensor::Tensor({6, 1, 448, 512}, tensor::DType::FLOAT32, decoder_loop_output_tensors[1].data(), false);
            auto decoder_loop_input_tensor2 = new tensor::Tensor({6, 1, 448, 512}, tensor::DType::FLOAT32, decoder_loop_output_tensors[2].data(), false);
            decoder_loop_input_tensors.add_tensor("in_n_layer_self_k_cache", decoder_loop_input_tensor1, false, true);
            decoder_loop_input_tensors.add_tensor("in_n_layer_self_v_cache", decoder_loop_input_tensor2, false, true);

            if (first_forward_decoder_loop) {
                first_forward_decoder_loop = false;
                decoder_loop_input_tensors.rm_tensor("n_layer_cross_k");
                decoder_loop_input_tensors.rm_tensor("n_layer_cross_v");
            }

            memcpy(logits.data(), decoder_loop_output_tensors[0].data(), decoder_loop_output_tensors[0].size_int() * tensor::dtype_size[decoder_loop_output_tensors[0].dtype()]);
            offset += 1;
            mask[WHISPER_N_TEXT_CTX - offset - 1] = 0;
            supress_tokens(logits, false);
            max_token_id = argmax(logits);
        }

        std::string s;
        for (const auto i : results) {
            char str[1024];
            base64_decode((const uint8_t*)param->token_tables[i].c_str(), (uint32_t)param->token_tables[i].size(), str);
            s += str;
        }

        if (param->language == "zh") {
            s = param->simple_converter->Convert(s);
        }
        return s;
    }
} // namespace maix::nn