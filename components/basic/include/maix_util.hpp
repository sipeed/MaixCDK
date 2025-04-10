/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include "maix_log.hpp"
#include <exception>
#include "maix_app.hpp"

namespace maix::util
{

    /**
     * @brief Run function with catch exception suuport
    */
    #define CATCH_EXCEPTION_RUN_RETURN(func, err_ret_value, ...) \
        while(1){ \
            maix::util::init_before_main();\
            try \
            { \
                int ret = (int)func(__VA_ARGS__); \
                if (!app::have_exit_msg()) \
                    app::set_exit_msg((err::Err)ret, "Unkown error"); \
                maix::util::do_exit_function(); \
                return ret; \
            } \
            catch(const std::exception& e) \
            { \
                std::string msg = "Exception: " + std::string(e.what()); \
                log::error("%s\n", msg.c_str()); \
                app::set_exit_msg(err::ERR_RUNTIME, msg); \
                maix::util::do_exit_function(); \
                return err_ret_value; \
            } \
            catch(const err::Err &e) \
            { \
                std::string msg = "Exception: " + err::to_str(e); \
                log::error("%s\n", msg.c_str()); \
                app::set_exit_msg(e, msg); \
                maix::util::do_exit_function(); \
                return err_ret_value; \
            } \
            catch(...) \
            { \
                std::string msg = "Unknown exception"; \
                log::error("%s\n", msg.c_str()); \
                app::set_exit_msg(err::ERR_RUNTIME, msg); \
                maix::util::do_exit_function(); \
                return err_ret_value; \
            } \
            return err_ret_value; \
            break; \
        } \

    /**
     * @brief disable the kernel debug
     * @maixcdk maix.util.disable_kernel_debug
    */
    void disable_kernel_debug();

    /**
     * @brief disable the kernel debug
     * @maixcdk maix.util.enable_kernel_debug
    */
    void enable_kernel_debug();

    /**
     * @brief Initialize before main
     * The function is used to add preparatory operations that need to be executed before the main program runs.
     * @maixpy maix.util.init_before_main
    */
    void init_before_main();

    /**
     * @brief register exit function
     * @maixcdk maix.util.register_exit_function
    */
    void register_exit_function(void (*process)(void));

    /**
     * @brief exec all of exit function
     * @maixpy maix.util.do_exit_function
    */
    void do_exit_function();

    /**
     * @brief Registering default processes that need to be executed on exit
     * @maixpy maix.util.register_atexit
    */
    void register_atexit();

    /**
     * @brief Enter any key to next step
     * @maixcdk maix.util.wait_any_key
    */
    inline void wait_any_key() {
        log::info(" Press any key to continue...");
        getchar();
    }

    /**
     * strip string, and return new striped string, will alloc new string.
     * @maixpy maix.util.str_strip
     */
    std::string str_strip(std::string &s);

    /**
     * strip string and replace string, won't alloc new string.
     * @maixcdk maix.util.str_strip_replace
     */
    void str_strip_replace(std::string &s);

    /**
     * return ad string view of strip string, only a view of string, so use it carefully!
     * @maixcdk maix.util.str_strip_view
     */
    std::string_view str_strip_view(std::string_view str);

    /**
     * split string with a charactor, and return a string view, so use it carefully.
     * @maixcdk maix.util.str_split_view
     */
    std::vector<std::string_view> str_split_view(std::string_view str, char delimiter);

    /**
     * split string with string, and return a string view, so use it carefully.
     * @maixcdk maix.util.str_split_view
     */
    std::vector<std::string_view> str_split_view(std::string_view str, std::string_view delimiter);


    /**
    * split string with a charactor, and return a new string.
    * @maixcdk maix.util.str_split
    */
    std::vector<std::string> str_split(const std::string &str, char delimiter);

    /**
    * split string with a charactor, and return a new string.
    * @maixcdk maix.util.str_split
    */
    std::vector<std::string> str_split(const std::string &str, const std::string &delimiter);

    /**
     * split n times(will get max n + 1 strings) with a charactor, and return a string view, so use it carefully.
     * @maixcdk maix.util.str_splitn_view
     */
    std::vector<std::string_view> str_splitn_view(std::string_view str, char delimiter, int n);

    /**
     * split n times(wll get max n + 1 strings) with string, and return a string view, so use it carefully.
     * @maixcdk maix.util.str_splitn_view
     */
    std::vector<std::string_view> str_splitn_view(std::string_view str, std::string_view delimiter);

    /**
     * split n times (will get max n + 1 strings) with char delimiter, and return a new string (safer version with copying).
     * @maixcdk maix.util.str_splitn
     */
     std::vector<std::string> str_splitn(const std::string& str, char delimiter, int n);

    /**
     * split n times (will get max n + 1 strings) with string delimiter, and return a new string (safer version with copying).
     * @maixcdk maix.util.str_splitn
     */
    std::vector<std::string> str_splitn(const std::string& str, const std::string& delimiter, int n);
}

