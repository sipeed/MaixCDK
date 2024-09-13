/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include "maix_image.hpp"
#include "maix_display.hpp"
#include "maix_camera.hpp"
#include "maix_audio.hpp"
#include <memory>

/**
 * @brief maix.video module
 * @maixpy maix.video
*/
namespace maix::video
{
    /**
     * Video type
     * @maixpy maix.video.VideoType
     */
    enum VideoType
    {
        VIDEO_NONE = 0,  // format invalid
        VIDEO_ENC_H265_CBR,     // Deprecated
        VIDEO_ENC_MP4_CBR,      // Deprecated
        VIDEO_DEC_H265_CBR,     // Deprecated
        VIDEO_DEC_MP4_CBR,      // Deprecated
        VIDEO_H264_CBR,         // Deprecated
        VIDEO_H265_CBR,         // Deprecated
        VIDEO_H264_CBR_MP4,     // Deprecated
        VIDEO_H265_CBR_MP4,     // Deprecated

        VIDEO_H264,
        VIDEO_H264_MP4,
        VIDEO_H264_FLV,
        VIDEO_H265,
        VIDEO_H265_MP4,
    };

    /**
     * Video type
     * @maixpy maix.video.MediaType
     */
    enum MediaType
    {
        MEDIA_TYPE_UNKNOWN = -1,    // Represents an unknown media type, which is usually treated as AVMEDIA_TYPE_DATA.
        MEDIA_TYPE_VIDEO,           // Represents a video stream, such as video content encoded in H.264, MPEG-4, etc.
        MEDIA_TYPE_AUDIO,           // Represents an audio stream, such as audio content encoded in AAC, MP3, etc.
        MEDIA_TYPE_DATA,            // Represents opaque data streams that are usually continuous. This type of stream is not necessarily audio or video and may be used for other data purposes.
        MEDIA_TYPE_SUBTITLE,        // Represents a subtitle stream used for displaying text or subtitle information, such as SRT, ASS, etc.
        MEDIA_TYPE_ATTACHMENT,      // Represents attachment streams that are usually sparse. Attachment streams can include images, fonts, or other files that need to be bundled with the media.
        MEDIA_TYPE_NB               // Represents the number of media types (count) and indicates the total number of media types defined in this enumeration. It is not a media type itself but is used for counting enumeration items.
    };

    /**
     * @brief Convert a value in timebase units to microseconds. value * 1000000 / (timebase[1] / timebase[0])
     * @param timebse Time base, used as the unit for calculating playback time. It must be an array containing two parameters,
     * in the format [num, den], where the first parameter is the numerator of the time base, and the second parameter is the denominator of the time base.
     * @param value Input value
     * @return Return the result in microseconds.
     * @maixpy maix.video.timebase_to_us
    */
    double timebase_to_us(std::vector<int> timebase, uint64_t value);

    /**
     * @brief Convert a value in timebase units to milliseconds.
     * @param timebse Time base, used as the unit for calculating playback time. It must be an array containing two parameters,
     * in the format [num, den], where the first parameter is the numerator of the time base, and the second parameter is the denominator of the time base.
     * @param value Input value
     * @return Return the result in milliseconds.
     * @maixpy maix.video.timebase_to_ms
    */
    double timebase_to_ms(std::vector<int> timebase, uint64_t value);

    /**
     * Context class
     * @maixpy maix.video.Context
     */
    class Context
    {
    public:
        /**
         * @brief Construct a new Context object
         * @param media_type enable capture, if true, you can use capture() function to get an image object
         * @param timebase Time base, used as the unit for calculating playback time. It must be an array containing two parameters,
         * in the format [num, den], where the first parameter is the numerator of the time base, and the second parameter is the denominator of the time base.
         * @maixpy maix.video.Context.__init__
         * @maixcdk maix.video.Context.Context
         */
        Context(video::MediaType media_type, std::vector<int> timebase) {
            _media_type = media_type;

            if (timebase.size() < 2) {
                _timebase = {0, 0};
            } else {
                _timebase = timebase;
            }

            _pcm = NULL;
            _image = NULL;
        }

