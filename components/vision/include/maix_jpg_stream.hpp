/**
 * @author lxowalle@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#ifndef __MAIX_JPG_STREAM_HPP
#define __MAIX_JPG_STREAM_HPP

#include "maix_err.hpp"
#include "maix_camera.hpp"
#include "maix_display.hpp"
#include "maix_video.hpp"
#include "maix_image.hpp"
#include "maix_basic.hpp"

namespace maix::http
{
    /**
     * JpegStreamer class
     * @maixpy maix.http.JpegStreamer
     */
    class JpegStreamer
    {
    public:
        /**
         * @brief Construct a new jpeg streamer object
         * @note You can get the picture stream through http://host:port/stream, you can also get it through http://ip:port, and you can add personal style through set_html() at this time
         * @param host http host
         * @param port http port, default is 8000
         * @param client_number the max number of client
         * @maixpy maix.http.JpegStreamer.__init__
         * @maixcdk maix.http.JpegStreamer.JpegStreamer
         */
        JpegStreamer(std::string host = std::string(), int port = 8000, int client_number = 16);
        ~JpegStreamer();

        /**
         * @brief start jpeg streame
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.http.JpegStreamer.start
        */
        err::Err start();

        /**
         * @brief stop http
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.http.JpegStreamer.stop
        */
        err::Err stop();

        /**
         * @brief Write data to http
         * @param img image object
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.http.JpegStreamer.write
        */
        err::Err write(image::Image *img);

        /**
         * @brief add your style in this api
         * default is:
         * <html>
         *      <body>
         *          <h1>JPG Stream</h1>
         *          <img src='/stream'>
         *      </body>
         * </html>
         * @param data html code
         * @return error code, err::ERR_NONE means success, others means failed
         * @maixpy maix.http.JpegStreamer.set_html
        */
        err::Err set_html(std::string data);

        /**
         * Get host
         * @return host name
         * @maixpy maix.http.JpegStreamer.host
        */
        std::string host() {
            return _host;
        }

        /**
         * Get port
         * @return port
         * @maixpy maix.http.JpegStreamer.port
        */
        int port() {
            return _port;
        }
    private:
        std::string _host;
        int _port;
    };
} // namespace maix::http

#endif // __MAIX_JPG_STREAM_HPP