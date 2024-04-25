/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_err.hpp"
#include <exception>
#include <assert.h>
#include <vector>

namespace maix::err
{
    static std::string error_str = "No error";

    const std::vector<std::string> err_str = {
        "No error",
        "Invalid arguments",
        "No memory",
        "Not implemented",
        "Not ready",
        "Not initialized",
        "Not opened",
        "Not permitted",
        "Re-open",
        "Busy",
        "Read error",
        "Write error",
        "Timeout",
        "Runtime error",
        "IO error",
        "Not found",
        "Already exist",
        "Buffer full",
        "Buffer empty",
        "Cancel",
        "Overflow",
    };

    std::string& get_error()
    {
        return error_str;
    }

    void set_error(const std::string &str)
    {
        error_str = str;
    }

    void set_error(const char *str)
    {
        error_str = str;
    }

    std::string to_str(Err e)
    {
        // size of err_str must equal to ERR_MAX
        assert(err_str.size() == ERR_MAX);

        if(e < ERR_NONE || e >= ERR_MAX)
        {
            return "Invalid error code";
        }
        return err_str[e];
    }

    void check_raise(Err e, const std::string &msg)
    {
        if(e != ERR_NONE)
        {
            std::string err_str = to_str(e);
            if(msg.size() > 0)
            {
                err_str += ": " + msg + "\n";
            }
            throw Exception(err_str);
        }
    }

    void check_bool_raise(bool ok, const std::string &msg)
    {
        if(!ok)
        {
            std::string err_str = "Unknown error";
            if(msg.size() > 0)
            {
                err_str += ": " + msg + "\n";
            }
            throw Exception(err_str);
        }
    }

    void check_null_raise(void *ptr, const std::string &msg)
    {
        if(!ptr)
        {
            std::string err_str = "Value is NULL";
            if(msg.size() > 0)
            {
                err_str += ": " + msg + "\n";
            }
            throw Exception(err_str);
        }
    }

    Exception::Exception(const std::string &msg, err::Err code)
    {
        if (code == err::ERR_NONE)
        {
            _msg = "";
        }
        else
        {
            _msg = to_str(code);
        }
        if(msg.size() > 0)
        {
            _msg += ": " + msg + "\n";
        }
        _code = code;
    }

    Exception::Exception(err::Err code, const std::string &msg)
    {
        if (code == err::ERR_NONE)
        {
            _msg = "";
        }
        else
        {
            _msg = to_str(code);
        }
        if(msg.size() > 0)
        {
            _msg += ": " + msg + "\n";
        }
        _code = code;
    }

    const char *Exception::what() const throw()
    {
        return _msg.data();
    }

    err::Err Exception::code() const
    {
        return _code;
    }

} // namespace maix::err
