/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include <memory>
#include "AudioFile.h"

/**
 * @brief maix.audio module
 * @maixpy maix.audio
*/
namespace maix::audio
{
    /**
     * Audio type
     * @maixpy maix.audio.Format
     */
    enum Format
    {
        FMT_NONE = 0,       // format invalid
        FMT_S8,             // unsigned 8 bits
        FMT_S16_LE,         // signed 16 bits, little endian
        FMT_S32_LE,         // signed 32 bits, little endian
        FMT_S16_BE,         // signed 16 bits, big endian
        FMT_S32_BE,         // signed 32 bits, big endian
        FMT_U8,             // unsigned 8 bits
        FMT_U16_LE,         // unsigned 16 bits, little endian
        FMT_U32_LE,         // unsigned 32 bits, little endian
        FMT_U16_BE,         // unsigned 16 bits, big endian
        FMT_U32_BE,         // unsigned 32 bits, big endian
    };

    /**
     * Map the audio format to the number of bits
     * @param format audio format
     * @return number of bits
     * @maixpy maix.audio.fmt_bits
    */
    const std::vector<int> fmt_bits = {
        0, 8, 16, 32, 16, 32, 8, 16, 32, 16, 32
    };

    /**
     * Audio file reader
     * @maixpy maix.audio.File
    */
    class File: public AudioFile<float>
    {
    private:
        int _channels;
        int _sample_rate;
        int _sample_bits;
        bool is_pcm_file = false;

        static float _clamp_float(float val) {
            return std::max(-1.0f, std::min(1.0f, val));
        }

        static void _float_to_pcm_bytes(const float* float_data, size_t sample_count, int bit_depth, uint8_t *out_bytes) {
            for (size_t i = 0; i < sample_count; ++i) {
                float sample = _clamp_float(float_data[i]);

                switch (bit_depth) {
                    case 8: {
                        // 8-bit PCM is unsigned
                        uint8_t val = static_cast<uint8_t>((sample + 1.0f) * 127.5f);  // [−1,1] → [0,255]
                        out_bytes[i] = val;
                        break;
                    }
                    case 16: {
                        int16_t val = static_cast<int16_t>(sample * 32767.0f);
                        std::memcpy(&out_bytes[i * 2], &val, 2);
                        break;
                    }
                    case 24: {
                        int32_t val = static_cast<int32_t>(sample * 8388607.0f);
                        uint8_t* p = &out_bytes[i * 3];
                        p[0] = val & 0xFF;
                        p[1] = (val >> 8) & 0xFF;
                        p[2] = (val >> 16) & 0xFF;
                        break;
                    }
                    case 32: {
                        int32_t val = static_cast<int32_t>(sample * 2147483647.0f);
                        std::memcpy(&out_bytes[i * 4], &val, 4);
                        break;
                    }
                    default:
                        throw std::runtime_error("Unsupported bit depth");
                }
            }
        }

