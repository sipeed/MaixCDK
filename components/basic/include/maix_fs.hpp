/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once
#include "maix_err.hpp"
#include <stdint.h>
#include <vector>

#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END

namespace maix::fs
{
    /**
     * SEEK enums
     * @maixpy maix.fs.SEEK
    */
    enum SEEK
    {
        SEEK_SET = 0,  // Seek from beginning of file.
        SEEK_CUR = 1,  // Seek from current position.
        SEEK_END = 2,  // Seek from end of file.
    };

    /**
     * Check if the path is absolute path
     * @param path path to check
     * @return true if path is absolute path
     * @maixpy maix.fs.isabs
     */
    bool isabs(const std::string &path);

    /**
     * Check if the path is a directory, if not exist, throw exception
     * @param path path to check
     * @return true if path is a directory
     * @maixpy maix.fs.isdir
    */
    bool isdir(const std::string &path);

    /**
     * Check if the path is a file, if not exist, throw exception
     * @param path path to check
     * @return true if path is a file
     * @maixpy maix.fs.isfile
    */
    bool isfile(const std::string &path);

    /**
     * Check if the path is a link, if not exist, throw exception
     * @param path path to check
     * @return true if path is a link
     * @maixpy maix.fs.islink
    */
    bool islink(const std::string &path);

    /**
     * Create soft link
     * @param src real file path
     * @param link link file path
     * @param force force link, if already have link file, will delet it first then create.
     * @maixpy maix.fs.symlink
    */
   err::Err symlink(const std::string &src, const std::string &link, bool force = false);

    /**
     * Check if the path exists
     * @param path path to check
     * @return true if path exists
     * @maixpy maix.fs.exists
    */
    bool exists(const std::string &path);

    /**
     * Create a directory recursively
     * @param path path to create
     * @param exist_ok if true, also return true if directory already exists
     * @param recursive if true, create directory recursively, otherwise, only create one directory, default is true
     * @return err::ERR_NONE(err.Err.ERR_NONE in MaixPy) if success, other error code if failed
     * @maixpy maix.fs.mkdir
    */
    err::Err mkdir(const std::string &path, bool exist_ok = true, bool recursive = true);

    /**
     * Remove a directory
     * @param path path to remove
     * @param recursive if true, remove directory recursively, otherwise, only remove empty directory, default is false
     * @return err::ERR_NONE(err.Err.ERR_NONE in MaixPy) if success, other error code if failed
     * @maixpy maix.fs.rmdir
    */
    err::Err rmdir(const std::string &path, bool recursive = false);

    /**
     * Remove a file
     * @param path path to remove
     * @return err::ERR_NONE(err.Err.ERR_NONE in MaixPy) if success, other error code if failed
     * @maixpy maix.fs.remove
    */
    err::Err remove(const std::string &path);

    /**
     * Rename a file or directory
     * @param src source path
     * @param dst destination path, if destination dirs not exist, will auto create
     * @return err::ERR_NONE(err.Err.ERR_NONE in MaixPy) if success, other error code if failed
     * @maixpy maix.fs.rename
    */
    err::Err rename(const std::string &src, const std::string &dst);

    /**
     * Sync files, ensure they're wrriten to disk from RAM
     * @maixpy maix.fs.sync
    */
    void sync();

    /**
     * Get file size
     * @param path path to get size
     * @return file size if success, -err::Err code if failed
     * @maixpy maix.fs.getsize
    */
    int getsize(const std::string &path);

    /**
     * Get directory name of path
     * @param path path to get dirname
     * @return dirname if success, empty string if failed
     * @maixpy maix.fs.dirname
    */
    std::string dirname(const std::string &path);

    /**
     * Get base name of path
     * @param path path to get basename
     * @return basename if success, empty string if failed
     * @maixpy maix.fs.basename
    */
    std::string basename(const std::string &path);

    /**
     * Get absolute path
     * @param path path to get absolute path
     * @return absolute path if success, empty string if failed
     * @maixpy maix.fs.abspath
    */
    std::string abspath(const std::string &path);

    /**
     * Get current working directory
     * @return current working directory absolute path
     * @maixpy maix.fs.getcwd
     */
    std::string getcwd();

