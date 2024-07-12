#include "maix_rtmp.hpp"

#include "maix_video.hpp"
#include "sophgo_middleware.hpp"
#include <stdio.h>
#include "sockutil.h"
#include "sys/system.h"
#include "rtmp-client.h"
#include "flv-reader.h"
#include "flv-writer.h"
#include "flv-header.h"
#include "flv-proto.h"
#include "maix_avc2flv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MMF_VENC_CHN	1
#define AAC_ADTS_HEADER_SIZE 7
#define FLV_TAG_HEAD_LEN 11
#define FLV_PRE_TAG_LEN 4

static int rtmp_client_send(void* param, const void* header, size_t len, const void* data, size_t bytes)
{
	socket_t* socket = (socket_t*)param;
	socket_bufvec_t vec[2];
	socket_setbufvec(vec, 0, (void*)header, len);
	socket_setbufvec(vec, 1, (void*)data, bytes);

	return socket_send_v_all_by_time(*socket, vec, bytes > 0 ? 2 : 1, 0, 5000);
}

static uint8_t* h264_startcode(uint8_t* data, size_t bytes)
{
	size_t i;
	for (i = 2; i + 1 < bytes; i++)
	{
		if (0x01 == data[i] && 0x00 == data[i - 1] && 0x00 == data[i - 2])
			return data + i + 1;
	}

	return NULL;
}

int maix_rtmp_client_push_h264(rtmp_client_t *client, struct flv_vec_t *vec, int num, uint32_t timestamp)
{
	int tmp_offset = 0;
	int tmp_max_size = 256 * 1024;
	uint8_t *tmp = (uint8_t *)malloc(tmp_max_size);
	if (!tmp) return -1;

	for (int i = 0; i < num; i ++) {
		uint8_t *nalu = h264_startcode((uint8_t *)vec[i].ptr, vec[i].len);
		uint8_t *raw = (uint8_t *)vec[i].ptr;
		int raw_size = (uint8_t *)vec[i].ptr + vec[i].len - raw;
		uint8_t nalutype = nalu[0] & 0x1f;
		if (nalutype == 0x7 || nalutype == 0x8 || nalutype == 6) {
			if (tmp_offset + raw_size > tmp_max_size) {
				free(tmp);
				tmp = NULL;
				return -1;
			}
			memcpy(tmp + tmp_offset, raw, raw_size);
			tmp_offset += raw_size;

		}  else if (nalutype == 0x5 || nalutype == 0x1) {
			if (tmp_offset + raw_size > tmp_max_size) {
				free(tmp);
				tmp = NULL;
				return -1;
			}
			memcpy(tmp + tmp_offset, raw, raw_size);
			tmp_offset += raw_size;

			uint8_t *flv;
			int flv_size;
			if (0 != maix_avc2flv(tmp, tmp_offset, timestamp, timestamp, &flv, &flv_size)) {
				printf("maix_avc2flv failed!\r\n");
				free(tmp);
				tmp = NULL;
				return -1;
			}
			// printf("tmp_offset:%d flv:%p flv_size:%d timestamp:%d\r\n", tmp_offset, flv, flv_size, timestamp);
			free(tmp);
			tmp = NULL;
			uint8_t *flv_next = flv;
			while (1) {
				if (flv_next >= (flv + flv_size)) break;
				struct flv_tag_header_t tag_header = {0};
				flv_tag_header_read(&tag_header, flv_next, FLV_TAG_HEAD_LEN);

				// printf("flv:%p flv_next:%p tag len:%d timestamp:%d\r\n", flv, flv_next, tag_header.size, timestamp);

				rtmp_client_push_video(client, flv + FLV_TAG_HEAD_LEN, tag_header.size, timestamp);
				flv_next += FLV_PRE_TAG_LEN + FLV_TAG_HEAD_LEN + tag_header.size;
			}

			tmp_offset = 0;
		}
	}

	if (tmp) {
		free(tmp);
		tmp = NULL;
	}

	return 0;
}