        /**
         * @brief Construct a new Context object
         * @param media_type enable capture, if true, you can use capture() function to get an image object
         * @param timebase Time base, used as the unit for calculating playback time. It must be an array containing two parameters,
         * in the format [num, den], where the first parameter is the numerator of the time base, and the second parameter is the denominator of the time base.
         * @param sample_rate sampling rate of audio
         * @param format audio format
         * @param channels number of audio channels
         * @maixcdk maix.video.Context.Context
         */
        Context(video::MediaType media_type, std::vector<int> timebase, int sample_rate, audio::Format format, int channels) {
            _media_type = media_type;

            if (timebase.size() < 2) {
                _timebase = {0, 0};
            } else {
                _timebase = timebase;
            }

            _audio_sample_rate = sample_rate;
            _audio_format = format;
            _audio_channels = channels;
            _pcm = NULL;
            _image = NULL;
        }

        ~Context() {
            if (_media_type == video::MEDIA_TYPE_VIDEO) {
                if (_image) {
                    delete _image;
                    _image = NULL;
                }
            } else if (_media_type == video::MEDIA_TYPE_AUDIO) {
                if (_pcm) {
                    delete _pcm;
                    _pcm = NULL;
                }
            }
        }

        /**
         * @brief Get sample rate of audio (only valid in the context of audio)
         * @return sample rate
         * @maixpy maix.video.Context.audio_sample_rate
        */
        int audio_sample_rate() { return _audio_sample_rate; }

        /**
         * @brief Get channels of audio (only valid in the context of audio)
         * @return channels
         * @maixpy maix.video.Context.audio_channels
        */
        int audio_channels() { return _audio_channels; }

        /**
         * @brief Get format of audio (only valid in the context of audio)
         * @return audio format. @see audio::Format
         * @maixpy maix.video.Context.audio_format
        */
        audio::Format audio_format() { return _audio_format; }

        /**
         * @brief Set pcm data (only valid in the context of audio)
         * @param duration Duration of the current pcm. unit: timebase
         * @param pts The start time of this pcm playback. If it is 0, it means this parameter is not supported. unit: timebase
         * @return err::Err
         * @maixpy maix.video.Context.set_pcm
        */
        err::Err set_pcm(maix::Bytes *data, int duration = 0, uint64_t pts = 0, bool copy = true) {
            maix::Bytes *bytes = NULL;
            bytes = new maix::Bytes(data->data, data->size(), true, copy);
            err::check_null_raise(bytes, "set_pcm failed");
            _pcm = bytes;
            _duration = duration < 0 ? 0 : duration;
            _pts = pts < 0 ? 0 : pts;
            _last_pts = 0;
            return err::ERR_NONE;
        }

        /**
        * @brief Get pcm data (only valid in the context of audio)
        * @attention Note that if you call this interface, you are responsible for releasing the memory of the data, and this interface cannot be called again.
        * @return Bytes
        * @maixpy maix.video.Context.get_pcm
        */
        Bytes *get_pcm() {
            maix::Bytes *bytes_out = _pcm;
            _pcm = NULL;
            return bytes_out;
        }

        /**
         * @brief Set image info
         * @param image image data
         * @param duration Duration of the current image. unit: timebase
         * @param pts The start time of this image playback. If it is 0, it means this parameter is not supported. unit: timebase
         * @param last_pts The start time of the previous image playback. It can be used to ensure the playback order. If it is 0, it means this parameter is not supported. unit: timebase
         * @maixcdk maix.video.Context.set_image
         */
        void set_image(image::Image *image, int duration = 0, uint64_t pts = 0, uint64_t last_pts = 0) {
            _image = image;
            _duration = duration < 0 ? 0 : duration;
            _pts = pts < 0 ? 0 : pts;
            _last_pts = last_pts < 0 ? 0 : last_pts;
        }