        err::Err _load_pcm(std::string path, int sample_rate = 16000, int channels = 1, int bits_per_sample = 16) {
            err::Err ret = err::ERR_NONE;

            auto new_file = fopen(path.c_str(), "rb");
            if (!new_file) {
                log::error("open wav file failed!");
                return err::ERR_RUNTIME;
            }
            fseek(new_file, 0, fs::SEEK_END);
            int data_size = ftell(new_file);
            auto bytes_per_frame = channels * bits_per_sample / 8;
            data_size = (data_size / bytes_per_frame) * bytes_per_frame;
            _sample_rate = sample_rate;
            _channels = channels;
            _sample_bits = bits_per_sample;

            pcm = std::make_unique<maix::Bytes>(nullptr, data_size);
            if (!pcm) {
                log::error("allocate memory failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }
            fseek(new_file, 0, fs::SEEK_SET);
            if (fread(pcm->data, 1, data_size, new_file) != (size_t)data_size) {
                log::error("read wav file failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }
            fclose(new_file);

            return ret;
        }

        err::Err _load_wav_aiff(std::string path) {
            bool loadedOK = AudioFile::load(path);
            if (!loadedOK) {
                log::error("load wav or aiff file failed!");
                return err::ERR_RUNTIME;
            }

            _sample_bits = AudioFile::getBitDepth();
            _sample_rate = AudioFile::getSampleRate();
            _channels = AudioFile::getNumChannels();
            auto sample_count = AudioFile::getNumSamplesPerChannel() * _channels;
            int offset = 0;
            std::vector<float> samples(sample_count);
            for (int i = 0; i < AudioFile::getNumSamplesPerChannel(); i++)
            {
                for (int channel = 0; channel < AudioFile::getNumChannels(); channel++)
                {
                    samples[offset ++] = AudioFile::samples[channel][i];
                }
            }

            float_to_pcm_bytes(samples.data(), sample_count, _sample_bits, pcm);

            // std::vector<uint8_t> out_bytes;
            // float_to_pcm_bytes(samples.data(), sample_count, _sample_bits, out_bytes);
            // pcm = std::make_unique<maix::Bytes>(out_bytes.data(), out_bytes.size());
            return err::ERR_NONE;
        }

        err::Err _save_pcm(std::string path)
        {
            err::Err ret = err::ERR_NONE;
            auto new_file = fopen(path.c_str(), "w+");
            if (!new_file) {
                log::error("open pcm file failed!");
                return err::ERR_RUNTIME;
            }

            if (fwrite(pcm->data, 1, pcm->data_len, new_file) != (size_t)pcm->data_len) {
                log::error("write pcm file failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }
            fclose(new_file);
            return ret;
        }

        err::Err _save_wav(std::string path) {
            AudioFile::setNumChannels(_channels);
            AudioFile::setSampleRate(_sample_rate);
            AudioFile::setBitDepth(_sample_bits);
            AudioFile::setNumSamplesPerChannel(pcm->data_len / _channels / _sample_bits * 8);

            std::vector<float> samples;
            int offset = 0;
            pcm_bytes_to_float(pcm->data, pcm->data_len / _sample_bits * 8, _sample_bits, samples);
            for (int i = 0; i < AudioFile::getNumSamplesPerChannel(); i++)
            {
                for (int channel = 0; channel < AudioFile::getNumChannels(); channel++)
                {
                    AudioFile::samples[channel][i] = samples[offset ++];
                }
            }
            auto ok = AudioFile::save(path, AudioFileFormat::Wave);
            if (ok) {
                return err::ERR_NONE;
            } else {
                return err::ERR_RUNTIME;
            }
        }

        err::Err _save_aiff(std::string path) {
            AudioFile::setNumChannels(_channels);
            AudioFile::setSampleRate(_sample_rate);
            AudioFile::setBitDepth(_sample_bits);
            AudioFile::setNumSamplesPerChannel(pcm->data_len / _channels);

            std::vector<float> samples;
            int offset = 0;
            pcm_bytes_to_float(pcm->data, pcm->data_len, _sample_bits, samples);
            for (int i = 0; i < AudioFile::getNumSamplesPerChannel(); i++)
            {
                for (int channel = 0; channel < AudioFile::getNumChannels(); channel++)
                {
                    AudioFile::samples[channel][i] = samples[offset ++];
                }
            }
            auto ok = AudioFile::save(path, AudioFileFormat::Aiff);
            if (ok) {
                return err::ERR_NONE;
            } else {
                return err::ERR_RUNTIME;
            }
        }
    public:
        std::unique_ptr<maix::Bytes> pcm;

        /**
         * @brief Convert a float value to PCM data
         * @param float_data The float value to convert.
         * @param sample_count The number of samples
         * @param bit_depth The bit depth
         * @param out_bytes The output buffer to store the converted data.
        */
        void float_to_pcm_bytes(const float* float_data, size_t sample_count, int bit_depth, std::vector<uint8_t>& out_bytes) {
            out_bytes.clear();
            size_t bytes_per_sample = bit_depth / 8;
            out_bytes.resize(sample_count * bytes_per_sample);

            _float_to_pcm_bytes(float_data, sample_count, bit_depth, out_bytes.data());
        }

        void float_to_pcm_bytes(const float* float_data, size_t sample_count, int bit_depth, Bytes **out_bytes) {
            size_t bytes_per_sample = bit_depth / 8;
            auto pcm_bytes = new Bytes(nullptr, sample_count * bytes_per_sample);
            *out_bytes = pcm_bytes;

            _float_to_pcm_bytes(float_data, sample_count, bit_depth, pcm_bytes->data);
        }

        void float_to_pcm_bytes(const float* float_data, size_t sample_count, int bit_depth, std::unique_ptr<Bytes> &out_bytes) {
            size_t bytes_per_sample = bit_depth / 8;
            out_bytes = std::make_unique<Bytes>(nullptr, sample_count * bytes_per_sample);

            _float_to_pcm_bytes(float_data, sample_count, bit_depth, out_bytes->data);
        }

        /**
         * @brief Convert a PCM sample to a float
         * @param pcm_data The PCM data
         * @param sample_count The number of samples
         * @param bit_depth The bit depth
         * @param float_out The output buffer
        */
        void pcm_bytes_to_float(const uint8_t* pcm_data, size_t sample_count, int bit_depth, std::vector<float>& float_out) {
            float_out.resize(sample_count);

            for (size_t i = 0; i < sample_count; ++i) {
                switch (bit_depth) {
                    case 8: {
                        uint8_t val = pcm_data[i];
                        float_out[i] = (val / 127.5f) - 1.0f;
                        break;
                    }
                    case 16: {
                        int16_t val;
                        std::memcpy(&val, &pcm_data[i * 2], 2);
                        float_out[i] = val / 32768.0f;
                        break;
                    }
                    case 24: {
                        const uint8_t* p = &pcm_data[i * 3];
                        int32_t val = (p[2] << 16) | (p[1] << 8) | p[0];
                        // sign-extend to 32-bit if necessary
                        if (val & 0x800000) val |= 0xFF000000;
                        float_out[i] = val / 8388608.0f;
                        break;
                    }
                    case 32: {
                        int32_t val;
                        std::memcpy(&val, &pcm_data[i * 4], 4);
                        float_out[i] = val / 2147483648.0f;
                        break;
                    }
                    default:
                        throw std::runtime_error("Unsupported bit depth");
                }
            }
        }

        /**
         * @brief Construct a new File object.
         * @param path wav or pcm file path
         * @param sample_rate sample rate, need to be filled in when parsing .pcm files
         * @param channels channels, need to be filled in when parsing .pcm files
         * @param bits_per_sample bits per sample, need to be filled in when parsing .pcm files
         * @maixpy maix.audio.File.__init__
        */
        File(int sample_rate = 16000, int channels = 1, int bits_per_sample = 16) {
            _sample_bits = bits_per_sample;
            _channels = channels;
            _sample_rate = sample_rate;
        }
        ~File() {

        }

        /**
         * Loads an audio file from a given file path.
         * @param path The file path to load the audio file from.
         * @param sample_rate The sample rate of the audio file. Only required for PCM files
         * @param channels The number of channels in the audio file. Only required for PCM files
         * @param bits_per_sample The number of bits per sample in the audio file. Only required for PCM files
         * @return An error code indicating whether the operation was successful or not.
         * @maixpy maix.audio.File.load
        */
        err::Err load(std::string path, int sample_rate = 16000, int channels = 1, int bits_per_sample = 16) {
            if (fs::exists(path)) {
                auto extension = fs::splitext(path)[1];
                if (extension == ".pcm") {
                    is_pcm_file = true;
                    return _load_pcm(path, sample_rate, channels, bits_per_sample);
                } else if (extension == ".wav") {
                    return _load_wav_aiff(path);
                } else if (extension == ".aiff") {
                    return _load_wav_aiff(path);
                } else {
                    log::error("Only files with the `.pcm` and `.wav` extensions are supported.");
                    return err::ERR_RUNTIME;
                }
            }
            return err::ERR_NONE;
        }

        /**
         * Saves an audio file to a given file path.
         * @param path The path to the file where the audio file will be saved.
         * @return An error code indicating whether the operation was successful or not.
         * @maixpy maix.audio.File.save
        */
        err::Err save(std::string path) {
            auto extension = fs::splitext(path)[1];
            if (extension == ".pcm") {
                is_pcm_file = true;
                return _save_pcm(path);
            } else if (extension == ".wav") {
                return _save_wav(path);
            } else if (extension == ".aiff") {
                return _save_aiff(path);
            } else {
                log::error("Only files with the `.pcm` and `.wav` extensions are supported.");
                return err::ERR_RUNTIME;
            }

            return err::ERR_NONE;
        }

        /**
         * Get pcm data
         * @return pcm data. datatype @see Bytes
         * @maixpy maix.audio.File.get_pcm
        */
        Bytes *get_pcm(bool copy = true) {
            return new maix::Bytes(pcm->data, pcm->data_len, false, copy);
        }

        /**
         * Set pcm data
         * @param new_pcm pcm data. datatype @see Bytes
         * @maixpy maix.audio.File.set_pcm
        */
        void set_pcm(Bytes *new_pcm, bool copy = true) {
            pcm = std::make_unique<maix::Bytes>(new_pcm->data, new_pcm->data_len, true, copy);
        }

        /**
         * Get sample bit
         * @param new_sample_bit if new_sample_bit > 0, set sample bit
         * @return current sample bit
         * @maixpy maix.audio.File.sample_bits
        */
        int sample_bits(int new_sample_bits = -1) {
            if (new_sample_bits > 0) {
                _sample_bits = new_sample_bits;
            }
            return _sample_bits;
        }

        /**
         * Get channels
         * @param new_channels if new_channels > 0, change channels
         * @return current channels
         * @maixpy maix.audio.File.channels
        */
        int channels(int new_channels = -1) {
            if (new_channels > 0) {
                _channels = new_channels;
            }
            return _channels;
        }

        /**
         * Get sample rate
         * @param new_sample_rate if new_sample_rate > 0, change sample rate
         * @return current sample rate
         * @maixpy maix.audio.File.sample_rate
        */
        int sample_rate(int new_sample_rate = -1) {
            if (new_sample_rate > 0) {
                _sample_rate = new_sample_rate;
            }
            return _sample_rate;
        }
    };

    /**
     * Recorder class
     * @maixpy maix.audio.Recorder
     */
    class Recorder
    {
        void *_param;
    public:
        /**
         * @brief Construct a new Recorder object. currectly only pcm and wav formats supported.
         * @param path record path. the path determines the location where you save the file, if path is none, the audio module will not save file.
         * @param sample_rate record sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format record sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel record sample channel, default is 1, means 1 channel sampling at the same time
         * @param block block record, default is true, means block record, if false, record will not block
         * @maixpy maix.audio.Recorder.__init__
         * @maixcdk maix.audio.Recorder.Recorder
         */
        Recorder(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1, bool block = true);
        ~Recorder();

        /**
         * Set/Get record volume
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume. range is [0, 100].
         * @return the current volume
         * @maixpy maix.audio.Recorder.volume
        */
        int volume(int value = -1);

        /**
         * Mute
         * @note MaixCAM2 dose not support this api.
         * @param data mute data, If you set this parameter to true, audio will set the value to mute,
         * if you don't, it will return the current mute status.
         * @return Returns whether mute is currently enabled.
         * @maixpy maix.audio.Recorder.mute
        */
        bool mute(int data = -1);

        /**
         * Reset record status
         * @param start start prepare audio data, default is True
         * @maixpy maix.audio.Recorder.reset
        */
        void reset(bool start = true);

        /**
         * Record, Read all cached data in buffer and return. If there is no audio data in the buffer, may return empty data.
         * @note Do not set the time too low, for example: 1ms, as the buffer may not be ready with audio data, which could corrupt the internal state.
         * @note In non-blocking mode, you need to actively execute reset() before you can start capturing audio.
         * Additionally, in non-blocking mode, if the buffer does not have enough data, only the currently prepared audio data will be returned.
         * As a result, the length of the actual output audio data may not match the length of the captured audio data.
         * @param record_ms Block and record audio data lasting `record_ms` milliseconds and save it to a file, the return value does not return audio data. Only valid if the initialisation `path` is set.
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.Recorder.record
        */
        maix::Bytes *record(int record_ms = -1);

        /**
         * Record, Read all cached data in buffer and return. If there is no audio data in the buffer, may return empty data.
         * @note This interface is experimental and may be removed in the future.
         * @note In non-blocking mode, you need to actively execute reset() before you can start capturing audio.
         * Additionally, in non-blocking mode, if the buffer does not have enough data, only the currently prepared audio data will be returned.
         * As a result, the length of the actual output audio data may not match the length of the captured audio data.
         * @param record_size Record audio data of size record_size.
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixcdk maix.audio.Recorder.record_bytes
        */
        maix::Bytes *record_bytes(int record_size = -1);

        /**
         * Finish the record, if you have passed in the path, this api will save the audio data to file.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Recorder.finish
        */
        err::Err finish();

        /**
         * Returns the number of bytes for frame_count frames.
         * @param frame_count frame count
         * @return frame bytes
         * @maixpy maix.audio.Recorder.frame_size
        */
        int frame_size(int frame_count = 1);

        /**
         * Return the number of frames available for reading during recording, unit is frame.
         * @note MaixCAM2 dose not support this api.
         * @note The number of bytes per frame can be calculated using frame_size().
         * @return remaining frames
         * @maixpy maix.audio.Recorder.get_remaining_frames
        */
        int get_remaining_frames();

        /**
         * Set/Get the audio buffer size, unit: frame.
         * @note Generally, the buffer size needs to be modified during non-blocking operations.
         * @note The number of bytes per frame can be calculated using frame_size().
         * @param period_size When period_size is less than 0, the current value of period_size will be returned;
         * when period_size is greater than 0, period_size will be updated, and the size of period_size after setting will be returned.
         * @return the current period size
         * @maixpy maix.audio.Recorder.period_size
        */
        int period_size(int period_size = -1);

        /**
         * Set/Get the audio buffer count, unit: frame.
         * @note Generally, the buffer size needs to be modified during non-blocking operations.
         * @param period_count When period_count is less than 0, the current value of period_count will be returned;
         * when period_count is greater than 0, period_count will be updated, and the size of period_count after setting will be returned.
         * @return the current period size
         * @maixpy maix.audio.Recorder.period_count
        */
        int period_count(int period_count = -1);

        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.Recorder.sample_rate
         */
        int sample_rate();

        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.Recorder.format
         */
        audio::Format format();

        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.Recorder.channel
         */
        int channel();
    };

    /**
     * Player class
     * @maixpy maix.audio.Player
     */
    class Player
    {
        void *_param;
    public:
        static maix::Bytes *NoneBytes;

        /**
         * @brief Construct a new Player object
         * @param path player path. the path determines the location where you save the file, if path is none, the audio module will not save file.
         * @param sample_rate player sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format player sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel player sample channel, default is 1, means 1 channel sampling at the same time
         * @param block block record, default is true, means block record, if false, record will not block
         * @maixpy maix.audio.Player.__init__
         * @maixcdk maix.audio.Player.Player
         */
        Player(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1, bool block = true);
        ~Player();

        /**
         * Set/Get player volume
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume. range is [0, 100].
         * @return the current volume
         * @maixpy maix.audio.Player.volume
        */
        int volume(int value = -1);

        /**
         * Play
         * @param data audio data, must be raw data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Player.play
        */
        err::Err play(maix::Bytes *data = maix::audio::Player::NoneBytes);

        /**
         * Returns the number of bytes for frame_count frames.
         * @param frame_count frame count
         * @return frame bytes
         * @maixpy maix.audio.Player.frame_size
        */
        int frame_size(int frame_count = 1);

        /**
         * Return the number of idle frames available for writing during playback, unit: frame. if there are no idle frames, it will cause blocking.
         * @note MaixCAM2 dose not support this api.
         * @note The number of bytes per frame can be calculated using frame_size().
         * @return remaining frames
         * @maixpy maix.audio.Player.get_remaining_frames
        */
        int get_remaining_frames();

        /**
         * Set/Get the audio buffer size, unit: frame.
         * @note Generally, the buffer size needs to be modified during non-blocking operations.
         * @note The number of bytes per frame can be calculated using frame_size().
         * @param period_size When period_size is less than 0, the current value of period_size will be returned;
         * when period_size is greater than 0, period_size will be updated, and the size of period_size after setting will be returned.
         * @return the current period size
         * @maixpy maix.audio.Player.period_size
        */
        int period_size(int period_size = -1);

        /**
         * Set/Get the audio buffer count, unit: frame.
         * @note Generally, the buffer size needs to be modified during non-blocking operations.
         * @param period_count When period_count is less than 0, the current value of period_count will be returned;
         * when period_count is greater than 0, period_count will be updated, and the size of period_count after setting will be returned.
         * @return the current period size
         * @maixpy maix.audio.Player.period_count
        */
        int period_count(int period_count = -1);

        /**
         * Reset player status
         * @param start start play audio data, default is False
         * @maixpy maix.audio.Player.reset
        */
        void reset(bool start = false);

        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.Player.sample_rate
         */
        int sample_rate();

        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.Player.format
         */
        audio::Format format();

        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.Player.channel
         */
        int channel();
    };
}