namespace maix::rtmp {
	Rtmp::Rtmp(std::string host, int port, std::string app, std::string stream, int bitrate) {
		_host = host;
		_port = port;
		_app = app;
		_stream = stream;
		_bitrate = bitrate;

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
				throw std::runtime_error("rtmp client init failed!");
			}
		}

		if (0 != pthread_mutex_init(&_lock, NULL)) {
			throw std::runtime_error("create lock failed!");
		}

		if (0 != mmf_init()) {
			err::check_raise(err::ERR_RUNTIME, "init mmf failed!");
		}

		_handler = rtmp;
		_thread = nullptr;
		_push_thread = nullptr;
		_app_thread = nullptr;
		_camera = nullptr;
		_start = false;
		_capture_image = nullptr;
		_need_capture = false;
		_path = std::string();
	}

	Rtmp::~Rtmp() {
		if (_start) {
			stop();
		}

		rtmp_client_t* rtmp = (rtmp_client_t *)_handler;
		rtmp_client_destroy(rtmp);
		socket_close(_socket);
		socket_cleanup();

		mmf_deinit();

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

			clock = time::ticks_ms();
			while (1 == flv_reader_read(f, &type, &timestamp, &taglen, packet, packet_size))
			{
				// log::debug("[%ld]type:%d  timestemp:%d taglen:%ld\r\n", time::ticks_ms(), type, timestamp, taglen);

				rtmp->lock(-1);
				if (!rtmp->is_started()) {
					rtmp->unlock();
					break;
				}

				uint64_t t = time::ticks_ms();
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

	static void _push_camera_thread(void *args) {
		Rtmp *rtmp = (Rtmp *)args;
		rtmp->lock(-1);
		camera::Camera *camera = rtmp->get_camera();
		rtmp_client_t *client = (rtmp_client_t *)rtmp->get_handler();
		rtmp->unlock();

		uint32_t stream_size = 0;
		uint32_t timestamp = 0;
		uint64_t curr_ms = time::ticks_ms();
		uint64_t last_ms = curr_ms;

		bool first_frame = 1;

		mmf_venc_cfg_t cfg = {
			.type = 2,  //1, h265, 2, h264
			.w = camera->width(),
			.h = camera->height(),
			.fmt = mmf_invert_format_to_mmf(image::Format::FMT_YVU420SP),
			.jpg_quality = 0,       // unused
			.gop = 50,
			.intput_fps = 30,
			.output_fps = 30,
			.bitrate = rtmp->bitrate() / 1000,
		};


		if (0 != mmf_add_venc_channel(MMF_VENC_CHN, &cfg)) {
			err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!");
		}

		if (0 != maix_avc2flv_init(rtmp->bitrate() / 2)) {
			err::check_raise(err::ERR_RUNTIME, "maix avc2flv init failed!");
		}
		while (1) {
			rtmp->lock(-1);
			if (!rtmp->is_started()) {
				rtmp->unlock();
				break;
			}

			int vi_ch = camera->get_channel();
			void *data;
			int data_size, width, height, format;
			do {
				mmf_stream_t stream = {0};
				if (mmf_venc_pop(MMF_VENC_CHN, &stream)) {
					log::error("mmf_venc_pop failed\n");
					mmf_venc_free(MMF_VENC_CHN);
					mmf_del_venc_channel(MMF_VENC_CHN);
					rtmp->unlock();
					break;
				}

				flv_vec_t vec[8];
				for (int i = 0; i < stream.count; i ++) {
					vec[i].ptr = stream.data[i];
					vec[i].len = stream.data_size[i];
					stream_size += stream.data_size[i];
				}

				if (first_frame) {
					last_ms = time::ticks_ms();
					timestamp = 0;
					first_frame = false;
				}

				if (0 != maix_rtmp_client_push_h264(client, vec, stream.count, timestamp)) {
					printf("rtmp push failed!\r\n");
				}

				if (mmf_venc_free(MMF_VENC_CHN)) {
					printf("mmf_venc_free failed\n");
					mmf_del_venc_channel(MMF_VENC_CHN);
					rtmp->unlock();
					break;
				}

				if (mmf_vi_frame_pop(vi_ch, &data, &data_size, &width, &height, &format)) {
					log::info("camera read image timeout!\r\n");
					rtmp->unlock();
					break;
				}

				curr_ms = time::ticks_ms();
				timestamp = curr_ms - last_ms;

				if (data_size > 2560 * 1440 * 3 / 2) {
					log::error("image is too large!\r\n");
					rtmp->unlock();
					break;
				}

				if (rtmp->get_camera()) {
					image::Image *_capture_image = rtmp->get_capture_image();
					if (_capture_image && _capture_image->data()) {
						delete _capture_image;
						rtmp->set_capture_image(NULL);
					}

					image::Format capture_format = (image::Format)mmf_invert_format_to_maix(format);
					bool need_align = (width % mmf_vi_aligned_width(vi_ch) == 0) ? false : true;   // Width need align only
					switch (capture_format) {
						case image::Format::FMT_BGR888: // fall through
						case image::Format::FMT_RGB888:
						{
							_capture_image = rtmp->set_capture_image(new image::Image(width, height, capture_format));
							uint8_t * image_data = (uint8_t *)_capture_image->data();
							if (need_align) {
								for (int h = 0; h < height; h++) {
									memcpy((uint8_t *)image_data + h * width * 3, (uint8_t *)data + h * width * 3, width * 3);
								}
							} else {
								memcpy(image_data, data, width * height * 3);
							}
						}
							break;
						case image::Format::FMT_YVU420SP:
						{
							_capture_image = rtmp->set_capture_image(new image::Image(width, height, capture_format));
							uint8_t * image_data = (uint8_t *)_capture_image->data();
							if (need_align) {
								for (int h = 0; h < height * 3 / 2; h ++) {
									memcpy((uint8_t *)image_data + h * width, (uint8_t *)data + h * width, width);
								}
							} else {
								memcpy(image_data, data, width * height * 3 / 2);
							}
							break;
						}
						default:
						{
							rtmp->set_capture_image(NULL);
							break;
						}
					}
				}

				mmf_venc_cfg_t cfg = {0};
				if (0 != mmf_venc_get_cfg(MMF_VENC_CHN, &cfg)) {
					err::check_raise(err::ERR_RUNTIME, "get venc config failed!\r\n");
				}

				int img_w = width;
				int img_h = height;
				int mmf_fmt = format;
				if (img_w != cfg.w
					|| img_h != cfg.h
					|| mmf_fmt != cfg.fmt) {
					log::warn("image size or format is incorrect, try to reinit venc!\r\n");
					mmf_del_venc_channel(MMF_VENC_CHN);
					cfg.w = img_w;
					cfg.h = img_h;
					cfg.fmt = mmf_invert_format_to_mmf(mmf_fmt);
					if (0 != mmf_add_venc_channel(MMF_VENC_CHN, &cfg)) {
						err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!\r\n");
					}
				}

				if (mmf_venc_push(MMF_VENC_CHN, (uint8_t *)data, width, height, format)) {
					mmf_del_venc_channel(MMF_VENC_CHN);
					rtmp->unlock();
					err::check_raise(err::ERR_RUNTIME, "mmf venc push failed!\r\n");
					break;
				}

				mmf_vi_frame_free(vi_ch);
			} while (stream_size == 0 && rtmp->is_started());
			rtmp->unlock();
        }

		maix_avc2flv_deinit();

		if (0 != mmf_del_venc_channel(MMF_VENC_CHN)) {
			err::check_raise(err::ERR_RUNTIME, "mmf venc init failed!");
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
			unlock();
			return err::ERR_BUSY;
		}

		if (path.size() > 0) {
			if (fs::splitext(path)[1] != ".flv") {
				log::error("check file path failed!\r\n");
				unlock();
				return err::ERR_RUNTIME;
			}

			_path = path;
			_start = true;
			_thread = new thread::Thread(_push_file_thread, this);
			if (this->_thread == NULL) {
				log::error("create camera thread failed!\r\n");
				unlock();
				return err::ERR_RUNTIME;
			}
		} else {
			_start = true;
			_app_thread = new thread::Thread(_push_camera_thread, this);
			if (this->_app_thread == NULL) {
				log::error("create camera thread failed!\r\n");
				unlock();
				return err::ERR_RUNTIME;
			}

		}

		unlock();
		return err::ERR_NONE;
	}

	err::Err Rtmp::stop() {
		_start = false;

		if (_thread) {
			_thread->join();
			_thread = nullptr;
		}

		if (_push_thread) {
			_push_thread->join();
			_push_thread = nullptr;
		}

		if (_app_thread) {
			_app_thread->join();
			_app_thread = nullptr;
		}

		return err::ERR_NONE;
	}
}