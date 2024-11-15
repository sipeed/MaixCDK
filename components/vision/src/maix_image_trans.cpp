/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */


#include <stdlib.h>
#include "maix_image_trans.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <iostream>
#include <queue>

typedef websocketpp::client<websocketpp::config::asio_client> client;
// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

#define WS_SERVER_URI "ws://localhost:7899"
#define MSG_ID_CONN     1
#define MSG_ID_CONN_ACK 2
#define MSG_ID_IMG      6
#define MSG_ID_SET_FMT  14
#define MSG_ID_SET_FMT_ACK 15
#define P_VERSION       0
#define IMG_ENCODE_NONE 0
#define IMG_ENCODE_JPEG 1
#define IMG_ENCODE_PNG  2

enum ImageTransFmt
{
    IMG_TRANS_FMT_NONE = 0, // pause trans
    IMG_TRANS_FMT_JPEG = 1,
    IMG_TRANS_FMT_PNG = 2,
    IMG_TRANS_FMT_MAX
};


namespace maix
{
    struct QueueItem
    {
        image::Image *p_img;
    };

    template <typename T, int MaxLen, typename Container = std::deque<T>>
    class FixedQueue : public std::queue<T, Container>
    {
    public:
        void push(const T &value)
        {
            if (this->size() == MaxLen)
            {
                // just ignore
                return;
            }
            std::queue<T, Container>::push(value);
        }
    };

    struct ClientHandle
    {
        client *c;
        websocketpp::connection_hdl hdl;
        bool init;
        bool conn_fail;
        bool th_exit;
        bool conn_connected;
        FixedQueue<QueueItem, 4> img_queue;
        std::vector<uint8_t> frame_buffer;
        ImageTrans *img_trans;
    };

    inline uint8_t get_img_encode_id(image::Format fmt)
    {
        switch (fmt)
        {
        case image::FMT_INVALID:
            return IMG_ENCODE_NONE;
        case image::FMT_JPEG:
            return IMG_ENCODE_JPEG;
        case image::FMT_PNG:
            return IMG_ENCODE_PNG;
        default:
            throw err::Exception(err::ERR_NOT_IMPL, "not support image format");
        }
    }

    static image::Format get_format_from_id(uint8_t id)
    {
        switch (id)
        {
        case IMG_ENCODE_NONE:
            return image::FMT_INVALID;
        case IMG_ENCODE_JPEG:
            return image::FMT_JPEG;
        case IMG_ENCODE_PNG:
            return image::FMT_PNG;
        default:
            throw err::Exception(err::ERR_NOT_IMPL, "not support image format");
        }
    }

    bool maixvision_mode()
    {
        // get MAIXVISION variable from env
        char *env = getenv("MAIXVISION");
        if (env && strcmp(env, "1") == 0)
        {
            return true;
        }
        return false;
    }

    image::Format maixvision_image_fmt()
    {
        char *env = getenv("MAIXVISION_IMG_FMT");
        if (env)
        {
            switch (atoi(env))
            {
            case IMG_TRANS_FMT_NONE:
                return image::Format::FMT_INVALID;
            case IMG_TRANS_FMT_JPEG:
                return image::Format::FMT_JPEG;
            case IMG_TRANS_FMT_PNG:
                return image::Format::FMT_PNG;
            default:
                break;
            }
        }
        return image::Format::FMT_JPEG;;
    }

    void on_open(client *c, websocketpp::connection_hdl hdl, ClientHandle *handle)
    {
        log::debug("send image connection open\n");
        handle->conn_connected = true;
    }

    void on_close(client *c, websocketpp::connection_hdl hdl, ClientHandle *handle)
    {
        log::debug("send image connection close\n");
        handle->conn_connected = false;
    }

    inline uint8_t sum_uint8(uint8_t *data, size_t len)
    {
        uint8_t sum = 0;
        for (size_t i = 0; i < len; i++)
        {
            sum += data[i];
        }
        return sum;
    }

