/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include <map>
#include <tuple>
#include "maix_err.hpp"
#include <valarray>
using namespace std;

namespace maix::app
{

    /**
     * APP version
     * @maixpy maix.app.Version
    */
    class Version
    {
    public:
        uint8_t major;
        uint8_t minor;
        uint8_t patch;

        /**
         * Convert to string, e.g. 1.0.0
         * @maixpy maix.app.Version.__str__
        */
        std::string __str__()
        {
            char buff[32];
            snprintf(buff, sizeof(buff), "%d.%d.%d", major, minor, patch);
            return buff;
        }

        /**
         * Convert from string, e.g. "1.0.0"
         * @maixpy maix.app.Version.from_str
        */
        static app::Version from_str(const string &version_str)
        {
            Version version;
            int ret = sscanf(version_str.c_str(), "%hhd.%hhd.%hhd", &version.major, &version.minor, &version.patch);
            if (ret != 3)
            {
                printf("version_str2num failed: %d\n", ret);
                version.major = 0;
                version.minor = 0;
                version.patch = 0;
            }
            return version;
        }
    } ;

    /**
     * APP info
     * @maixpy maix.app.APP_Info
    */
    class APP_Info
    {
    public:
        APP_Info()
            : id(""), name(""), icon(""), version({0, 0, 0}), exec(""), author(""), desc("")
        {
        }

        APP_Info(string id, string name, string icon, Version version, string exec,
                      string author, string desc, map<string, string> names, map<string, string> descs)
            : id(id), name(name), icon(icon), version(version), exec(exec), author(author), desc(desc), names(names), descs(descs)
        {
        }

        APP_Info(string id, string name, string icon, string version, string exec,
                      string author, string desc, map<string, string> names, map<string, string> descs)
            : id(id), name(name), icon(icon), exec(exec), author(author), desc(desc), names(names), descs(descs)
        {
            Version ver = app::Version::from_str(version);
            this->version.major = ver.major;
            this->version.minor = ver.minor;
            this->version.patch = ver.patch;
        }

        ~APP_Info() {}

    public:
        /**
         * APP id
         * @maixpy maix.app.APP_Info.id
        */
        string id;
        /**
         * APP name
         * @maixpy maix.app.APP_Info.name
        */
        string name;
        /**
         * APP icon
         * @maixpy maix.app.APP_Info.icon
        */
        string icon;
        /**
         * APP version
         * @maixpy maix.app.APP_Info.version
        */
        Version version;
        /**
         * APP exec
         * @maixpy maix.app.APP_Info.exec
        */
        string exec;
        /**
         * APP author
         * @maixpy maix.app.APP_Info.author
        */
        string author;
        /**
         * APP desc
         * @maixpy maix.app.APP_Info.desc
        */
        string desc;
        /**
         * APP names
         * @maixpy maix.app.APP_Info.names
        */
        map<string, string> names; // locale: name
        /**
         * APP descs
         * @maixpy maix.app.APP_Info.descs
        */
        map<string, string> descs; // locale: desc
    };

    /**
     * Get current APP ID.
     * @return APP ID.
     * @maixpy maix.app.app_id
    */
    string app_id();

    /**
     * Set current APP ID.
     * @param app_id APP ID.
     * @maixpy maix.app.set_app_id
    */
    string set_app_id(const string &app_id);

    /**
     * Get APP info file path.
     * @maixpy maix.app.get_apps_info_path
    */
    string get_apps_info_path();

    /**
     * Get APP info list.
     * @param ignore_launcher if true, ignore launcher APP. default false.
     * @param ignore_app_store if true, ignore app store APP. default false.
     * @return APP info list. APP_Info object list.
     * @maixpy maix.app.get_apps_info
    */
    vector<app::APP_Info> &get_apps_info(bool ignore_launcher = false, bool ignore_app_store = false);

    /**
     * Get app info by app id.
     * @return app.APP_Info type.
     * @maixpy maix.app.get_app_info
    */
    app::APP_Info get_app_info(const std::string &app_id);

    /**
     * Get APP info, APP can store private data in this directory.
     * @return APP data path "./data", just return the data folder in current path because APP executed in app install path or project path.
     *         So, you must execute your program in you project path to use the project/data folder when you debug your APP.
     * @maixpy maix.app.get_app_data_path
     */
    string get_app_data_path();


    /**
     * Get APPS store path. Please DON'T write this directory manually.
     * This API may deprecated in future.
     * @return APPS store path.
    */
    string get_apps_path();

    /**
     * Get APP path.
     * @param app_id APP ID, if empty, return current APP path, else return the APP path by app_id.
     * @return APP path, just return the current path because APP executed in app install path or project path.
     *         So, you must execute your program in you project path to use the project/data folder when you debug your APP.
     * @maixpy maix.app.get_app_path
     */
    string get_app_path(const string &app_id = "");

    /**
     * Get global temporary data path, APPs can use this path as temporary data directory.
     * @return temporary data path.
     * @maixpy maix.app.get_tmp_path
    */
    string get_tmp_path();

    /**
     * Get data path of share, shared data like picture and video will put in this directory
     * @return share data path.
     * @maixpy maix.app.get_share_path
     */
    string get_share_path();

