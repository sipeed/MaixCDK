/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2025.8.7: update this file.
 */

 #include <string>
 #include <regex>
 #include <cctype>


 namespace maix::peripheral::gpio
 {

    bool maix_gpio_port_parse_pin(const std::string& pin_str, int& group, int& index) {
        // GPIO0_A4  --> 0, 4
        // IO1_A16 --> 1, 16
         std::regex pattern(R"(^(GPIO|IO)(\d{1,2})_A(\d{1,2})$)");
         std::smatch match;

         if (std::regex_match(pin_str, match, pattern)) {
             group = std::stoi(match[2].str());
             index = std::stoi(match[3].str());
             return true;
         }
         return false;
     }

 }; // namespace maix::peripheral::gpio