    // This message handler will be invoked once for each incoming message. It
    // prints the message and then sends a copy of the message back to the server.
    void on_message(client *c, websocketpp::connection_hdl hdl, message_ptr msg, ClientHandle *handle)
    {
        uint8_t frame[] = {0xAC, 0xBE, 0xCB, 0xCA, 0x04, 0x00, 0x00, 0x00, P_VERSION, MSG_ID_CONN_ACK, 0x01, 0x00, 0x00};
        uint8_t *data = (uint8_t *)msg->get_payload().c_str();
        uint32_t len = msg->get_payload().length();
        uint8_t *data_to_send = nullptr;
        size_t   data_send_len = 0;

        log::debug("recv message data len: %d\n", len);
        if(len >= 12 && data[9] == MSG_ID_CONN_ACK)
        {
            frame[11] = sum_uint8(frame, 11);
            if(memcmp(data, frame, 12) == 0)
            {
                log::debug("recv connect ack\n");
                handle->init = true;
            }
            else
            {
                frame[10] = 0;
                frame[11] = sum_uint8(frame, 11);
                if(memcmp(data, frame, 12) == 0)
                {
                    handle->conn_fail = true;
                    log::info("recv connect fail ack\n");
                }
            }
        }
        else if(len >= 12 && data[9] == MSG_ID_SET_FMT)
        {
            uint8_t sum = sum_uint8(data, 11);
            if(sum != data[11])
            {
                log::error("recv set fmt msg sum error\n");
            }
            else
            {
                uint8_t new_fmt = data[10];
                ((uint32_t*)frame)[1] = 5;
                if(new_fmt >= IMG_TRANS_FMT_MAX)
                {
                    log::error("recv set fmt msg fmt error\n");
                    frame[10] = 0;
                }
                else
                {
                    frame[10] = 1;
                    handle->img_trans->set_format(get_format_from_id(new_fmt));
                }
                frame[9] = MSG_ID_SET_FMT_ACK;
                frame[11] = new_fmt;
                frame[12] = sum_uint8(frame, 12);
                data_to_send = frame;
                data_send_len = 13;
            }
        }

        if(data_to_send)
        {
            c->send(hdl, data_to_send, data_send_len, websocketpp::frame::opcode::binary);
        }

    }

    void send_image_process(void *args)
    {
        ClientHandle *handle = (ClientHandle *)args;
        client *c = handle->c;
        websocketpp::connection_hdl hdl = handle->hdl;
        websocketpp::lib::error_code ec;

        log::debug("send image to maixvision thread started\n");

        // wait for connection success, timeout 10s
        int timeout = 100;
        while(!handle->conn_connected)
        {
            time::sleep_ms(100);
            timeout--;
            if(timeout <= 0)
            {
                log::error("connect maixvision service timeout\n");
                handle->init = false;
                return;
            }
        }

        log::debug("connect maixvision service success\n");

        // send connection cmd to server
        int conn_fail_cnt = 0;
        uint8_t frame[] = {0xAC, 0xBE, 0xCB, 0xCA, 0x07, 0x00, 0x00, 0x00, P_VERSION, MSG_ID_CONN, 'c', 'o', 'd', 'e', 0x00};
        for (size_t i = 0; i < sizeof(frame) - 1; i++)
        {
            frame[sizeof(frame) - 1] += frame[i];
        }
        c->send(hdl, frame, sizeof(frame), websocketpp::frame::opcode::binary, ec);
        if (ec)
        {
            log::error("send connect cmd because: %s", ec.message().c_str());
            handle->init = false;
        }
        uint64_t start_time = time::time_s();
        while(!handle->init)
        {
            if(time::time_s() - start_time > 10)
            {
                log::error("connect maixvision service timeout\n");
                handle->init = false;
                return;
            }
            time::sleep_ms(10);
            if(handle->conn_fail)
            {
                handle->conn_fail = false;
                conn_fail_cnt++;
                if(conn_fail_cnt > 3)
                {
                    log::error("connect failed !!!\n");
                    break;
                }
                time::sleep(2);
                log::error("connect failed, retry\n");
                c->send(hdl, frame, sizeof(frame), websocketpp::frame::opcode::binary, ec);
                if (ec)
                {
                    log::error("send connect cmd because: %s", ec.message().c_str());
                    handle->init = false;
                }
                return;
            }
        }

        while (handle->init)
        {
            if (handle->img_queue.empty())
            {
                time::sleep_ms(10);
                continue;
            }
            QueueItem &item = handle->img_queue.front();
            if (!handle->init)
                break;

            image::Image *img = item.p_img;
            // log::debug("send iamge, size: %ld", img.data_size());

            if(handle->frame_buffer.size() < (size_t)img->data_size() + 12)
            {
                handle->frame_buffer.resize(img->data_size() + 12);
                handle->frame_buffer[0] = 0xAC;
                handle->frame_buffer[1] = 0xBE;
                handle->frame_buffer[2] = 0xCB;
                handle->frame_buffer[3] = 0xCA;
                handle->frame_buffer[8] = P_VERSION;
                handle->frame_buffer[9] = MSG_ID_IMG;
            }
            handle->frame_buffer[10] = get_img_encode_id(handle->img_trans->get_format());
            memcpy(&handle->frame_buffer[11], img->data(), img->data_size());
            ((uint32_t*)handle->frame_buffer.data())[1] = img->data_size() + 4;
            uint8_t *p_sum = handle->frame_buffer.data() + img->data_size() + 11;
            uint8_t *p = handle->frame_buffer.data();
            *p_sum = 0;
            for(int i=0; i<img->data_size() + 11; i++)
            {
                *p_sum += *p++;
            }
            c->send(hdl, handle->frame_buffer.data(), img->data_size() + 12, websocketpp::frame::opcode::binary, ec);
            if (ec)
            {
                log::error("send failed because: %s", ec.message().c_str());
            }
            delete img;
            handle->img_queue.pop();
        }
        time::sleep_ms(20); // ensure ~ImageTrans's push(item) finish
        while(!handle->img_queue.empty())
        {
            QueueItem &item = handle->img_queue.front();
            delete item.p_img;
            handle->img_queue.pop();
        }
        handle->th_exit = true;
    }