    /**
     * Get realpath of path
     * @param path path to get realpath
     * @return realpath if success, empty string if failed
     * @maixpy maix.fs.realpath
     */
    std::string realpath(const std::string &path);

    /**
     * Get file extension
     * @param path path to get extension
     * @return prefix_path and extension list if success, empty string if failed
     * @maixpy maix.fs.splitext
    */
    std::vector<std::string> splitext(const std::string &path);

    /**
     * List files in directory
     * @param path path to list
     * @param recursive if true, list recursively, otherwise, only list current directory, default is false
     * @param full_path if true, return full path, otherwise, only return basename, default is false
     * @return files list if success, nullptr if failed, you should manually delete it in C++.
     * @maixpy maix.fs.listdir
    */
    std::vector<std::string> *listdir(const std::string &path, bool recursive = false, bool full_path = false);

    /**
     * File read write ops
     * @maixpy maix.fs.File
    */
    class File
    {
    public:
        /**
         * @brief Construct File object
         * @maixpy maix.fs.File.__init__
         * @maixcdk maix.fs.File.File
         */
        File()
        {
            _fp = nullptr;
        }

        ~File()
        {
            close();
        }

        /**
         * Open a file
         * @param path path to open
         * @param mode open mode, support "r", "w", "a", "r+", "w+", "a+", "rb", "wb", "ab", "rb+", "wb+", "ab+"
         * @return err::ERR_NONE(err.Err.ERR_NONE in MaixPy) if success, other error code if failed
         * @maixpy maix.fs.File.open
        */
        err::Err open(const std::string &path, const std::string &mode);

        /**
         * Close a file
         * @maixpy maix.fs.File.close
        */
        void close();

        /**
         * Read data from file
         * @param buf buffer to store data
         * @param size buffer size(max read size)
         * @return read size if success, -err::Err code if failed
         * @maixcdk maix.fs.File.read
        */
        int read(void *buf, int size);

        /**
         * Read data from file API2
         * @param size max read size
         * @return bytes data if success(need delete manually in C/C++), nullptr if failed
         * @maixpy maix.fs.File.read
        */
        std::vector<uint8_t> *read(int size);

        /**
         * Read line from file
         * @param line buffer to store line
         * @return read size if success, -err::Err code if failed
         * @maixcdk maix.fs.File.readline
        */
        int readline(std::string &line);

        /**
         * Read line from file
         * @return line if success, empty string if failed. You need to delete the returned object manually in C/C++.
         * @maixpy maix.fs.File.readline
        */
        std::string *readline();

        /**
         * End of file or not
         * @return 0 if not reach end of file, else eof.
         * @maixpy maix.fs.File.eof
        */
       int eof();

        /**
         * Write data to file
         * @param buf buffer to write
         * @param size buffer size
         * @return write size if success, -err::Err code if failed
         * @maixcdk maix.fs.File.write
        */
        int write(const void *buf, int size);

        /**
         * Write data to file API2
         * @param buf buffer to write
         * @return write size if success, -err::Err code if failed
         * @maixpy maix.fs.File.write
        */
        int write(const std::vector<uint8_t> &buf);

        /**
         * Seek file position
         * @param offset offset to seek
         * @param whence @see maix.fs.SEEK
         * @return new position if success, -err::Err code if failed
         * @maixpy maix.fs.File.seek
        */
        int seek(int offset, int whence);

        /**
         * Get file position
         * @return file position if success, -err::Err code if failed
         * @maixpy maix.fs.File.tell
        */
        int tell();

        /**
         * Flush file
         * @return err::ERR_NONE(err.Err.ERR_NONE in MaixPy) if success, other error code if failed
         * @maixpy maix.fs.File.flush
        */
        err::Err flush();
    private:
        void *_fp;
    };

    /**
     * Open a file, and return a File object
     * @param path path to open
     * @param mode open mode, support "r", "w", "a", "r+", "w+", "a+", "rb", "wb", "ab", "rb+", "wb+", "ab+"
     * @return File object if success(need to delete object manually in C/C++), nullptr if failed
     * @maixpy maix.fs.open
    */
    fs::File *open(const std::string &path, const std::string &mode);

    /**
     * Get temp files directory
     * @return temp files directory
     * @maixpy maix.fs.tempdir
    */
    std::string tempdir();
}
