#include <iostream>

#include "maix_basic.hpp"
#include "main.h"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>


typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace maix;


// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

// This message handler will be invoked once for each incoming message. It
// prints the message and then sends a copy of the message back to the server.
void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;


    // websocketpp::lib::error_code ec;

    // c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
    // if (ec) {
    //     std::cout << "Echo failed because: " << ec.message() << std::endl;
    // }
}

struct send_thread_args{
    client *c;
    websocketpp::connection_hdl hdl;
};

int _main(int argc, char* argv[]) {
    // Create a client endpoint
    client c;

    std::string uri = "ws://localhost:9002";

    if (argc == 2) {
        uri = argv[1];
    }

    try {
        // Set logging to be pretty verbose (everything except message payloads)
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        c.init_asio();

        // Register our message handler
        c.set_message_handler(bind(&on_message,&c,::_1,::_2));

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return 0;
        }

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        c.connect(con);

        send_thread_args args;
        args.c = &c;
        args.hdl = con->get_handle();

        // new thread to send message
        maix::thread::Thread t = maix::thread::Thread([](void *args){
            send_thread_args *args_ = (send_thread_args *)args;
            client *c = args_->c;
            websocketpp::connection_hdl hdl = args_->hdl;
            int count = 0;
            log::info("send thread started\n");
            while(1){
                websocketpp::lib::error_code ec;
                std::string msg = "hello world " + std::to_string(count++) + "\n";
                log::info("send message: %s", msg.c_str());
                c->send(hdl, msg, websocketpp::frame::opcode::text, ec);
                if (ec) {
                    std::cout << "Echo failed because: " << ec.message() << std::endl;
                }
                time::sleep(5);

            }
        }, &args);
        log::info("start thread\n");
        t.detach();

        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        c.run();
    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


