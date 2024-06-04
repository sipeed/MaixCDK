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
}

