
#include "maix_basic.hpp"
#include "maix_comm.hpp"
#include "main.h"

using namespace maix;

#define APP_CMD_ECHO 0x01
#define APP_ID "my_app1"
#define BUFF_RX_LEN 1024

int _main(int argc, char* argv[])
{
    protocol::MSG *msg;

    app::set_app_id(APP_ID);

    // init communication object, will init uart or tcp server according to system config
    // we can get current setting by maix::app::get_sys_config_kv("comm", "method")
    comm::CommProtocol p = comm::CommProtocol(BUFF_RX_LEN);

    while(!app::need_exit())
    {
        // static uint8_t buff_rx[] = { 0xAA, 0xCA, 0xAC, 0xBB, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0xC8, 0xF5 };
        msg = p.get_msg();
        if(msg)
        {
            if(msg->is_resp)
                continue;
            if(msg->cmd == APP_CMD_ECHO)
            {
                log::info("recv echo cmd\n");
                std::string msg_content = "echo from app " + app::app_id() + "\n";
                p.resp_ok(msg->cmd, (uint8_t *)msg_content.c_str(), msg_content.length());
            }
            else if(msg->cmd == protocol::CMD_SET_REPORT)
            {
                p.resp_err(msg->cmd, err::ERR_NOT_IMPL, "this cmd not support auto upload");
            }
            delete msg;
        }
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