    /**
     * Get picture path of share, shared picture will put in this directory
     * @return share picture path.
     * @maixpy maix.app.get_picture_path
     */
    string get_picture_path();

    /**
     * Get video path of share, shared video will put in this directory
     * @return share video path.
     * @maixpy maix.app.get_video_path
    */
    string get_video_path();

    /**
     * Get font path of share, shared font will put in this directory
     * @return share font path.
     * @maixpy maix.app.get_font_path
    */
    string get_font_path();

    /**
     * Get icon path of share, shared icon will put in this directory
     * @return share icon path.
     * @maixpy maix.app.get_icon_path
    */
    string get_icon_path();

    /**
     * Get system config item value.
     * @param item name of setting item, e.g. wifi, language. more see settings APP.
     * @param key config key, e.g. for wifi, key can be ssid, for language, key can be locale.
     * @param value default value, if not found, return this value.
     * @param from_cache if true, read from cache, if false, read from file.
     * @return config value, always string type, if not found, return empty string.
     * @maixpy maix.app.get_sys_config_kv
    */
    string get_sys_config_kv(const string &item, const string &key, const string &value = "", bool from_cache = true);

    /**
     * Set system config item value.
     * It's not recommend to use this method in your APP, this method should only called by settings APP.
     * This method may deprecated in future.
    */
    err::Err set_sys_config_kv(const string &item, const string &key, const string &value, bool write_file = true);

    /**
     * Get system config path, ini format.
     * All system config info store in this file.
     * Options detail refer to settings APP.
     * This file is read only for APPs, please don't edit it despite we have no permission control.
     * Please DON'T modify this file directly, only the settings APP can modify it.
     * This method may deprecated in future.
    */
    string get_sys_config_path();

    /**
     * Get APP config item value.
     * @param item name of setting item, e.g. user_info
     * @param key config key, e.g. for user_info, key can be name, age etc.
     * @param value default value, if not found, return this value.
     * @param from_cache if true, read from cache, if false, read from file.
     * @return config value, always string type, if not found, return empty string.
     * @maixpy maix.app.get_app_config_kv
    */
    string get_app_config_kv(const string &item, const string &key, const string &value = "", bool from_cache = true);

    /**
     * Set APP config item value.
     * @param item name of setting item, e.g. user_info
     * @param key config key, e.g. for user_info, key can be name, age etc.
     * @param value config value, always string type.
     * @param write_file if true, write to file, if false, just write to cache.
     * @return err::Err
     * @maixpy maix.app.set_app_config_kv
    */
    err::Err set_app_config_kv(const string &item, const string &key, const string &value, bool write_file = true);

    /**
     * Get APP config path, ini format, so you can use your own ini parser to parse it like `configparser` in Python.
     * All APP config info is recommended to store in this file.
     * @return APP config path(ini format).
     * @maixpy maix.app.get_app_config_path
    */
    string get_app_config_path();

    /**
     * Set APP exit code and exit message.
     * If code != 0, the launcher will show a dialog to user, and display the msg.
     * @param code exit code, 0 means success, other means error, if code is 0, do nothing.
     * @param msg exit message, if code is 0, msg is not used.
     * @return exit code, the same as arg @code.
     * @maixpy maix.app.set_exit_msg
    */
    err::Err set_exit_msg(err::Err code, const string &msg);


    /**
     * Get APP exit code and exit message.
     * @param cache if true, read from cache, if false, read from file. default false.
     * @return exit return app_id, exit code and exit message.
     * @maixpy maix.app.get_exit_msg
    */
    tuple<string, err::Err, string> get_exit_msg(bool cache = false);

    /**
     * Check if have exit msg
     * @param cache if true, just check from cache, if false, check from file. default false.
     * @return true if have exit msg, false if not.
     * @maixpy maix.app.have_exit_msg
    */
    bool have_exit_msg(bool cache = false);

    /**
     * Exit this APP and start another APP(by launcher).
     * Call this API will call set_exit_flag(true), you should check app::need_exit() in your code.
     * And exit this APP if app::need_exit() return true.
     * @param app_id APP ID which will be started. app_id and idx must have one is valid.
     * @param idx APP index. app_id and idx must have one is valid.
     * @param start_param string type, will send to app, app can get this param by `app.get_start_param()`
     * @attention If app id or idx the same as current app, do nothing.
     * @maixpy maix.app.switch_app
    */
    void switch_app(const string &app_id, int idx = -1, const std::string &start_param = "");

    /**
     * Get start param set by caller
     * @return param, string type
     * @maixpy maix.app.get_start_param
    */
    const std::string get_start_param();

    /**
     * Shoule this APP exit?
     * @return true if this APP should exit, false if not.
     * @attention This API is a function, not a variable.
     * @maixpy maix.app.need_exit
    */
    bool need_exit();

    /**
     * App should running? The same as !app::need_exit() (not app::need_exit() in MaixPy).
     * @return true if this APP should running, false if not.
     * @attention This API is a function, not a variable.
     * @maixpy maix.app.running
    */
    bool running();

    /**
     * Set exit flag. You can get exit flag by app.need_exit().
     * @param exit true if this APP should exit, false if not.
     * @maixpy maix.app.set_exit_flag
    */
    void set_exit_flag(bool exit);

} // namespace maix::app
