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
     * @maixpy maix.audio.AudioFileReader
    */
    class AudioFileReader
    {
    private:
        int _channels;
        int _sample_rate;
        int _sample_bits;
        int _data_size;
        std::unique_ptr<maix::Bytes> _pcm;
        typedef struct {
            unsigned long fmt_size;
            unsigned short fmt_id;
            unsigned short channels;
            unsigned long sample_rate;
            unsigned short bits_per_sample;
            unsigned short block_align;
            unsigned long bytes_per_sec;
            unsigned long data_size;  // size of pcm
        } wav_header_t;

        err::Err read_wav_header(std::string path, wav_header_t &header, int &header_seek) {
            err::Err ret = err::ERR_NONE;
            char ch[5];
            unsigned int size;

            FILE *fp = fopen(path.c_str(), "rb");
            if (!fp) {
                return err::ERR_RUNTIME;
            }

			fread(ch, 1, 4, fp);
			ch[4] = '\0';
			if (strcmp(ch, "RIFF")) {
				fclose(fp);
				return err::ERR_RUNTIME;
			}

			fseek(fp, 4, fs::SEEK_CUR);
			fread(ch, 1, 4, fp);
			ch[4] = '\0';
			if (strcmp(ch, "WAVE")) {
				fclose(fp);
				return err::ERR_RUNTIME;
			}

			fseek(fp, 4, fs::SEEK_CUR);
			fread(&header.fmt_size, 4, 1, fp);
			fread(&header.fmt_id, 2, 1, fp);
			fread(&header.channels, 2, 1, fp);
			fread(&header.sample_rate, 4, 1, fp);
			fread(&header.bytes_per_sec, 4, 1, fp);
			fread(&header.block_align, 2, 1, fp);
			fread(&header.bits_per_sample, 2, 1, fp);
			fseek(fp, header.fmt_size - 16, fs::SEEK_CUR);
			fread(ch, 1, 4, fp);
			while (strcmp(ch, "data")) {
				if (fread(&size, 4, 1, fp) != 1) {
					fclose(fp);
					return err::ERR_RUNTIME;
				}
				fseek(fp, size, fs::SEEK_CUR);
				fread(ch, 1, 4, fp);
			}
			fread(&header.data_size, 4, 1, fp);

			if (header.bits_per_sample != 8 && header.bits_per_sample != 16) {
				fclose(fp);
				return err::ERR_RUNTIME;
			}

			if (header.channels != 1 && header.channels != 2) {
				fclose(fp);
				return err::ERR_RUNTIME;
			}

            header_seek = ftell(fp);

            fclose(fp);
            return ret;
        }

        err::Err read_wav(std::string path) {
            err::Err ret = err::ERR_NONE;

            wav_header_t wav_header;
            int header_seek = 0;
            ret = read_wav_header(path, wav_header, header_seek);
            if (ret != err::ERR_NONE) {
                log::error("read_wav_header error");
                return ret;
            }

            auto new_file = fopen(path.c_str(), "rb");
            if (!new_file) {
                log::error("open wav file failed!");
                return err::ERR_RUNTIME;
            }

            fseek(new_file, header_seek, fs::SEEK_SET);

            _channels = wav_header.channels;
            _sample_rate = wav_header.sample_rate;
            _data_size = wav_header.data_size;
            _sample_bits = wav_header.bits_per_sample;

            _pcm = std::make_unique<maix::Bytes>(nullptr, _data_size);
            if (!_pcm) {
                log::error("allocate memory failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }

            if (fread(_pcm->data, 1, _data_size, new_file) != (size_t)_data_size) {
                log::error("read wav file failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }
            fclose(new_file);

            return ret;
        }


        err::Err read_pcm(std::string path, int sample_rate = 16000, int channels = 1, int bits_per_sample = 16) {
            err::Err ret = err::ERR_NONE;

            auto new_file = fopen(path.c_str(), "rb");
            if (!new_file) {
                log::error("open wav file failed!");
                return err::ERR_RUNTIME;
            }
            fseek(new_file, 0, fs::SEEK_END);
            _data_size = ftell(new_file);
            auto bytes_per_frame = channels * bits_per_sample / 8;
            _data_size = (_data_size / bytes_per_frame) * bytes_per_frame;
            _sample_rate = sample_rate;
            _channels = channels;
            _sample_bits = bits_per_sample;

            _pcm = std::make_unique<maix::Bytes>(nullptr, _data_size);
            if (!_pcm) {
                log::error("allocate memory failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }
            fseek(new_file, 0, fs::SEEK_SET);
            if (fread(_pcm->data, 1, _data_size, new_file) != (size_t)_data_size) {
                log::error("read wav file failed!");
                fclose(new_file);
                return err::ERR_RUNTIME;
            }
            fclose(new_file);

            return ret;
        }
    public:
        /**
         * @brief Construct a new AudioFileReader object.
         * @param path wav or pcm file path
         * @param sample_rate sample rate, need to be filled in when parsing .pcm files
         * @param channels channels, need to be filled in when parsing .pcm files
         * @param bits_per_sample bits per sample, need to be filled in when parsing .pcm files
         * @maixpy maix.audio.AudioFileReader.__init__
        */
        AudioFileReader(std::string path, int sample_rate = 16000, int channels = 1, int bits_per_sample = 16) {
            err::check_bool_raise(path.size() > 0, "path is empty");
            _sample_bits = bits_per_sample;
            _channels = channels;
            _sample_rate = sample_rate;

            if (fs::exists(path)) {
                auto extension = fs::splitext(path)[1];
                if (extension == ".wav") {
                    err::check_raise(read_wav(path));
                } else if (extension == ".pcm") {
                    err::check_raise(read_pcm(path, _sample_rate, _channels, _sample_bits));
                } else {
                    err::check_raise(err::ERR_NOT_FOUND, "Only files with the `.pcm` and `.wav` extensions are supported.");
                }
            }
        }
        ~AudioFileReader() {

        }

        /**
         * Get pcm data
         * @return pcm data. datatype @see Bytes
         * @maixpy maix.audio.AudioFileReader.pcm
        */
        Bytes *pcm(bool copy = true) {
            return new maix::Bytes(_pcm->data, _pcm->data_len, false, copy);
        }

        /**
         * Get sample bit
         * @return sample bit
         * @maixpy maix.audio.AudioFileReader.sample_bits
        */
        int sample_bits() {return _sample_bits;}

        /**
         * Get sample bit
         * @return sample bit
         * @maixpy maix.audio.AudioFileReader.channels
        */
        int channels() {return _channels;}

        /**
         * Get sample rate
         * @return sample rate
         * @maixpy maix.audio.AudioFileReader.sample_rate
        */
        int sample_rate() {return _sample_rate;}

        /**
         * Get data size
         * @return data size
         * @maixpy maix.audio.AudioFileReader.data_size
        */
        int data_size() {return _data_size;}
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

