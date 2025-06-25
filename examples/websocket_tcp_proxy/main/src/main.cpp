#include "main.h"
#include "maix_basic.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::connection_hdl;

using namespace maix;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;

int tcp_sock = -1;
std::mutex send_mutex;

bool connect_tcp_server(const char *host, int port)
{
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0)
    {
        perror("socket");
        return false;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(tcp_sock);
        return false;
    }

    if (connect(tcp_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(tcp_sock);
        return false;
    }

    return true;
}

void tcp_receive_loop(server *s, connection_hdl hdl)
{
    char buffer[1024];
    while (true)
    {
        ssize_t len = recv(tcp_sock, buffer, sizeof(buffer), 0);
        if (len <= 0)
        {
            perror("recv");
            break;
        }
        std::string msg(buffer, len);
        std::lock_guard<std::mutex> lock(send_mutex);
        s->send(hdl, msg, websocketpp::frame::opcode::binary);
    }
}

void on_message(server *s, connection_hdl hdl, message_ptr msg)
{
    const std::string &payload = msg->get_payload();
    send(tcp_sock, payload.data(), payload.size(), 0);
}

void on_connected(server *s, websocketpp::connection_hdl hdl)
{
    log::info("new client connected\n");
}

void on_disconnect(server *s, websocketpp::connection_hdl hdl)
{
    log::info("client disconnected\n");
}

int _main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <tcp_server_ip> <tcp_server_port> <websocket_listening_port>" << std::endl;
        return -1;
    }

    const char *tcp_ip = argv[1];
    int tcp_port = atoi(argv[2]);
    int websocket_port = atoi(argv[3]);

    printf("Connecting to TCP server at %s:%d...\n", tcp_ip, tcp_port);

    if (!connect_tcp_server(tcp_ip, tcp_port))
    {
        std::cerr << "Failed to connect to TCP server" << std::endl;
        return -1;
    }

    server echo_server;
    try
    {
        echo_server.set_access_channels(websocketpp::log::alevel::all);
        echo_server.clear_access_channels(websocketpp::log::alevel::frame_payload);
        echo_server.init_asio();
        echo_server.set_reuse_addr(true);
        echo_server.set_message_handler(bind(&on_message, &echo_server, _1, _2));

        echo_server.set_open_handler([&](connection_hdl hdl)
                                     {
                                         std::thread(tcp_receive_loop, &echo_server, hdl).detach();
                                     });
        // disconnect handler
        echo_server.set_close_handler(bind(&on_disconnect, &echo_server, ::_1));
        // connect handler
        echo_server.set_open_handler(bind(&on_connected, &echo_server, ::_1));

        echo_server.listen(websocket_port);
        echo_server.start_accept();
        printf("WebSocket server listening on port %d ...\n", websocket_port);
        echo_server.run();
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "other exception" << std::endl;
    }

    close(tcp_sock);
    printf("Program exit.\n");
    return 0;
}


// static void signal_handle(int signal)
// {
//     switch (signal) {
//     case SIGINT:
//         maix::app::set_exit_flag(true);
//         raise(SIGINT);
//     break;
//     default: break;
//     }
// }

int main(int argc, char* argv[])
{
    // Catch signal and process
    // sys::register_default_signal_handle();
    // signal(SIGINT, signal_handle);

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}


