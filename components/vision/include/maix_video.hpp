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
        VIDEO_ENC_H265_CBR,
        VIDEO_ENC_MP4_CBR,
        VIDEO_DEC_H265_CBR,
        VIDEO_DEC_MP4_CBR,
        VIDEO_H264_CBR,
        VIDEO_H265_CBR,
        VIDEO_H264_CBR_MP4,
        VIDEO_H265_CBR_MP4,
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
        Bytes *to_bytes(bool copy) {
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
         * @maixpy maix.video.Encoder.__init__
         * @maixcdk maix.video.Encoder.Encoder
         */
        Encoder(int width = 2560, int height = 1440, image::Format format = image::Format::FMT_YVU420SP, video::VideoType type = video::VideoType::VIDEO_H265_CBR, int framerate = 30, int gop = 50, int bitrate = 3000 * 1000, int time_base = 1000, bool capture = false);
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
         * @maixpy maix.video.Encoder.get_pts
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
         * @maixpy maix.video.Encoder.get_dts
        */
        uint64_t get_dts(uint64_t time_ms)
        {
            return time_ms * 1000 / _time_base;
        }
    private:
        int _width;
        int _height;
        image::Format _format;
        video::VideoType _type;
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
    };


    /**
     * Decoder class
     * @maixpy maix.video.Decoder
     */
    class Decoder
    {
    public:
        /**
         * @brief Construct a new Decoder object
         * @maixpy maix.video.Decoder.__init__
         * @maixcdk maix.video.Decoder.Decoder
         */
        Decoder();
        ~Decoder();

        /**
         * Prepare data to decode
         * @param data need decode data
         * @param copy if false, need to ensure that data is not released in decoding.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.video.Decoder.prepare
        */
        err::Err prepare(Bytes *data, bool copy = true);

        /**
         * Prepare data to decode
         * @param data need decode data
         * @param data_size size of data to be decoded
         * @param copy if false, need to ensure that data is not released in decoding.
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixcdk maix.video.Decoder.prepare
        */
        err::Err prepare(void *data, int data_size, bool copy = true);

        /**
         * Decode
         * @param frame the frame will be decode (not used)
         * @return decode result
         * @maixpy maix.video.Decoder.decode
        */
        image::Image *decode(video::Frame *frame = nullptr);
    private:
        int _path;
        Bytes *_prepare_data;
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