        /**
         * @brief Retrieve the image data to be played.
         * @attention Note that if you call this interface, you are responsible for releasing the memory of the image, and this interface cannot be called again.
         * @maixpy maix.video.Context.image
        */
        image::Image *image() {
            image::Image *out = _image;
            _image = NULL;
            return out;
        }

        /**
         * @brief Get the media type to determine whether it is video, audio, or another media type.
         * @maixpy maix.video.Context.media_type
        */
        video::MediaType media_type() {
            return _media_type;
        }

        /**
         * @brief Get the start time of the current playback., in units of time base.
         * @maixpy maix.video.Context.pts
        */
        uint64_t pts() {
            return _pts;
        }

        /**
         * @brief Get the start time of the previous playback, in units of time base.
         * @maixpy maix.video.Context.last_pts
        */
        uint64_t last_pts() {
            return _last_pts;
        }

        /**
         * @brief Get the time base.
         * @maixpy maix.video.Context.timebase
        */
        std::vector<int> timebase() {
            return _timebase;
        }

        /**
         * @brief Duration of the current frame. unit: timebase
         * @maixpy maix.video.Context.duration
        */
        int duration() {
            return _duration;
        }

        /**
         * @brief Duration of the current frame. unit: us
         * @maixpy maix.video.Context.duration_us
        */
        uint64_t duration_us() {
            return _duration * 1000000 / (_timebase[1] / _timebase[0]);
        }
        private:
        video::MediaType _media_type;
        image::Image *_image;
        uint64_t _pts;
        uint64_t _last_pts;
        std::vector<int> _timebase; // [den, num], timebase = den / num
        int _duration;

        int _audio_sample_rate;
        audio::Format _audio_format;
        int _audio_channels;
        Bytes *_pcm;
    };

    /**
     * Frame class
     * @maixpy maix.video.Frame
     */
    class Frame
    {
        uint64_t _pts;                        // unit: time_base
        uint64_t _dts;                        // unit: time_base
        uint64_t _duration;                   // equals next_pts - this_pts in presentation order. unit: time_base.
        uint8_t *_data;
        size_t _data_size;
        video::VideoType _type;
        bool _is_alloc;
    public:
        /**
         * @brief Frame object
         * @param data src data pointer, use pointers directly without copying.
         * Note: this object will try to free this memory
         * @param len data len
         * @param pts presentation time stamp. unit: time_base
         * @param dts decoding time stamp. unit: time_base
         * @param duration packet display time. unit: time_base (not used)
         * @param auto_detele if true, will delete data when destruct. When copy is true, this arg will be ignore.
         * @param copy data will be copy to new buffer if true, if false, will use data directly,
         *             default true to ensure memory safety.
         * @maixcdk maix.video.Frame.Frame
         */
        Frame(uint8_t *data, int len, uint64_t pts = -1, uint64_t dts = -1, int64_t duration = 0, bool auto_detele = false, bool copy = false) {
            _data = data;
            _data_size = _data ? len : 0;
            _pts = pts;
            _dts = dts;
            _duration = duration;
            if(len > 0)
            {
                if(data && copy)
                {
                    _data = (uint8_t *)malloc(_data_size);
                    _is_alloc = true;
                    memcpy(_data, data, _data_size);
                } else {
                    _is_alloc = auto_detele;
                }
            } else {
                _is_alloc = false;
            }
        }

        /**
         * @brief Frame number (pair of numerator and denominator).
         * @maixcdk maix.video.Frame.Frame
         */
        Frame() {
            _data = NULL;
            _data_size = 0;
            _is_alloc = false;
        }

        ~Frame() {
            if (_is_alloc && _data)
            {
                free(_data);
                _data = nullptr;
            }
        }

