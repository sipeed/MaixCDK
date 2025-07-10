/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.6.7: Add yolov8 support.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_nn.hpp"

namespace maix::nn
{
    /**
     * Whiper class
     * @maixpy maix.nn.Whisper
     */
    class Whisper
    {
        bool _dual_buff = true;
        int _input_pcm_samplerate;
        int _input_pcm_channels;
        int _input_pcm_bits_per_frame;
        void *_extra_param;
    protected:
        std::string type_str = "whisper";
    public:
        /**
         * Constructor of Whisper class
         * @param model model path, default empty, you can load model later by load function.
         * @param language language code, default "zh", supported language code: "zh", "en"
         * @throw If model arg is not empty and load failed, will throw err::Exception.
         * @maixpy maix.nn.Whisper.__init__
         * @maixcdk maix.nn.Whisper.Whisper
         */
        Whisper(const string &model = "", std::string language = "zh");
        ~Whisper();

        std::string type();

        /**
         * Unload model from memory
         * @return ERR_NONE if success
        */
        err::Err unload();

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.Whisper.load
         */
        err::Err load(const string &model);

        /**
         * Transcribe audio file to text
         * @note If the wav file has multiple channels, only the first channel will be used.
         * @param file Pass in an audio file, supporting files in WAV format.
         * @return The output result after automatic speech recognition.
         * @maixpy maix.nn.Whisper.transcribe
        */
        std::string transcribe(std::string &file);

        /**
         * Transcribe pcm data to text
         * @param pcm RAW data
         * @return The output result after automatic speech recognition.
         * @maixpy maix.nn.Whisper.transcribe_raw
        */
        std::string transcribe_raw(Bytes *pcm, int sample_rate = 16000, int channels = 1, int bits_per_frame = 16);

        /**
         * Get input pcm samplerate
         * @return input pcm samplerate
         * @maixpy maix.nn.Whisper.input_pcm_samplerate
        */
        int input_pcm_samplerate() {
            return _input_pcm_samplerate;
        }

        /**
         * Get input pcm channels
         * @return input pcm channels
         * @maixpy maix.nn.Whisper.input_pcm_channels
        */
        int input_pcm_channels() {
            return _input_pcm_channels;
        }

        /**
         * Get input pcm bits per frame
         * @return input pcm bits per frame
         * @maixpy maix.nn.Whisper.input_pcm_bits_per_frame
        */
        int input_pcm_bits_per_frame() {
            return _input_pcm_bits_per_frame;
        }
    };
} // namespace maix::nn