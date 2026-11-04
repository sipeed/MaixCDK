#include "maix_basic.hpp"
#include "main.h"
#include "maix_wifi.hpp"

#include <charconv>
#include <cstdio>
#include <string>

using namespace maix;
using namespace maix::network;

namespace
{
    enum class Command
    {
        SCAN,
        STATUS,
        STA_CONNECT,
        STA_DISCONNECT,
        AP_START,
        AP_STOP,
    };

    struct CommandArgs
    {
        Command command = Command::STATUS;
        std::string ssid;
        std::string password;
        bool wait = true;
        int timeout = 60;
        std::string mode = "g";
        int channel = 0;
        std::string ip = "192.168.66.1";
        std::string netmask = "255.255.255.0";
        bool hidden = false;
    };

    void print_help(const char *program)
    {
        std::printf(
            "WiFi command-line demo\n"
            "\n"
            "Usage:\n"
            "  %s --help\n"
            "  %s scan\n"
            "  %s status\n"
            "  %s sta-connect <ssid> <password> [wait=true] [timeout=60]\n"
            "  %s sta-disconnect\n"
            "  %s ap-start <ssid> <password> [mode=g] [channel=0] "
            "[ip=192.168.66.1] [netmask=255.255.255.0] [hidden=false]\n"
            "  %s ap-stop\n"
            "\n"
            "Commands:\n"
            "  scan            Scan nearby access points and print network information.\n"
            "  status          Print interface, mode, connection, SSID, IP, MAC, and gateway.\n"
            "  sta-connect     Connect in station mode. Empty password selects an open network.\n"
            "  sta-disconnect  Disconnect from the current station network; on MaixCAM2,\n"
            "                  wlan0 stays available so scan can run immediately.\n"
            "  ap-start        Start an access point. Modes are g, a, or b; channel 0 uses\n"
            "                  the public API default. Empty password selects an open AP.\n"
            "  ap-stop         Stop access-point mode; on MaixCAM2, start STA mode.\n"
            "\n"
            "MaixCAM2 recovery:\n"
            "  sta-connect and ap-start ask the public Wifi API to start wifi.service if\n"
            "  wlan0 is externally unavailable, then wait up to about 5 seconds for it.\n"
            "  Normal sta-disconnect keeps wifi.service and wlan0 available for scanning.\n"
            "  Other commands never start the service implicitly.\n"
            "\n"
            "Boolean values:\n"
            "  true, false, 1, or 0\n"
            "\n"
            "Examples (replace placeholders with device-test values):\n"
            "  %s scan\n"
            "  %s sta-connect YOUR_STA_SSID 'YOUR_STA_PASSWORD'\n"
            "  %s sta-connect OPEN_STA '' false 60\n"
            "  %s ap-start DEMO_AP 'YOUR_AP_PASSWORD' g 6\n"
            "  %s ap-start OPEN_DEMO_AP '' g 0 192.168.66.1 255.255.255.0 true\n"
            "\n"
            "Passwords are accepted as arguments but are never printed by this demo.\n",
            program, program, program, program, program, program, program,
            program, program, program, program, program);
    }

    bool parse_int(const char *text, int &value)
    {
        if(text == nullptr || text[0] == '\0')
            return false;
        const char *end = text + std::char_traits<char>::length(text);
        const std::from_chars_result result = std::from_chars(text, end, value);
        return result.ec == std::errc() && result.ptr == end;
    }

    bool parse_bool(const char *text, bool &value)
    {
        const std::string input(text == nullptr ? "" : text);
        if(input == "true" || input == "1")
        {
            value = true;
            return true;
        }
        if(input == "false" || input == "0")
        {
            value = false;
            return true;
        }
        return false;
    }

    bool parse_arguments(int argc, char *argv[], CommandArgs &args, std::string &error)
    {
        if(argc < 2)
        {
            error = "missing command";
            return false;
        }

        const std::string command(argv[1]);
        if(command == "scan" || command == "status" || command == "sta-disconnect" || command == "ap-stop")
        {
            if(argc != 2)
            {
                error = "this command does not accept arguments";
                return false;
            }
            if(command == "scan")
                args.command = Command::SCAN;
            else if(command == "status")
                args.command = Command::STATUS;
            else if(command == "sta-disconnect")
                args.command = Command::STA_DISCONNECT;
            else
                args.command = Command::AP_STOP;
            return true;
        }

        if(command == "sta-connect")
        {
            if(argc < 4 || argc > 6)
            {
                error = "sta-connect expects <ssid> <password> [wait] [timeout]";
                return false;
            }
            args.command = Command::STA_CONNECT;
            args.ssid = argv[2];
            args.password = argv[3];
            if(argc >= 5 && !parse_bool(argv[4], args.wait))
            {
                error = "wait must be true, false, 1, or 0";
                return false;
            }
            if(argc >= 6 && (!parse_int(argv[5], args.timeout) || args.timeout < 0))
            {
                error = "timeout must be a non-negative decimal integer";
                return false;
            }
            return true;
        }

        if(command == "ap-start")
        {
            if(argc < 4 || argc > 9)
            {
                error = "ap-start expects <ssid> <password> [mode] [channel] [ip] [netmask] [hidden]";
                return false;
            }
            args.command = Command::AP_START;
            args.ssid = argv[2];
            args.password = argv[3];
            if(argc >= 5)
                args.mode = argv[4];
            if(argc >= 6 && !parse_int(argv[5], args.channel))
            {
                error = "channel must be a decimal integer";
                return false;
            }
            if(argc >= 7)
                args.ip = argv[6];
            if(argc >= 8)
                args.netmask = argv[7];
            if(argc >= 9 && !parse_bool(argv[8], args.hidden))
            {
                error = "hidden must be true, false, 1, or 0";
                return false;
            }
            return true;
        }

        error = "unknown command";
        return false;
    }