        /**
         * @brief Get raw data of packet
         * @param data data pointer
         * @param len data length pointer
         * @return raw data
         * @maixcdk maix.video.Frame.get
         */
        err::Err get(void **data, int *len) {
            if (data) *data = _data;
            if (len) *len = _data_size;
            return err::ERR_NONE;
        }

        /**
         * @brief Get raw data of packet
         * @param copy if true, will alloc memory and copy data to new buffer
         * @return raw data
         * @maixpy maix.video.Frame.to_bytes
         */
        Bytes *to_bytes(bool copy = false) {
            Bytes *b = NULL;
            if (copy) {
                b = new Bytes(_data, _data_size, true, true);
            } else {
                b = new Bytes(_data, _data_size, false, false);
            }
            return b;
        }

        /**
         * @brief Get raw data of packet
         * @return raw data
         * @maicdk maix.video.Frame.data
         */
        uint8_t *data() {
            return _data;
        }

        /**
         * @brief Get raw data size of packet
         * @return size of raw data
         * @maixpy maix.video.Frame.size
         */
        size_t size() {
            return _data_size;
        }

        /**
         * @brief Check packet is valid
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Frame.is_valid
         */
        bool is_valid() {
            return (_data && _data_size != 0) ? true : false;
        }

        /**
         * @brief Set pts
         * @param pts presentation time stamp. unit: time_base
         * @maixpy maix.video.Frame.set_pts
         */
        void set_pts(uint64_t pts) {
            _pts = pts;
        }

        /**
         * @brief Set dts
         * @param dts decoding time stamp.  unit: time_base
         * @maixpy maix.video.Frame.set_dts
         */
        void set_dts(uint64_t dts) {
            _dts = dts;
        }

        /**
         * @brief Set duration
         * @param duration packet display time. unit: time_base
         * @maixpy maix.video.Frame.set_duration
         */
        void set_duration(uint64_t duration) {
            _duration = duration;
        }

        /**
         * @brief Set pts
         * @param pts presentation time stamp. unit: time_base
         * @return pts value
         * @maixpy maix.video.Frame.get_pts
         */
        uint64_t get_pts() {
            return _pts;
        }

        /**
         * @brief Set dts
         * @param dts decoding time stamp.  unit: time_base
         * @return dts value
         * @maixpy maix.video.Frame.get_dts
         */
        uint64_t get_dts() {
            return _dts;
        }

        /**
         * @brief Get duration
         * @return duration value
         * @maixpy maix.video.Frame.get_duration
         */
        uint64_t get_duration() {
            return _duration;
        }

        /**
         * @brief Get frame type
         * @return video type. @see video::VideoType
         * @maixpy maix.video.Frame.type
         */
        video::VideoType type() {
            return _type;
        }
    };

    /**
     * Packet class
     * @maixpy maix.video.Packet
    */
    class Packet
    {
        uint64_t _pts;                        // unit: time_base
        uint64_t _dts;                        // unit: time_base
        uint64_t _duration;                   // equals next_pts - this_pts in presentation order. unit: time_base.
        uint8_t *_data;
        size_t _data_size;
    public:
        /**
         * @brief Packet number (pair of numerator and denominator).
         * @param data src data pointer, use pointers directly without copying.
         * Note: this object will try to free this memory
         * @param len data len
         * @param pts presentation time stamp. unit: time_base
         * @param dts decoding time stamp. unit: time_base
         * @param duration packet display time. unit: time_base
         * @maixpy maix.video.Packet.__init__
         * @maixcdk maix.video.Packet.Packet
         */
        Packet(uint8_t *data, int len, uint64_t pts = -1, uint64_t dts = -1, int64_t duration = 0) {
            _data = data;
            _data_size = _data ? len : 0;
            _pts = pts;
            _dts = dts;
            _duration = duration;
        }

        /**
         * @brief Packet number (pair of numerator and denominator).
         * @maixcdk maix.video.Packet.Packet
         */
        Packet() {
            _data = NULL;
            _data_size = 0;
        }

