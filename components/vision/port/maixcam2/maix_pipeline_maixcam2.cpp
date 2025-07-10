#include "maix_pipeline.hpp"
#include "ax_middleware.hpp"

using namespace maix::middleware;

namespace maix::pipeline {
    Stream::Stream(void *stream, bool auto_delete, std::string from) {
        err::check_null_raise(stream, "pipeline frame is null");
        __stream = stream;
        __auto_delete = auto_delete;
        __from = from;

        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");

        // switch (video_stream.stPack.enType) {
        // case PT_H264:
        //     log::info("video stream type: h264");
        //     for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
        //         log::info("frame type:%d", video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType);
        //     }
        // break;
        // default:
        // break;
        // }
    }

    Stream::~Stream() {
        if (__auto_delete && __stream) {
            delete (maixcam2::Frame *)__stream;
            __stream = nullptr;
        }
    }

    int Stream::data_count() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        return video_stream.stPack.u32NaluNum;
    }

    Bytes *Stream::data(int idx) {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[idx].u32NaluOffset,
                        video_stream.stPack.stNaluInfo[idx].u32NaluLength, false, false);
    }

    int Stream::data_size(int idx) {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        err::check_bool_raise(video_stream.stPack.u32NaluNum > (AX_U32)idx, "idx out of range");
        return video_stream.stPack.stNaluInfo[idx].u32NaluLength;
    }

    Bytes *Stream::get_sps_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_SPS) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_SPS) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            default:
            break;
            }
        }
        return nullptr;
    }

    Bytes *Stream::get_pps_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_PPS) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_PPS) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            default:
            break;
            }
        }
        return nullptr;
    }

    Bytes *Stream::get_i_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_IDRSLICE) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_ISLICE) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            default:
            break;
            }
        }
        return nullptr;
    }

    Bytes *Stream::get_p_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_PSLICE) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_PSLICE) {
                    return new Bytes(video_stream.stPack.pu8Addr + video_stream.stPack.stNaluInfo[i].u32NaluOffset,
                                    video_stream.stPack.stNaluInfo[i].u32NaluLength, false, false);
                }
            break;
            default:
            break;
            }
        }
        return nullptr;
    }

    bool Stream::has_pps_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_PPS) {
                    return true;
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_PPS) {
                    return true;
                }
            break;
            default:
            break;
            }
        }

        return false;
    }

    bool Stream::has_sps_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_SPS) {
                    return true;
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_SPS) {
                    return true;
                }
            break;
            default:
            break;
            }
        }
        return false;
    }

    bool Stream::has_i_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_IDRSLICE) {
                    return true;
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_IDRSLICE) {
                    return true;
                }
            break;
            default:
            break;
            }
        }
        return false;
    }

    bool Stream::has_p_frame() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        for (AX_U32 i = 0; i < video_stream.stPack.u32NaluNum; i++) {
            switch (video_stream.stPack.enType) {
            case PT_H264:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH264EType == AX_H264E_NALU_PSLICE) {
                    return true;
                }
            break;
            case PT_H265:
                if (video_stream.stPack.stNaluInfo[i].unNaluType.enH265EType == AX_H265E_NALU_PSLICE) {
                    return true;
                }
            break;
            default:
            break;
            }
        }
        return false;
    }

    int Stream::pts() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__stream;
        AX_VENC_STREAM_T video_stream;
        err::check_raise(frame->get_venc_stream(&video_stream), "get video stream failed");
        return video_stream.stPack.u64PTS;
    }

    void *Stream::stream() {
        return __stream;
    }

    Frame::Frame(void *frame, bool auto_delete, std::string from) {
        err::check_null_raise(frame, "pipeline frame is null");
        __frame = frame;
        __auto_delete = auto_delete;
    }

    Frame::~Frame() {
        if (__auto_delete && __frame) {
            delete (maixcam2::Frame *)__frame;
            __frame = nullptr;
        }
    }

    int Frame::width() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        AX_VIDEO_FRAME_T video_frame;
        err::check_raise(frame->get_video_frame(&video_frame), "get video frame failed");
        return video_frame.u32Width;
    }

    int Frame::height() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        AX_VIDEO_FRAME_T video_frame;
        err::check_raise(frame->get_video_frame(&video_frame), "get video frame failed");
        return video_frame.u32Height;
    }

    image::Format Frame::format() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        AX_VIDEO_FRAME_T video_frame;
        err::check_raise(frame->get_video_frame(&video_frame), "get video frame failed");
        return maixcam2::get_maix_fmt_from_ax(video_frame.enImgFormat);
    }

    image::Image *Frame::to_image() {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        auto format = maixcam2::get_maix_fmt_from_ax(frame->fmt);
        auto img = new image::Image(frame->w, frame->h, format);
        auto data = (uint8_t *)img->data();
        memcpy(data, (uint8_t *)frame->data, frame->len);

        switch (format) {
        case image::Format::FMT_RGBA8888:   //fall through
        case image::Format::FMT_BGRA8888:
        {
            uint32_t *img_buff_u32 = (uint32_t *)data;
            for (int h = 0; h < img->height(); h ++) {
                for (int w = 0; w < img->width(); w ++) {
                    img_buff_u32[h * img->width() + w] |= 0xFF000000;
                }
            }
        }
            break;
        default: break;
        }

        return img;
    }

    int Frame::stride(int idx) {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        err::check_bool_raise(idx <= 2 && idx >= 0, "invalid plane index");
        AX_VIDEO_FRAME_T video_frame;
        err::check_raise(frame->get_video_frame(&video_frame), "get video frame failed");
        return video_frame.u32PicStride[idx];
    }

    uint64_t Frame::virtual_address(int idx) {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        err::check_bool_raise(idx <= 2 && idx >= 0, "invalid plane index");
        AX_VIDEO_FRAME_T video_frame;
        err::check_raise(frame->get_video_frame(&video_frame), "get video frame failed");
        return video_frame.u64VirAddr[idx];
    }

    uint64_t Frame::physical_address(int idx) {
        maixcam2::Frame *frame = (maixcam2::Frame *)__frame;
        err::check_bool_raise(idx <= 2 && idx >= 0, "invalid plane index");
        AX_VIDEO_FRAME_T video_frame;
        err::check_raise(frame->get_video_frame(&video_frame), "get video frame failed");
        return video_frame.u64PhyAddr[idx];
    }

    void *Frame::frame(void) {
        return __frame;
    }
}