    ImageTrans::ImageTrans(image::Format fmt, int quality)
    {
        this->set_format(fmt, quality);
        this->_handle = new ClientHandle();
        ClientHandle *handle = (ClientHandle*)this->_handle;
        handle->img_trans = this;
        handle->c = new client();
        try
        {
            // Set logging to be pretty verbose (everything except message payloads)
            handle->c->set_access_channels(websocketpp::log::alevel::none);
            handle->c->clear_access_channels(websocketpp::log::alevel::frame_payload);

            // Initialize ASIO
            handle->c->init_asio();

            // Register our message handler
            handle->c->set_message_handler(bind(&on_message, handle->c, ::_1, ::_2, handle));
            // connected handler
            handle->c->set_open_handler(bind(&on_open, handle->c, ::_1, handle));
            // close handler
            handle->c->set_close_handler(bind(&on_close, handle->c, ::_1, handle));

            websocketpp::lib::error_code ec;
            client::connection_ptr con = handle->c->get_connection(WS_SERVER_URI, ec);
            if (ec)
            {
                log::error("get connection error: %s", ec.message().c_str());
                return;
            }

            // Note that connect here only requests a connection. No network messages are
            // exchanged until the event loop starts running in the next line.
            handle->c->connect(con);
            handle->hdl = con->get_handle();
            handle->th_exit = false;

            // new thread to send message
            thread::Thread t = thread::Thread(send_image_process, handle);
            log::debug("start send image thread\n");
            t.detach();

            // Start the ASIO io_service run loop
            // this will cause a single connection to be made to the server. handle->c->run()
            // will exit when this connection is closed.
            thread::Thread t2 = thread::Thread([](void *args){
                ClientHandle *handle = (ClientHandle *)args;
                handle->c->run();
            }, handle);
            log::debug("start send image thread2\n");
            t2.detach();

        }
        catch (websocketpp::exception const &e)
        {
            log::error("create connection exception: %s", e.what());
        }
    }

    ImageTrans::~ImageTrans()
    {
        ClientHandle *handle = (ClientHandle *)this->_handle;
        handle->init = false;
        QueueItem item;
        item.p_img = new image::Image(10, 10);
        handle->img_queue.push(item);
        while (!handle->th_exit)
        {
            time::sleep_ms(10);
        }
        delete handle->c;
        delete handle;
    }

    err::Err ImageTrans::set_format(image::Format fmt, int quality)
    {
        if(fmt != image::FMT_JPEG && fmt != image::FMT_PNG && fmt != image::FMT_INVALID)
        {
            return err::ERR_ARGS;
        }
        this->_fmt = fmt;
        this->_quality = quality;
        return err::ERR_NONE;
    }

    err::Err ImageTrans::send_image(image::Image &img)
    {
        ClientHandle *handle = (ClientHandle *)this->_handle;
        if(!handle->init)
        {
            uint64_t t = time::ticks_ms();
            while (!handle->init)
            {
                if(time::ticks_ms() - t > 500)
                    return err::Err::ERR_NOT_READY;
            }
        }
        image::Image *compressed = &img;
        QueueItem item;
        if(_fmt == image::FMT_INVALID) // pause send mode
        {
            return err::Err::ERR_NONE;
        }
        // compress image to jpeg
        if (img.format() != _fmt)
        {
            if(_fmt == image::FMT_JPEG)
            {
                compressed = img.to_jpeg(this->_quality);
            }
            else
            {
                compressed = img.to_format(_fmt);
            }
            if (compressed == nullptr)
            {
                log::error("compress image failed\n");
                return err::Err::ERR_RUNTIME;
            }
        }
        else
        {
            compressed = img.copy();
        }
        item.p_img = compressed;
        handle->img_queue.push(item);
        return err::Err::ERR_NONE;
    }

} // namespace maix
