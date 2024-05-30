#include "maix_rtmp.hpp"

namespace maix::rtmp {
	Rtmp::Rtmp(std::string host, int port, std::string app, std::string stream, int bitrate) {
		_host = host;
		_port = port;
		_app = app;
		_stream = stream;
		_bitrate = bitrate;
	}

	Rtmp::~Rtmp() {
	}

	// return 0 ok, other error
	int Rtmp::push_video(void *data, size_t data_size, uint32_t timestamp) {
		(void)data;
		(void)data_size;
		(void)timestamp;
		return 0;
	}

	// return 0 ok, other error
	int Rtmp::push_audio(void *data, size_t data_size, uint32_t timestamp) {
		(void)data;
		(void)data_size;
		(void)timestamp;
		return 0;
	}

	// return 0 ok, other error
	int Rtmp::push_script(void *data, size_t data_size, uint32_t timestamp) {
		(void)data;
		(void)data_size;
		(void)timestamp;
		return 0;
	}

	err::Err Rtmp::lock(uint32_t time) {
		(void)time;
		return err::ERR_NOT_IMPL;
	}

	err::Err Rtmp::unlock() {
		return err::ERR_NOT_IMPL;
	}

	err::Err Rtmp::start(std::string path) {
		(void)path;
		return err::ERR_NOT_IMPL;
	}

	err::Err Rtmp::stop() {
		return err::ERR_NOT_IMPL;
	}
}