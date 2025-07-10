#include "maix_pipeline.hpp"
#include "ax_middleware.hpp"

namespace maix::pipeline {
    Stream::Stream(void *stream, bool auto_delete, std::string from) {
        err::check_raise(err::ERR_NOT_IMPL, "Construct stream failed");
    }

    Stream::~Stream() {
    }

    int Stream::data_count() {
        return 0;
    }

    Bytes *Stream::data(int idx) {
        return nullptr;
    }

    int Stream::data_size(int idx) {
        return 0;
    }

    Bytes *Stream::get_sps_frame() {
        return nullptr;
    }

    Bytes *Stream::get_pps_frame() {
        return nullptr;
    }

    Bytes *Stream::get_i_frame() {
        return nullptr;
    }

    Bytes *Stream::get_p_frame() {
        return nullptr;
    }

    bool Stream::has_pps_frame() {
        return false;
    }

    bool Stream::has_sps_frame() {
        return false;
    }

    bool Stream::has_i_frame() {
        return false;
    }

    bool Stream::has_p_frame() {
        return false;
    }

    int Stream::pts() {
        return 0;
    }

    void *Stream::stream() {
        return nullptr;
    }

    Frame::Frame(void *frame, bool auto_delete, std::string from) {
    }

    Frame::~Frame() {
    }

    int Frame::width() {
        return 0;
    }

    int Frame::height() {
        return 0;
    }

    image::Format Frame::format() {
        return image::FMT_INVALID;
    }

    image::Image *Frame::to_image() {
        return nullptr;
    }

    int Frame::stride(int idx) {
        return 0;
    }

    uint64_t Frame::virtual_address(int idx) {
        return 0;
    }

    uint64_t Frame::physical_address(int idx) {
        return 0;
    }

    void *Frame::frame() {
        return __frame;
    }
}