        ~Packet() {
            if (_data) {
                free(_data);
                _data = NULL;
            }
        }

        /**
         * @brief Get raw data of packet
         * @return raw data
         * @maixpy maix.video.Packet.get
         */
        std::vector<uint8_t> get() {
            std::vector<uint8_t> vec(_data, _data + _data_size);
            return vec;
        }

        /**
         * @brief Get raw data of packet
         * @return raw data
         * @maixpy maix.video.Packet.data
         */
        uint8_t *data() {
            return _data;
        }

        /**
         * @brief Get raw data size of packet
         * @return size of raw data
         * @maixpy maix.video.Packet.data_size
         */
        size_t data_size() {
            return _data_size;
        }

        /**
         * @brief Check packet is valid
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.is_valid
         */
        bool is_valid() {
            return (_data && _data_size != 0) ? true : false;
        }

        /**
         * @brief Set pts
         * @param pts presentation time stamp. unit: time_base
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.set_pts
         */
        void set_pts(uint64_t pts) {
            _pts = pts;
        }

        /**
         * @brief Set dts
         * @param dts decoding time stamp.  unit: time_base
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.set_dts
         */
        void set_dts(uint64_t dts) {
            _dts = dts;
        }

        /**
         * @brief Set duration
         * @param duration packet display time. unit: time_base
         * @return true, packet is valid; false, packet is invalid
         * @maixpy maix.video.Packet.set_duration
         */
        void set_duration(uint64_t duration) {
            _duration = duration;
        }
    };

    /**
     * Encode class
     * @maixpy maix.video.Encoder
    */
    class Encoder
    {
    public:
        static maix::image::Image *NoneImage;

        /**
         * @brief Construct a new Video object
         * @param width picture width. this value may be set automatically. default is 2560.
         * @param height picture height. this value may be set automatically. default is 1440.
         * @param format picture format. default is image::Format::FMT_YVU420SP. @see image::Format
         * @param type video encode/decode type. default is ENC_H265_CBR. @see EncodeType
         * @param framerate frame rate. framerate default is 30, means 30 frames per second
         * for video. 1/time_base is not the average frame rate if the frame rate is not constant.
         * @param gop for h264/h265 encoding, the interval between two I-frames, default is 50.
         * @param bitrate for h264/h265 encoding, used to limit the bandwidth used by compressed data, default is 3000kbps
         * @param time_base frame time base. time_base default is 1000, means 1/1000 ms (not used)
         * @param capture enable capture, if true, you can use capture() function to get an image object
         * @param block This parameter determines whether encoding should block until it is complete.
         * If set to true, it will wait until encoding is finished before returning.
         * If set to false, it will return the current encoding result on the next call.
         * @maixpy maix.video.Encoder.__init__
         * @maixcdk maix.video.Encoder.Encoder
         */
        Encoder(std::string path = "", int width = 2560, int height = 1440, image::Format format = image::Format::FMT_YVU420SP, video::VideoType type = video::VideoType::VIDEO_H264, int framerate = 30, int gop = 50, int bitrate = 3000 * 1000, int time_base = 1000, bool capture = false, bool block = true);
        ~Encoder();

        /**
         * @brief Bind camera
         * @param camera camera object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Encoder.bind_camera
        */
        err::Err bind_camera(camera::Camera *camera);

        /**
         * Encode image.
         * @param img the image will be encode.
         * if the img is NULL, this function will try to get image from camera, you must use bind_camera() function to bind the camera.
         * @return encode result
         * @maixpy maix.video.Encoder.encode
        */
        video::Frame *encode(image::Image *img = maix::video::Encoder::NoneImage);

        /**
         * Capture image
         * @attention Each time encode is called, the last captured image will be released.
         * @return error code
         * @maixpy maix.video.Encoder.capture
        */
        image::Image *capture() {
            err::check_null_raise(_capture_image, "Can't capture image, please make sure the capture flag is set, and run this api after encode().");
            image::Image *new_image = new image::Image(_capture_image->width(), _capture_image->height(), _capture_image->format(),
                (uint8_t *)_capture_image->data(), _capture_image->data_size(), false);
            return new_image;
        }

