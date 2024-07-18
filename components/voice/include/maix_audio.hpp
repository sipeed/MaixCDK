/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include <memory>

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
     * Recorder class
     * @maixpy maix.audio.Recorder
     */
    class Recorder
    {
        std::string _path;
        int _sample_rate;
        int _channel;
        audio::Format _format;

        void *_handle;
        void *_buffer;
        size_t _buffer_size;
        FILE *_file;
        int _period_size;
    public:
        /**
         * @brief Construct a new Recorder object. currectly only pcm and wav formats supported.
         * @param path record path. the path determines the location where you save the file, if path is none, the audio module will not save file.
         * @param sample_rate record sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format record sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel record sample channel, default is 1, means 1 channel sampling at the same time
         * @maixpy maix.audio.Recorder.__init__
         * @maixcdk maix.audio.Recorder.Recorder
         */
        Recorder(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1);
        ~Recorder();

        /**
         * Set/Get record volume
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.Recorder.volume
        */
        int volume(int value = -1);

        /**
         * Record, Read all cached data in buffer and return. If there is no audio data in the buffer, may return empty data.
         * @param record_ms Block and record audio data lasting `record_ms` milliseconds and save it to a file, the return value does not return audio data. Only valid if the initialisation `path` is set.
         * @return pcm data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.Recorder.record
        */
        maix::Bytes *record(int record_ms = -1);

        /**
         * Finish the record, if you have passed in the path, this api will save the audio data to file.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.Recorder.finish
        */
        err::Err finish();

        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.Recorder.sample_rate
         */
        int sample_rate() {
            return _sample_rate;
        }

        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.Recorder.format
         */
        audio::Format format() {
            return _format;
        }

        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.Recorder.channel
         */
        int channel() {
            return _channel;
        }
    };

    /**
     * Player class
     * @maixpy maix.audio.Player
     */
    class Player
    {
        std::string _path;
        int _sample_rate;
        int _channel;
        audio::Format _format;

        void *_handle;
        void *_buffer;
        size_t _buffer_size;
        FILE *_file;
        int _period_size;
    public:
        static maix::Bytes *NoneBytes;

        /**
         * @brief Construct a new Player object
         * @param path player path. the path determines the location where you save the file, if path is none, the audio module will not save file.
         * @param sample_rate player sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format player sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel player sample channel, default is 1, means 1 channel sampling at the same time
         * @maixpy maix.audio.Player.__init__
         * @maixcdk maix.audio.Player.Player
         */
        Player(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1);
        ~Player();

        /**
         * Set/Get player volume(Not support now)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
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
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.Player.sample_rate
         */
        int sample_rate() {
            return _sample_rate;
        }

        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.Player.format
         */
        audio::Format format() {
            return _format;
        }

        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.Player.channel
         */
        int channel() {
            return _channel;
        }
    };

     /**
     * Recorder class
     * @maixpy maix.audio.RecorderPause
     */
    class RecorderPause
    {
        std::string _path;
        int _sample_rate;
        int _channel;
        audio::Format _format;

        void *_handle;
        void *_buffer;
        size_t _buffer_size;
        FILE *_file;
    public:
        /**
         * @brief Create a recording class that allows recording to pause and continue
         * @param path record path. keep  null or "" path
         * @param sample_rate record sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format record sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel record sample channel, default is 1, means 1 channel sampling at the same time
         * @maixpy maix.audio.RecorderPause.__init__
         * @maixcdk maix.audio.RecorderPause.RecorderPause
         */
        RecorderPause(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1);
        ~RecorderPause();

        /**
         * Set/Get record volume
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.RecorderPause.volume
        */
        int volume(int value = -1);
        /**
         * Record, Read all cached data in buffer and return.
         * @param record_pcm_to_file_keep_pause record wav to file  With pause and end functions
         * @param flag_record Start and over flag bits for recording
         * @param flag_record_pause Pause and continue flag bits for recording
         * @return wav data. datatype @see Bytes. If you pass in record_ms parameter, the return value is an empty Bytes object.
         * @maixpy maix.audio.RecorderPause.record_pcm_to_file_keep_pause
        */
        maix::Bytes * record_pcm_to_file_keep_pause(std::string path_record_pcm_file,std::atomic<bool>& flag_record,std::atomic<bool>& flag_record_pause); 

        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.RecorderPause.sample_rate
         */
        int sample_rate() {
            return _sample_rate;
        }

        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.RecorderPause.format
         */
        audio::Format format() {
            return _format;
        }

        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.RecorderPause.channel
         */
        int channel() {
            return _channel;
        }
    };

    /**
     * PlayerBlock class
     * @maixpy maix.audio.PlayerBlock
     */
    class PlayerBlock
    {
        std::string _path;
        int _sample_rate;
        int _channel;
        audio::Format _format;

        void *_handle;
        void *_buffer;
        size_t _buffer_size;
        FILE *_file;
    public:
        static maix::Bytes *NoneBytes;

        /**
         * @brief Create a blocking playback class with a buffer size of 1024
         * @param path player path. null or ""(current only byte data can be played).
         * @param sample_rate player sample rate, default is 48000(48KHz), means 48000 samples per second.
         * @param format player sample format, default is audio::Format::FMT_S16_LE, means sampling 16 bits at a time and save as signed 16 bits, little endian. see @audio::Format
         * @param channel player sample channel, default is 1, means 1 channel sampling at the same time
         * @maixpy maix.audio.PlayerBlock.__init__
         * @maixcdk maix.audio.PlayerBlock.PlayerBlock
         */
        PlayerBlock(std::string path = std::string(), int sample_rate = 48000, audio::Format format = audio::Format::FMT_S16_LE, int channel = 1);
        ~PlayerBlock();

        /**
         * Set/Get player volume(It's inverted, the gear is 0-32, you set the number to be 32- the gear you set)
         * @param value volume value, If you use this parameter, audio will set the value to volume,
         * if you don't, it will return the current volume.
         * @return the current volume
         * @maixpy maix.audio.PlayerBlock.volume
        */
        int volume(int value = -1);
        /**
         * Simply drop the buffer and stop the device
         * @maixpy maix.audio.PlayerBlock.audio_drop
        */
        void audio_drop();
        /**
         * Wait for buffer data to finish playing before stopping the device
         * @maixpy maix.audio.PlayerBlock.audio_drain
        */
        void audio_drain();
        /**
         * Make the device ready
         * @maixpy maix.audio.PlayerBlock.audio_prepare
        */
        void audio_prepare();
        /**
         * Device stop and continue (currently not supported)
         * @maixpy maix.audio.PlayerBlock.audio_pause
        */
        int audio_pause(int enable=1);
        /**
         * play_data
         * @param data audio data, must be raw data
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.audio.PlayerBlock.play_data
        */
        err::Err play_data(maix::Bytes *data = maix::audio::Player::NoneBytes);
        /**
         * Get sample rate
         * @return returns sample rate
         * @maixpy maix.audio.PlayerBlock.sample_rate
         */
        int sample_rate() {
            return _sample_rate;
        }

        /**
         * Get sample format
         * @return returns sample format
         * @maixpy maix.audio.PlayerBlock.format
         */
        audio::Format format() {
            return _format;
        }

        /**
         * Get sample channel
         * @return returns sample channel
         * @maixpy maix.audio.PlayerBlock.channel
         */
        int channel() {
            return _channel;
        }
    };
}

