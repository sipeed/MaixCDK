#include "maix_rtmp.hpp"

#include <stdio.h>
#include "sockutil.h"
#include "sys/system.h"
#include "rtmp-client.h"
#include "flv-reader.h"
#include "flv-proto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

static int rtmp_client_send(void* param, const void* header, size_t len, const void* data, size_t bytes)
{
	socket_t* socket = (socket_t*)param;
	socket_bufvec_t vec[2];
	socket_setbufvec(vec, 0, (void*)header, len);
	socket_setbufvec(vec, 1, (void*)data, bytes);

	return socket_send_v_all_by_time(*socket, vec, bytes > 0 ? 2 : 1, 0, 5000);
}

namespace maix::rtmp {
	Rtmp::Rtmp(std::string host, int port, std::string app, std::string stream) {
		_host = host;
		_port = port;
		_app = app;
		_stream = stream;

		char packet[20 * 1024];
		snprintf(packet, sizeof(packet), "rtmp://%s/%s", host.c_str(), app.c_str()); // tcurl
		struct rtmp_client_handler_t handler;
		memset(&handler, 0, sizeof(handler));
		handler.send = rtmp_client_send;

		socket_init();
		socket_t socket = socket_connect_host(_host.c_str(), _port, 2000);
		if (socket <= 0) {
			throw std::runtime_error("socket connect failed!");
		}
		socket_setnonblock(socket, 0);
		_socket = socket;

		rtmp_client_t* rtmp = rtmp_client_create(_app.c_str(), _stream.c_str(), packet/*tcurl*/, &_socket, &handler);
		int r = rtmp_client_start(rtmp, 0);
		while (4 != rtmp_client_getstate(rtmp) && (r = socket_recv(_socket, packet, sizeof(packet), 0)) > 0)
		{
			int res = rtmp_client_input(rtmp, packet, r);
			if (res != 0) {
				throw std::runtime_error("rtmp_client_input failed!");
			}
		}

		if (0 != pthread_mutex_init(&_lock, NULL)) {
			throw std::runtime_error("create lock failed!");
		}

		_handler = rtmp;
		_thread = nullptr;
		_cam = nullptr;
		_path = std::string();
	}

	Rtmp::~Rtmp() {
		rtmp_client_t* rtmp = (rtmp_client_t *)_handler;
		rtmp_client_destroy(rtmp);
		socket_close(_socket);
		socket_cleanup();

		pthread_mutex_destroy(&_lock);
	}

	// return 0 ok, other error
	int Rtmp::push_video(void *data, size_t data_size, uint32_t timestamp) {
		rtmp_client_t* rtmp = (rtmp_client_t *)_handler;
		if (rtmp == nullptr) {
			throw std::runtime_error("rtmp hander is null!");
		}
		return rtmp_client_push_video(rtmp, data, data_size, timestamp);
	}

	// return 0 ok, other error
	int Rtmp::push_audio(void *data, size_t data_size, uint32_t timestamp) {
		rtmp_client_t* rtmp = (rtmp_client_t *)_handler;
		if (rtmp == nullptr) {
			throw std::runtime_error("rtmp hander is null!");
		}
		return rtmp_client_push_audio(rtmp, data, data_size, timestamp);
	}

	// return 0 ok, other error
	int Rtmp::push_script(void *data, size_t data_size, uint32_t timestamp) {
		rtmp_client_t* rtmp = (rtmp_client_t *)_handler;
		if (rtmp == nullptr) {
			throw std::runtime_error("rtmp hander is null!");
		}
		return rtmp_client_push_script(rtmp, data, data_size, timestamp);
	}

	static void _push_file_thread(void *args) {
		Rtmp *rtmp = (Rtmp *)args;
		rtmp->lock(-1);
		std::string path = rtmp->get_path();
		rtmp->unlock();

		int r, type;
		int aacconfig = 0;
		size_t taglen;
		uint32_t timestamp;
		uint32_t s_timestamp = 0;
		uint32_t diff = 0;
		uint64_t clock = 0;
		size_t packet_size = 1024 * 1024;
		char *packet = (char *)malloc(packet_size);
		if (packet == nullptr) {
			log::error("rtmp thread malloc failed!\r\n");
			return;
		}

		while (1) {
			rtmp->lock(-1);
			if (!rtmp->is_started()) {
				rtmp->unlock();
				break;
			}
			void *f = ::flv_reader_create(&path[0]);
			rtmp->unlock();
			if (f == nullptr) {
				log::error("Find not path %s!\r\n", &path[0]);
				break;
			}

			clock = time::time_ms();
			while (1 == flv_reader_read(f, &type, &timestamp, &taglen, packet, packet_size))
			{
				// log::debug("[%ld]type:%d  timestemp:%d taglen:%ld\r\n", time::time_ms(), type, timestamp, taglen);

				rtmp->lock(-1);
				if (!rtmp->is_started()) {
					rtmp->unlock();
					break;
				}

				uint64_t t = time::time_ms();
				if (clock + timestamp > t && clock + timestamp < t + 3 * 1000) // dts skip
				{
					// log::info("skip dts, sleep %d ms\r\n", clock + timestamp - t);
					time::sleep_ms(clock + timestamp - t);
				}
				else if (clock + timestamp > t + 3 * 1000) {
					clock = t - timestamp;
				}

				timestamp += diff;
				s_timestamp = timestamp > s_timestamp ? timestamp : s_timestamp;
				if (8 == type)
				{
					if (0 == packet[1])
					{
						if(0 != aacconfig)
							continue;
						aacconfig = 1;
					}
					r = rtmp->push_audio(packet, taglen, timestamp);
				}
				else if (9 == type)
				{
					r = rtmp->push_video(packet, taglen, timestamp);
				}
				else if (18 == type)
				{
					r = rtmp->push_script(packet, taglen, timestamp);
				}
				else
				{
					r = 0; // ignore
				}

				if (0 != r)
				{
					log::error("send failed! r = %d\r\n", r);
					rtmp->unlock();
					break;
				}
				rtmp->unlock();
			}
			flv_reader_destroy(f);
			diff = s_timestamp + 30;
        }

		if (packet) {
			free(packet);
			packet = nullptr;
		}
	}

	err::Err Rtmp::lock(uint32_t time) {
		uint32_t count = 0;
		while (0 != pthread_mutex_trylock(&_lock)) {
			count ++;
			if (count >= time) {
				break;
			}
			time::sleep_ms(1);
		}

		if (count >= time)
			return err::ERR_BUSY;
		else
			return err::ERR_NONE;
	}

	err::Err Rtmp::unlock() {
		if (0 == pthread_mutex_unlock(&_lock)) {
			return err::ERR_NONE;
		}
		return err::ERR_RUNTIME;
	}

	err::Err Rtmp::start(std::string path) {
		lock(-1);
		if (_start == true) {
			return err::ERR_BUSY;
		}

		if (path.size() > 0) {
			if (fs::splitext(path) != ".flv") {
				log::error("check file path failed!\r\n");
				return err::ERR_RUNTIME;
			}

			_path = path;
			_start = true;
			_thread = new thread::Thread(_push_file_thread, this);
			if (this->_thread == NULL) {
				log::error("create camera thread failed!\r\n");
				return err::ERR_RUNTIME;
			}
		} else {
			if (_cam == nullptr) {
				log::error("you need bind camera first!\r\n");
				return err::ERR_RUNTIME;
			}
		}

		unlock();
		return err::ERR_NONE;
	}

	err::Err Rtmp::stop() {
		lock(-1);
		_start = false;
		unlock();

		_thread->join();
		_thread = nullptr;
		return err::ERR_NONE;
	}
}