        /**
         * Get video width
         * @return video width
         * @maixpy maix.video.Encoder.width
        */
        int width()
        {
            return _width;
        }

        /**
         * Get video height
         * @return video height
         * @maixpy maix.video.Encoder.height
        */
        int height()
        {
            return _height;
        }

        /**
         * Get video encode type
         * @return VideoType
         * @maixpy maix.video.Encoder.type
        */
        video::VideoType type()
        {
            return _type;
        }

        /**
         * Get video encode framerate
         * @return frame rate
         * @maixpy maix.video.Encoder.framerate
        */
        int framerate()
        {
            return _framerate;
        }

        /**
         * Get video encode gop
         * @return gop value
         * @maixpy maix.video.Encoder.gop
        */
        int gop()
        {
            return _gop;
        }

        /**
         * Get video encode bitrate
         * @return bitrate value
         * @maixpy maix.video.Encoder.bitrate
        */
        int bitrate()
        {
            return _bitrate;
        }

        /**
         * Get video encode time base
         * @return time base value
         * @maixpy maix.video.Encoder.time_base
        */
        int time_base()
        {
            return _time_base;
        }

        /**
         * Get current pts, unit: time_base
         * Note: The current default is to assume that there is no B-frame implementation, so pts and bts are always the same
         * @param time_ms start time from the first frame. unit: ms
         * @return time base value
         * @maixcdk maix.video.Encoder.get_pts
        */
        uint64_t get_pts(uint64_t time_ms)
        {
            return time_ms * 1000 / _time_base;
        }

        /**
         * Get current dts, unit: time_base
         * Note: The current default is to assume that there is no B-frame implementation, so pts and bts are always the same
         * @param time_ms start time from the first frame. unit: ms
         * @return time base value
         * @maixcdk maix.video.Encoder.get_dts
        */
        uint64_t get_dts(uint64_t time_ms)
        {
            return time_ms * 1000 / _time_base;
        }
    private:
        std::string _path;
        int _width;
        int _height;
        image::Format _format;
        video::VideoType _type;
        bool _block;
        int _framerate;
        int _gop;
        int _bitrate;
        int _time_base;
        bool _need_capture;
        image::Image *_capture_image;
        camera::Camera *_camera;
        bool _bind_camera;
        uint64_t _pts;                        // unit: time_base
        uint64_t _dts;                        // unit: time_base
        uint64_t _start_encode_ms;
        bool _encode_started;
        void *_param;
    };


    /**
     * Decoder class
     * @maixpy maix.video.Decoder
     */
    class Decoder
    {
    public:
        /**
         * @brief Construct a new decoder object
         * @param path Path to the file to be decoded. Supports files with .264 and .mp4 extensions. Note that only mp4 files containing h.264 streams are supported.
         * @param format Decoded output format, currently only support YUV420SP
         * @maixpy maix.video.Decoder.__init__
         * @maixcdk maix.video.Decoder.Decoder
         */
        Decoder(std::string path, image::Format format = image::Format::FMT_YVU420SP);
        ~Decoder();

        /**
         * Decode the video stream, returning the image of the next frame each time.
         * @param block Whether it blocks or not. If true, it will wait for the decoding to complete and return the current frame.
         * If false, it will return the result of the previous frame's decoding. If the previous frame's decoding result is empty,
         * it will return an unknown Context, and you can use the media_type method of the Context to determine if a valid result exists.
         * default is true.
         * @return Decoded context information.
         * @maixpy maix.video.Decoder.decode_video
        */
        video::Context * decode_video(bool block = true);

        /**
         * Decode the video stream, returning the image of the next frame each time.
         * @return Decoded context information.
         * @maixpy maix.video.Decoder.decode_audio
        */
        video::Context * decode_audio();

