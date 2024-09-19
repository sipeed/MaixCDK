#include "maix_network.hpp"
#include "maix_wifi.hpp"

namespace maix::network
{
    static std::string _get_gateway(const std::string &_iface)
    {
        // get gateway ip
        std::string cmd = "ip route | grep default | grep " + _iface + " | awk '{print $3}'";
        FILE *fp = popen(cmd.c_str(), "r");
        if (fp == NULL)
        {
            return "";
        }
        char buf[1024];
        if (fgets(buf, sizeof(buf), fp) == NULL)
        {
            pclose(fp);
            return "";
        }
        pclose(fp);
        return buf;
    }

    bool have_network()
    {
        // check wlan0
        wifi::Wifi w;
        if(w.is_connected())
            return true;
        // check eth0
        if(!_get_gateway("eth0").empty())
            return true;
        return false;
    }
}

