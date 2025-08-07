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
        // GPIOB16 --> 1, 16
        // B16 --> 1, 16
        std::regex pattern(R"(^((GPIO)?)([A-Z])(\d{1,2})$)");
        std::smatch match;

        if (std::regex_match(pin_str, match, pattern)) {
            char group_char = match[3].str()[0];
            group = group_char - 'A';  // 'A' -> 0, 'B' -> 1, ..., 'Z' -> 25
            index = std::stoi(match[4].str());
            return true;
        }
        return false;
    }

 }; // namespace maix::peripheral::gpio
