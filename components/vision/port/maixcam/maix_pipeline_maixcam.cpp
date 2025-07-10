#include "maix_pipeline.hpp"
#include "sophgo_middleware.hpp"
#include <string>
using namespace maix;

namespace maix::pipeline {
    typedef struct {
        bool is_venc;
        int channel;
        mmf_stream_t stream;
    } stream_param_t;

    static int __parse_from(std::string from, stream_param_t &param) {
        std::size_t pos = std::string::npos;
        if (std::string::npos != (pos = from.find("venc"))) {             // from="venc,0"
            param.is_venc = true;

            if (std::string::npos == (pos = from.find(","))) {
                return -1;
            }
            param.channel = std::stoi(from.substr(pos + 1, 1));
        }

        if (param.channel < 0) {
            return -1;
        }

        return 0;
    }

    Stream::Stream(void *stream, bool auto_delete, std::string from) {
        stream_param_t *param = (stream_param_t *)malloc(sizeof(stream_param_t));
        err::check_null_raise(param, "Malloc failed");
        memset(param, 0, sizeof(stream_param_t));

        __stream = param;
        __auto_delete = auto_delete;
        __from = from;

        if (0 != __parse_from(from, *param)) {
            err::check_raise(err::ERR_RUNTIME, "Invalid stream from string: " + from);
        }

        if (param->is_venc) {
            memcpy(&param->stream, stream, sizeof(mmf_stream_t));
        } else {
            err::check_raise(err::ERR_RUNTIME, "Construct stream is not supported");
        }

        // for (int i = 0; i < param->stream.count; i++) {
        //     log::info("[%d] data: %p, size: %d", i, param->stream.data[i], param->stream.data_size[i]);
        // }
    }