    int report_api_result(const char *operation, err::Err result)
    {
        if(result != err::ERR_NONE)
        {
            log::error("%s failed: %s", operation, err::to_str(result).c_str());
            return 1;
        }
        log::info("%s succeeded", operation);
        return 0;
    }

    void print_status(wifi::Wifi &device, const std::string &iface)
    {
        log::info("WiFi interface: %s", iface.c_str());
        log::info("Mode: %s", device.is_ap_mode() ? "AP" : "STA");
        log::info("Connected: %s", device.is_connected() ? "true" : "false");
        // Force a refresh because this command did not populate the Wifi object's SSID cache.
        log::info("SSID: %s", device.get_ssid(false).c_str());
        log::info("IP: %s", device.get_ip().c_str());
        log::info("MAC: %s", device.get_mac().c_str());
        log::info("Gateway: %s", device.get_gateway().c_str());
    }

    int run_command(const CommandArgs &args, wifi::Wifi &device, const std::string &iface)
    {
        switch(args.command)
        {
        case Command::SCAN:
        {
            const err::Err result = device.start_scan();
            if(result != err::ERR_NONE)
                return report_api_result("scan start", result);

            log::info("wait for 3s to scan wifi");
            time::sleep(3);
            std::vector<wifi::AP_Info> scan_result;
            try
            {
                scan_result = device.get_scan_result();
            }
            catch(...)
            {
                device.stop_scan();
                throw;
            }
            device.stop_scan();
            log::info("ssid, bssid, channel, rssi, security");
            for(auto &ap : scan_result)
            {
                log::info("%-20s, %s, %-10d, %-10d, %s",
                          ap.ssid_str().c_str(), ap.bssid.c_str(), ap.channel, ap.rssi,
                          ap.security.c_str());
            }
            return 0;
        }
        case Command::STATUS:
            print_status(device, iface);
            return 0;
        case Command::STA_CONNECT:
            return report_api_result("STA connect",
                                     device.connect(args.ssid, args.password, args.wait, args.timeout));
        case Command::STA_DISCONNECT:
            return report_api_result("STA disconnect", device.disconnect());
        case Command::AP_START:
            return report_api_result("AP start",
                                     device.start_ap(args.ssid, args.password, args.mode, args.channel,
                                                     args.ip, args.netmask, args.hidden));
        case Command::AP_STOP:
            return report_api_result("AP stop", device.stop_ap());
        }
        log::error("internal command dispatch error");
        return 1;
    }
}

int _main(int argc, char *argv[])
{
    if(argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))
    {
        print_help(argv[0]);
        return 0;
    }

    CommandArgs args;
    std::string argument_error;
    if(!parse_arguments(argc, argv, args, argument_error))
    {
        log::error("invalid arguments: %s", argument_error.c_str());
        print_help(argv[0]);
        return 2;
    }

    const std::vector<std::string> wifi_ifaces = wifi::list_devices();
    for(const auto &iface : wifi_ifaces)
        log::info("wifi iface: %s", iface.c_str());

    std::string wifi_iface;
    if(wifi_ifaces.empty())
    {
#if PLATFORM_MAIXCAM2
        if(args.command == Command::STA_CONNECT || args.command == Command::AP_START)
        {
            wifi_iface = "wlan0";
            log::info("wlan0 is not currently discoverable; the Wifi API will attempt service recovery");
        }
        else
#endif
        {
            log::error("no wifi iface found");
            return 1;
        }
    }
    else
    {
        wifi_iface = wifi_ifaces[0];
    }

    wifi::Wifi device(wifi_iface);
    return run_command(args, device, wifi_iface);
}

int main(int argc, char *argv[])
{
    sys::register_default_signal_handle();
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}
