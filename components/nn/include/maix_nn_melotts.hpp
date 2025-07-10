/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2025.5.20: Add melotts support.
 */

#pragma once
#include "maix_basic.hpp"

namespace maix::nn
{
    /**
     * MeloTTS class
     * @maixpy maix.nn.MeloTTS
     */
    class MeloTTS
    {
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
        MeloTTS(const string &model = "", std::string language = "zh", double speed = 0.8f, double noise_scale = 0.3f, double noise_scale_w = 0.6f, double sdp_ratio = 0.2f);
        ~MeloTTS();

        std::string type();

        /**
         * Load model from file
         * @param model Model path want to load
         * @return err::Err
         * @maixpy maix.nn.MeloTTS.load
         */
        err::Err load(const string &model);

        /**
         * Unload model from memory
         * @return ERR_NONE if success
        */
        err::Err unload();

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
        Bytes *forward(std::string text, std::string path = "", bool output_pcm = false);

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
        double _speed;
        int _sample_rate;
        void *_extra_param;
    protected:
        std::string type_str = "melotts";
    };
} // namespace maix::nn