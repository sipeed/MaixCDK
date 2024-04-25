#include "maix_network.hpp"
#include "maix_wifi.hpp"

namespace maix::network
{
    bool have_network()
    {
        wifi::Wifi w;
        if(w.is_connected())
            return true;
        return false;
    }
}

