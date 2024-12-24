#ifndef __BUHHNKKFCN_DEBUG_HPP__
#define __BUHHNKKFCN_DEBUG_HPP__


#include "maix_basic.hpp"
#include <stdexcept>

const char* TAG() noexcept
{
    return "APP ToF";
}

#define eprintln(fmt, ...) do {maix::log::error0("[%s]", TAG()); \
                                printf(fmt, ##__VA_ARGS__);\
                                printf("\n");} while(0)
#define println(fmt, ...) do {maix::log::info0("[%s]", TAG()); \
                                printf(fmt, ##__VA_ARGS__);\
                                printf("\n");} while(0)

#define dbg(fmt, ...) do {maix::log::info0("DEBUG<%d>", __LINE__);\
                            printf(fmt, ##__VA_ARGS__);\
                            printf("\n");} while(0)

#define panic(fmt, ...) do {eprintln(fmt, ##__VA_ARGS__);\
                            char _buff[256]{0x00}; \
                            ::snprintf(_buff, std::size(_buff), "In \n\tfile <%s> \n\tfunc <%s> \n\tlen <%d>\n", __FILE__, __PRETTY_FUNCTION__, __LINE__); \
                            throw std::runtime_error(std::string(_buff));} while(0)

#define check_ptr(ptr)     do { if (ptr == nullptr) {panic("Detected use of null pointer, terminated!");}} while(0)


#endif // __BUHHNKKFCN_DEBUG_HPP__