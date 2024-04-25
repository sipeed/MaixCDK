/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>

namespace maix
{
namespace err
{
    /**
     * @brief Maix Error code
     * @maixpy maix.err.Err
    */
    enum Err
    {
        // !!! fixed error code, DO NOT change number already defined, only append new error code
        ERR_NONE        = 0,   // No error
        ERR_ARGS           ,   // Invalid arguments
        ERR_NO_MEM         ,   // No memory
        ERR_NOT_IMPL       ,   // Not implemented
        ERR_NOT_READY      ,   // Not ready
        ERR_NOT_INIT       ,   // Not initialized
        ERR_NOT_OPEN       ,   // Not opened
        ERR_NOT_PERMIT     ,   // Not permitted
        ERR_REOPEN         ,   // Re-open
        ERR_BUSY           ,   // Busy
        ERR_READ           ,   // Read error
        ERR_WRITE          ,   // Write error
        ERR_TIMEOUT        ,   // Timeout
        ERR_RUNTIME        ,   // Runtime error
        ERR_IO             ,   // IO error
        ERR_NOT_FOUND      ,   // Not found
        ERR_ALREAY_EXIST   ,   // Already exist
        ERR_BUFF_FULL      ,   // Buffer full
        ERR_BUFF_EMPTY     ,   // Buffer empty
        ERR_CANCEL         ,   // Cancel
        ERR_OVERFLOW       ,   // Overflow
        ERR_MAX,
    }; // !! Change this please update err_str in maix_err.cpp too !!

    /**
     * @brief Maix Exception
     * @maixpy maix.err.Exception
    */
    class Exception : public std::exception
    {
    public:
        Exception(const std::string &msg, err::Err code = err::ERR_NONE);
        Exception(const err::Err code, const std::string &msg = "");
        virtual const char *what() const throw();

        err::Err code() const;

    private:
        std::string _msg;
        err::Err _code;
    };

    /**
     * Error code to string
     * @param[in] e error code, err::Err type
     * @return error string
     * @maixpy maix.err.to_str
    */
    std::string to_str(err::Err e);

    /**
     * @brief get last error string
     * @return error string
     * @maixpy maix.err.get_error
    */
    std::string& get_error();

    /**
     * @brief set last error string
     * @param[in] str error string
     * @maixpy maix.err.set_error
    */
    void set_error(const std::string &str);

    /**
     * @brief set last error string
     * @param[in] str error string
    */
    void set_error(const char *str);

    /**
     * Check error code, if not ERR_NONE, raise err.Exception
     * @param[in] e error code, err::Err type
     * @param[in] msg error message
     * @maixpy maix.err.check_raise
    */
    void check_raise(err::Err e, const std::string &msg = "");


    /**
     * Check condition, if false, raise err.Exception
     * @param[in] ok condition, if true, do nothing, if false, raise err.Exception
     * @param[in] msg error message
     * @maixpy maix.err.check_bool_raise
    */
    void check_bool_raise(bool ok, const std::string &msg = "");

    /**
     * Check NULL pointer, if NULL, raise exception
     * @param[in] ptr pointer
     * @param[in] msg error message
     * @maixpy maix.err.check_null_raise
    */
    void check_null_raise(void *ptr, const std::string &msg = "");

}; // namespace err
}; // namespace maix


