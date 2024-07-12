/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_fs.hpp"
#include "maix_log.hpp"
#include <stdio.h>
#include <unistd.h>

#if __cplusplus < 201703L
#include <experimental/filesystem>
namespace fs_sys = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs_sys = std::filesystem;
#endif

namespace maix::fs
{
    bool isabs(const std::string &path)
    {
        // check if path is a absolute path use fs_sys::path::is_absolute
        fs_sys::path p(path);
        return p.is_absolute();
    }

    bool isdir(const std::string &path)
    {
        // check if path is a directory use fs_sys::is_directory
        return fs_sys::is_directory(path);
    }

    bool isfile(const std::string &path)
    {
        // check if path is a file use fs_sys::is_regular_file
        return fs_sys::is_regular_file(path);
    }

    bool islink(const std::string &path)
    {
        // check if path is a link use fs_sys::is_symlink
        return fs_sys::is_symlink(path);
    }

    err::Err symlink(const std::string &src, const std::string &link, bool force)
    {
        // 检查源文件是否存在
        if (!fs::exists(src))
        {
            return err::Err::ERR_NOT_FOUND;
        }

        // 删除已存在的软链接
        if (fs::exists(link))
        {
            if (!force)
            {
                return err::Err::ERR_ALREAY_EXIST;
            }
            try
            {
                fs::remove(link);
            }
            catch (...)
            {
                return err::Err::ERR_IO;
            }
        }

        // 创建软链接
        try
        {
            fs_sys::create_symlink(src, link);
        }
        catch (...)
        {
            return err::Err::ERR_IO;
        }
        return err::Err::ERR_NONE;
    }

    bool exists(const std::string &path)
    {
        // check if path exists use fs_sys::exists
        return fs_sys::exists(path);
    }

    err::Err mkdir(const std::string &path, bool exist_ok, bool recursive)
    {
        // create a directory use fs_sys::create_directories
        // if exist_ok is true, also return true if directory already exists
        if (!exist_ok && fs_sys::exists(path))
        {
            return err::ERR_ALREAY_EXIST;
        }
        if (recursive)
        {
            fs_sys::create_directories(path);
            return err::ERR_NONE;
        }
        else
        {
            fs_sys::create_directory(path);
            return err::ERR_NONE;
        }
    }

    err::Err rmdir(const std::string &path, bool recursive)
    {
        // remove a directory use fs_sys::remove_all
        if (!fs_sys::exists(path))
        {
            return err::ERR_NOT_FOUND;
        }
        if (recursive)
        {
            fs_sys::remove_all(path);
            return err::ERR_NONE;
        }
        else
        {
            fs_sys::remove(path);
            return err::ERR_NONE;
        }
    }

    err::Err remove(const std::string &path)
    {
        // remove a file use fs_sys::remove
        if (!fs_sys::exists(path))
        {
            return err::ERR_NOT_FOUND;
        }
        fs_sys::remove(path);
        return err::ERR_NONE;
    }

    err::Err rename(const std::string &src, const std::string &dst)
    {
        // rename a file or directory use fs_sys::rename
        if (!fs_sys::exists(src))
        {
            return err::ERR_NOT_FOUND;
        }
        fs_sys::rename(src, dst);
        return err::ERR_NONE;
    }

    void sync()
    {
        ::sync();
    }

    int getsize(const std::string &path)
    {
        // get file size use fs_sys::file_size
        if (!fs_sys::exists(path))
        {
            return -err::ERR_NOT_FOUND;
        }
        return fs_sys::file_size(path);
    }

    std::string dirname(const std::string &path)
    {
        fs_sys::path p(path);
        std::string ret = p.parent_path().string();
        if (ret.empty())
        {
            ret = ".";
        }
        return ret;
    }

    std::string basename(const std::string &path)
    {
        fs_sys::path p(path);
        return p.filename().string();
    }

    std::string abspath(const std::string &path)
    {
        return fs_sys::absolute(path).string();
    }

    std::string getcwd()
    {
        return fs_sys::absolute(fs_sys::current_path()).string();
    }

    std::string realpath(const std::string &path)
    {
        return fs_sys::canonical(path).string();
    }

    std::vector<std::string> splitext(const std::string &path)
    {
        fs_sys::path p(path);
        std::vector<std::string> result;

        // 获取文件的后缀名
        std::string extension = p.extension().string();

        // 获取文件的前缀
        std::string stem = p.stem().string();
        std::string parent_path = p.parent_path().string();

        // 拼接前缀路径和文件名
        std::string prefix = parent_path.empty() ? stem : parent_path + fs_sys::path::preferred_separator + stem;

        result.push_back(prefix);
        result.push_back(extension);

        return result;
    }