    Stream::~Stream() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (__stream) {
            if (__auto_delete) {
                if (param->is_venc) {
                    mmf_venc_free(param->channel);
                }
            }

            free(__stream);
            __stream = nullptr;
        }
    }

    int Stream::data_count() {
        stream_param_t *param = (stream_param_t *)__stream;
        return param->stream.count;
    }

    Bytes *Stream::data(int idx) {
        stream_param_t *param = (stream_param_t *)__stream;
        err::check_bool_raise(param->stream.count > idx, "idx out of range");
        return new Bytes(param->stream.data[idx],
                       param->stream.data_size[idx], false, false);
    }

    int Stream::data_size(int idx) {
        stream_param_t *param = (stream_param_t *)__stream;
        err::check_bool_raise(param->stream.count > idx, "idx out of range");
        return param->stream.data_size[idx];
    }

    Bytes *Stream::get_sps_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count > 1) {
            return new Bytes(param->stream.data[0],
                        param->stream.data_size[0], false, false);
        }

        return nullptr;
    }

    Bytes *Stream::get_pps_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count > 1) {
            return new Bytes(param->stream.data[1],
                        param->stream.data_size[1], false, false);
        }

        return nullptr;
    }

    Bytes *Stream::get_i_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count > 1) {
            return new Bytes(param->stream.data[2],
                        param->stream.data_size[2], false, false);
        }

        return nullptr;
    }

    Bytes *Stream::get_p_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count == 1) {
            return new Bytes(param->stream.data[0],
                        param->stream.data_size[0], false, false);
        }

        return nullptr;
    }

    bool Stream::has_pps_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count > 1) {
            return true;
        }

        return false;
    }

    bool Stream::has_sps_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count > 1) {
            return true;
        }

        return false;
    }

    bool Stream::has_i_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count > 1) {
            return true;
        }

        return false;
    }

    bool Stream::has_p_frame() {
        stream_param_t *param = (stream_param_t *)__stream;
        if (param->stream.count == 1) {
            return true;
        }

        return false;
    }

    int Stream::pts() {
        return 0;
    }

    void *Stream::stream() {
        stream_param_t *param = (stream_param_t *)__stream;
        return &param->stream;
    }

    typedef struct {
        bool is_vpss_chn;
        bool is_vdec;
        int group;
        int channel;
        union {
            VIDEO_FRAME_INFO_S frame;
        };
    } frame_param_t;

    static int __parse_from(std::string from, frame_param_t &param) {
        std::size_t pos = std::string::npos;
        if (std::string::npos != (pos = from.find("vpsschn"))) {             // from="vpsschn,0,0"
            param.is_vpss_chn = true;

            if (std::string::npos == (pos = from.find(","))) {
                return -1;
            }
            auto group_str = from.substr(pos + 1, 1);
            param.group = std::stoi(group_str);

            if (std::string::npos == (pos = from.find(","))) {
                return -1;
            }
            param.channel = std::stoi(from.substr(pos + 1, 1));
        } else if (std::string::npos != (pos = from.find("vdec"))) {        // from="vdec,0"
            param.is_vpss_chn = true;

            if (std::string::npos == (pos = from.find(","))) {
                return -1;
            }
            param.channel = std::stoi(from.substr(pos + 1));
        }

        if (param.channel < 0) {
            return -1;
        }

        return 0;
    }

    Frame::Frame(void *frame, bool auto_delete, std::string from) {
        frame_param_t *param = (frame_param_t *)malloc(sizeof(frame_param_t));
        err::check_null_raise(param, "Malloc failed");
        memset(param, 0, sizeof(frame_param_t));

        __frame = param;
        __auto_delete = auto_delete;
        __from = __from;

        if (0 != __parse_from(from, *param)) {
            err::check_raise(err::ERR_RUNTIME, "Invalid stream from string: " + from);
        }

        if (param->is_vdec) {
            memcpy(&param->frame, frame, sizeof(VIDEO_FRAME_INFO_S));
        } else if (param->is_vpss_chn) {
            memcpy(&param->frame, frame, sizeof(VIDEO_FRAME_INFO_S));
        } else {
            err::check_raise(err::ERR_RUNTIME, "Convert to venc stream failed");
        }
    }

    Frame::~Frame() {
        frame_param_t *param = (frame_param_t *)__frame;
        if (__frame) {
            if (__auto_delete) {
                if (param->is_vdec) {
                    CVI_VDEC_ReleaseFrame(param->channel, &param->frame);
                } else if (param->is_vpss_chn) {
                    CVI_VPSS_ReleaseChnFrame(param->group, param->channel, &param->frame);
                }
            }

            free(__frame);
            __frame = nullptr;
        }
    }

    int Frame::width() {
        frame_param_t *param = (frame_param_t *)__frame;
        return param->frame.stVFrame.u32Width;
    }

    int Frame::height() {
        frame_param_t *param = (frame_param_t *)__frame;
        return param->frame.stVFrame.u32Height;
    }

    image::Format Frame::format() {
        frame_param_t *param = (frame_param_t *)__frame;
        return (image::Format)mmf_invert_format_to_maix(param->frame.stVFrame.enPixelFormat);
    }

    image::Image *Frame::to_image() {
        err::check_raise(err::ERR_NOT_IMPL, "This function is not implemented yet.");
        return nullptr;
    }

    int Frame::stride(int idx) {
        frame_param_t *param = (frame_param_t *)__frame;
        err::check_bool_raise(idx <= 2 && idx >= 0, "invalid plane index");
        return param->frame.stVFrame.u32Stride[idx];
    }

    uint64_t Frame::virtual_address(int idx) {
        frame_param_t *param = (frame_param_t *)__frame;
        err::check_bool_raise(idx <= 2 && idx >= 0, "invalid plane index");
        return (uint64_t)param->frame.stVFrame.pu8VirAddr[idx];
    }

    uint64_t Frame::physical_address(int idx) {
        frame_param_t *param = (frame_param_t *)__frame;
        err::check_bool_raise(idx <= 2 && idx >= 0, "invalid plane index");
        return (uint64_t)param->frame.stVFrame.u64PhyAddr[idx];
    }

    void *Frame::frame() {
        frame_param_t *param = (frame_param_t *)__frame;
        return &param->frame;
    }
}

