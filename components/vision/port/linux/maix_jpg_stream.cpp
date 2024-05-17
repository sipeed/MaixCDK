/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2024.5.17: Add framework, create this file.
 */
#include "maix_jpg_stream.hpp"
namespace maix::http
{
        JpegStreamer::JpegStreamer(std::string host, int port, int client_number) {
			(void)host;
			(void)port;
			(void)client_number;
		}

        JpegStreamer::~JpegStreamer() {

		}

        err::Err JpegStreamer::start() {

		}

        err::Err JpegStreamer::stop() {

		}

        err::Err JpegStreamer::write(image::Image *img) {

		}

        err::Err JpegStreamer::set_html(std::string data) {

		}
}