        /**
         * Decode the video and audio stream
         * @param block Whether it blocks or not. If true, it will wait for the decoding to complete and return the current frame.
         * If false, it will return the result of the previous frame's decoding. If the previous frame's decoding result is empty,
         * it will return an unknown Context, and you can use the media_type method of the Context to determine if a valid result exists.
         * default is true.
         * @return Decoded context information.
         * @maixpy maix.video.Decoder.decode
        */
        video::Context * decode(bool block = true);

        /**
         * @brief Get sample rate of audio (only valid in the context of audio)
         * @return sample rate
         * @maixpy maix.video.Context.audio_sample_rate
        */
        int audio_sample_rate() { return _audio_sample_rate; }

        /**
         * @brief Get channels of audio (only valid in the context of audio)
         * @return channels
         * @maixpy maix.video.Context.audio_channels
        */
        int audio_channels() { return _audio_channels; }

        /**
         * @brief Get format of audio (only valid in the context of audio)
         * @return audio format. @see audio::Format
         * @maixpy maix.video.Context.audio_format
        */
        audio::Format audio_format() { return _audio_format; }

        /**
         * @brief Get the video width
         * @return video width
         * @maixpy maix.video.Decoder.width
        */
        int width()
        {
            return _width;
        }

        /**
         * @brief Get the video height
         * @return video height
         * @maixpy maix.video.Decoder.height
        */
        int height()
        {
            return _height;
        }

        /**
         * @brief Get the video bitrate
         * @return bitrate value
         * @maixpy maix.video.Decoder.bitrate
        */
        int bitrate()
        {
            return _bitrate;
        }

        /**
         * @brief Get the video fps
         * @return fps value
         * @maixpy maix.video.Decoder.fps
        */
        int fps()
        {
            return _fps;
        }

        /**
         * @brief Seek to the required playback position
         * @param time timestamp value, unit: s
         * @return return the current position, unit: s
         * @maixpy maix.video.Decoder.seek
        */
        double seek(double time = -1);

        /**
         * @brief Get the maximum duration of the video. If it returns 0, it means it cannot be predicted.
         * @return duration value, unit: s
         * @maixpy maix.video.Decoder.duration
        */
        double duration();

        /**
         * @brief Get the time base.
         * @maixpy maix.video.Decoder.timebase
        */
        std::vector<int> timebase() {
            return _timebase;
        }

        /**
         * @brief If find audio data, return true
         * @maixpy maix.video.Decoder.has_audio
        */
        bool has_audio() {
            return _has_audio;
        }

        /**
         * @brief If find video data, return true
         * @maixpy maix.video.Decoder.has_video
        */
        bool has_video() {
            return _has_video;
        }
    private:
        std::string _path;
        int _width;
        int _height;
        int _bitrate;
        int _fps;
        int _has_audio;
        int _has_video;
        uint64_t _last_pts;
        image::Format _format_out;
        std::vector<int> _timebase;

        int _audio_sample_rate;
        audio::Format _audio_format;
        int _audio_channels;

        void *_param;
    };

    /**
     * Video class
     * @maixpy maix.video.Video
     */
    class Video
    {
    public:
        static maix::image::Image *NoneImage;

        /**
         * @brief Construct a new Video object
         * @param path video path. the path determines the location where you load or save the file, if path is none, the video module will not save or load file.
         * xxx.h265 means video format is H265, xxx.mp4 means video format is MP4
         * @param width picture width. this value may be set automatically. default is 2560.
         * @param height picture height. this value may be set automatically. default is 1440.
         * @param format picture pixel format. this value may be set automatically. default is FMT_YVU420SP.
         * @param time_base frame time base. time_base default is 30, means 1/30 ms
         * @param framerate frame rate. framerate default is 30, means 30 frames per second
         * for video. 1/time_base is not the average frame rate if the frame rate is not constant.
         * @param capture enable capture, if true, you can use capture() function to get an image object
         * @param open If true, video will automatically call open() after creation. default is true.
         * @maixpy maix.video.Video.__init__
         * @maixcdk maix.video.Video.Video
         */
        Video(std::string path = std::string(), int width = 2560, int height = 1440, image::Format format = image::Format::FMT_YVU420SP, int time_base = 30, int framerate = 30, bool capture = false, bool open = true);
        ~Video();