    std::vector<std::string> *listdir(const std::string &path, bool recursive, bool full_path)
    {
        // list directory use fs_sys::directory_iterator
        if (!fs_sys::exists(path))
        {
            return nullptr;
        }
        std::vector<std::string> *list = new std::vector<std::string>();
        if (recursive)
        {
            if (full_path)
                for (auto &p : fs_sys::recursive_directory_iterator(path))
                {
                    list->push_back(p.path().string());
                }
            else
                for (auto &p : fs_sys::recursive_directory_iterator(path))
                {
                    list->push_back(p.path().filename().string());
                }
        }
        else
        {
            if (full_path)
                for (auto &p : fs_sys::directory_iterator(path))
                {
                    list->push_back(p.path().string());
                }
            else
                for (auto &p : fs_sys::directory_iterator(path))
                {
                    list->push_back(p.path().filename().string());
                }
        }
        return list;
    }

    fs::File *open(const std::string &path, const std::string &mode)
    {
        err::Err error = err::ERR_NONE;
        fs::File *file = new fs::File();
        error = file->open(path, mode);
        if (error != err::ERR_NONE)
        {
            log::error("open file %s failed, error code: %d\n", path.c_str(), error);
            delete file;
            return nullptr;
        }
        return file;
    }

    std::string tempdir()
    {
        return fs_sys::temp_directory_path().string();
    }

    err::Err File::open(const std::string &path, const std::string &mode)
    {
        // open file use std::fopen
        if (_fp != nullptr)
        {
            return err::ERR_NOT_READY;
        }
        _fp = std::fopen(path.c_str(), mode.c_str());
        if (_fp == nullptr)
        {
            log::error("open file %s failed\n", path.c_str());
            return err::ERR_ARGS;
        }
        return err::ERR_NONE;
    }

    void File::close()
    {
        // close file use std::fclose
        if (_fp != nullptr)
        {
            std::fclose((FILE *)_fp);
            _fp = nullptr;
        }
    }

    int File::read(void *buf, int size)
    {
        // read data from file use std::fread
        if (_fp == nullptr)
        {
            return -err::ERR_NOT_READY;
        }
        return std::fread(buf, 1, size, (FILE *)_fp);
    }

    std::vector<uint8_t> *File::read(int size)
    {
        // read data from file use std::fread
        if (_fp == nullptr)
        {
            log::error("file not opened\n");
            return nullptr;
        }
        std::vector<uint8_t> *buf = new std::vector<uint8_t>(size);
        int read_size = std::fread(buf->data(), 1, size, (FILE *)_fp);
        if (read_size < 0)
        {
            delete buf;
            return nullptr;
        }
        buf->resize(read_size);
        return buf;
    }

    int File::readline(std::string &line)
    {
        // read line from file use std::fgets
        if (_fp == nullptr)
        {
            return -err::ERR_NOT_OPEN;
        }
        char buf[1024] = {0};
        if (std::fgets(buf, 1024, (FILE *)_fp) == nullptr)
        {
            return 0;
        }
        line = buf;
        return line.size();
    }

    std::string *File::readline()
    {
        // read line from file use std::fgets
        if (_fp == nullptr)
        {
            throw err::Exception(err::ERR_NOT_OPEN, "file not opened");
        }
        char buf[1024] = {0};
        if (std::fgets(buf, 1024, (FILE *)_fp) == nullptr)
        {
            return new std::string();
        }
        std::string *line = new std::string(buf);
        return line;
    }

    /**
     * End of file or not
     * @return 0 if not reach end of file, else eof.
     * @maixpy maix.fs.File.eof
     */
    int File::eof()
    {
        return std::feof((FILE *)_fp);
    }

    int File::write(const void *buf, int size)
    {
        // write data to file use std::fwrite
        if (_fp == nullptr)
        {
            return -err::ERR_NOT_READY;
        }
        return std::fwrite(buf, 1, size, (FILE *)_fp);
    }

    int File::write(const std::vector<uint8_t> &buf)
    {
        // write data to file use std::fwrite
        if (_fp == nullptr)
        {
            return -err::ERR_NOT_READY;
        }
        return std::fwrite(buf.data(), 1, buf.size(), (FILE *)_fp);
    }

    int File::seek(int offset, int whence)
    {
        // seek file position use std::fseek
        if (_fp == nullptr)
        {
            return -err::ERR_NOT_READY;
        }
        return std::fseek((FILE *)_fp, offset, whence);
    }

    int File::tell()
    {
        // get file position use std::ftell
        if (_fp == nullptr)
        {
            return -err::ERR_NOT_READY;
        }
        return std::ftell((FILE *)_fp);
    }

    err::Err File::flush()
    {
        // flush file use std::fflush
        if (_fp == nullptr)
        {
            return err::ERR_NOT_READY;
        }
        std::fflush((FILE *)_fp);
        return err::ERR_NONE;
    }

} // namespace maix::fs
