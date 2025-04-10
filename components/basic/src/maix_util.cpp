#include "maix_util.hpp"
#include <vector>
#include <string>
#include <string_view>


namespace maix::util
{
    std::string str_strip(std::string &s)
    {
        size_t start = 0;
        size_t end = s.size();
        while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) {
            ++start;
        }
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
            --end;
        }
        return s.substr(start, end - start);
    }

    void str_strip_replace(std::string &s)
    {
        size_t start = 0, end = s.size();

        while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;

        if (start > 0 || end < s.size()) {
            s.erase(end, std::string::npos);
            s.erase(0, start);
        }
    }

    std::string_view str_strip_view(std::string_view str) {
        size_t start = 0;
        size_t end = str.size();
        while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
            ++start;
        }
        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
            --end;
        }
        return str.substr(start, end - start);
    }

    std::vector<std::string_view> str_split_view(std::string_view str, char delimiter)
    {
        std::vector<std::string_view> result;
        size_t start = 0;
        while (start < str.size()) {
            size_t end = str.find(delimiter, start);
            if (end == std::string_view::npos) {
                result.emplace_back(str.substr(start));
                break;
            } else {
                result.emplace_back(str.substr(start, end - start));
                start = end + 1;
            }
        }
        return result;
    }

    std::vector<std::string_view> str_split_view(std::string_view str, std::string_view delimiter)
    {
        std::vector<std::string_view> result;
        size_t pos = 0;
        while (true) {
            size_t next = str.find(delimiter, pos);
            if (next == std::string_view::npos) {
                result.emplace_back(str.substr(pos));
                break;
            }
            result.emplace_back(str.substr(pos, next - pos));
            pos = next + delimiter.size();
        }
        return result;
    }

     std::vector<std::string> str_split(const std::string &str, char delimiter) {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end;
        while ((end = str.find(delimiter, start)) != std::string::npos) {
            result.emplace_back(str.substr(start, end - start));
            start = end + 1;
        }
        result.emplace_back(str.substr(start));  // last part
        return result;
    }

    std::vector<std::string> str_split(const std::string &str, const std::string &delimiter) {
        std::vector<std::string> result;
        if (delimiter.empty()) {  // avoid infinite loop
            result.push_back(str);
            return result;
        }

        size_t start = 0;
        size_t end;
        while ((end = str.find(delimiter, start)) != std::string::npos) {
            result.emplace_back(str.substr(start, end - start));
            start = end + delimiter.length();
        }
        result.emplace_back(str.substr(start));  // last part
        return result;
    }

    std::vector<std::string_view> str_splitn_view(std::string_view str, char delimiter, int n) {
        std::vector<std::string_view> result;

        if (n <= 0) {
            result.push_back(str);
            return result;
        }

        size_t start = 0;
        while (n > 0) {
            size_t pos = str.find(delimiter, start);
            if (pos == std::string_view::npos) {
                break;
            }
            result.emplace_back(str.substr(start, pos - start));
            start = pos + 1;
            --n;
        }

        result.emplace_back(str.substr(start));
        return result;
    }

    std::vector<std::string_view> str_splitn_view(std::string_view str, std::string_view delimiter, int n)
    {
        std::vector<std::string_view> result;

        size_t start = 0;
        for (int i = 0; i < n; ++i) {
            size_t pos = str.find(delimiter, start);
            if (pos == std::string_view::npos) {
                break;
            }
            result.emplace_back(str.substr(start, pos - start));
            start = pos + delimiter.size();
        }

        result.emplace_back(str.substr(start));
        return result;
    }

     std::vector<std::string> str_splitn(const std::string& str, char delimiter, int n) {
        std::vector<std::string> result;

        if (n <= 0) {
            result.push_back(str);
            return result;
        }

        size_t start = 0;
        while (n > 0) {
            size_t pos = str.find(delimiter, start);
            if (pos == std::string::npos) {
                break;
            }
            result.emplace_back(str.substr(start, pos - start));
            start = pos + 1;
            --n;
        }

        result.emplace_back(str.substr(start));  // 添加剩余部分
        return result;
    }

    std::vector<std::string> str_splitn(const std::string& str, const std::string& delimiter, int n) {
        std::vector<std::string> result;

        if (delimiter.empty() || n <= 0) {
            result.push_back(str);
            return result;
        }

        size_t start = 0;
        while (n > 0) {
            size_t pos = str.find(delimiter, start);
            if (pos == std::string::npos) {
                break;
            }
            result.emplace_back(str.substr(start, pos - start));
            start = pos + delimiter.length();
            --n;
        }

        result.emplace_back(str.substr(start));  // 添加剩余部分
        return result;
    }
}