        /**
         * Open video and run
         * @param path video path. the path determines the location where you load or save the file, if path is none, the video module will not save or load file.
         * xxx.h265 means video format is H265, xxx.mp4 means video format is MP4
         * @param fps video fps
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.open
         */
        err::Err open(std::string path = std::string(), double fps = 30.0);

        /**
         * Close video
         * @maixpy maix.video.Video.close
        */
        void close();

        /**
         * @brief Bind camera
         * @param camera camera object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Video.bind_camera
        */
        err::Err bind_camera(camera::Camera *camera);

        /**
         * Encode image.
         * @param img the image will be encode.
         * if the img is NULL, this function will try to get image from camera, you must use bind_camera() function to bind the camera.
         * @return encode result
         * @maixpy maix.video.Video.encode
        */
        video::Packet *encode(image::Image *img = maix::video::Video::NoneImage);

        /**
         * Decode frame
         * @param frame the frame will be decode
         * @return decode result
         * @maixpy maix.video.Video.decode
        */
        image::Image *decode(video::Frame *frame = nullptr);

        /**
         * Encode or decode finish
         * @return error code
         * @maixpy maix.video.Video.finish
        */
        err::Err finish();

        /**
         * Capture image
         * @attention Each time encode is called, the last captured image will be released.
         * @return error code
         * @maixpy maix.video.Video.capture
        */
        image::Image *capture() {
            err::check_null_raise(_capture_image, "Can't capture image, please make sure the capture flag is set, and run this api after encode().");
            image::Image *new_image = new image::Image(_capture_image->width(), _capture_image->height(), _capture_image->format(),
                (uint8_t *)_capture_image->data(), _capture_image->data_size(), false);
            return new_image;
        }

        /**
         * Check if video is recording
         * @return true if video is recording, false if not
         * @maixpy maix.video.Video.is_recording
        */
        bool is_recording() {
            return this->_is_recording;
        }

        /**
         * Check if video is opened
         * @return true if video is opened, false if not
         * @maixpy maix.video.Video.is_opened
        */
        bool is_opened() {
            return this->_is_opened;
        }

        /**
         * @brief check video device is closed or not
         * @return closed or not, bool type
         * @maixpy maix.video.Video.is_closed
        */
        bool is_closed() { return !is_opened();}

        /**
         * Get video width
         * @return video width
         * @maixpy maix.video.Video.width
        */
        int width()
        {
            return _width;
        }

        /**
         * Get video height
         * @return video height
         * @maixpy maix.video.Video.height
        */
        int height()
        {
            return _height;
        }
    private:
        std::string _pre_path;
        double _pre_fps;
        bool _pre_record;
        int _pre_interval;
        int _pre_width;
        int _pre_height;
        bool _pre_audio;
        int _pre_sample_rate;
        int _pre_channel;

        std::string _path;
        std::string _tmp_path;
        double _fps;
        bool _record;
        int _interval;
        int _width;
        int _height;
        bool _audio;
        int _sample_rate;
        int _channel;

        bool _bind_camera;
        bool _is_recording;
        camera::Camera *_camera;
        thread::Thread *_thread;
        video::VideoType _video_type;
        bool _is_opened;
        uint64_t _record_ms;
        uint64_t _record_start_ms;
        int _fd;
        bool _need_capture;
        image::Image *_capture_image;

        bool _need_auto_config;
        int _time_base;
        int _framerate;
        fs::File file;
        uint64_t _last_pts;
    };
}

