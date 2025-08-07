#pragma once

#include <string>
#include <vector>


namespace maix::peripheral::uart
{
    std::vector<std::string> maix_uart_port_get_ports()
    {
        return std::vector<std::string>{
            "/dev/ttyS0",
            "/dev/ttyS1",
            "/dev/ttyS2",
            "/dev/ttyS3",
            "/dev/ttyS4"
        };
    }
}


