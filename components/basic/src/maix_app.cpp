/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#include "maix_app.hpp"
#include "maix_fs.hpp"
#include "maix_log.hpp"
#include "stdio.h"
#include "global_config.h"

#include "math.h"
#include "inifile.h"
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <assert.h>

#define APP_ROOT_PATH "/maixapp"
namespace maix::app
{
    // cache system config info
    static inifile::IniFile sys_conf_ini;
    static inifile::IniFile app_conf_ini;
    static bool sys_conf_ini_loaded = false;
    static bool app_conf_ini_loaded = false;
    static err::Err exit_code = err::ERR_NONE;
    static std::string exit_msg = "";
    static bool should_exit = false;
    static std::string _app_id = PROJECT_ID;
    static bool _app_id_searched = false;

    static string _read_id(const string &app_yaml)
    {
        fs::File *fp = fs::open(app_yaml, "r");
        string line;
        string id;
        while (1)
        {
            int read = fp->readline(line);
            if (read <= 0)
            {
                break;
            }
            if (line.find("id:") == 0)
            {
                id = line.substr(3);
                break;
            }
        }
        fp->close();
        delete fp;
        // strip id
        id.erase(0, id.find_first_not_of(" "));
        id.erase(id.find_last_not_of(" ") + 1);
        if (id.empty())
        {
            log::error("read app id failed\n");
            throw err::Exception(err::ERR_ARGS, "read app id from app.yaml failed");
        }
        return id;
    }

    string app_id()
    {
        if ((_app_id.empty() || _app_id == "maixpy") && !_app_id_searched)
        {
            // if have app.yaml, read id from app.yaml
            if (fs::exists("app.yaml"))
            {
                _app_id = _read_id("app.yaml");
            }
            _app_id_searched = true;
        }
        return _app_id;
    }

    string set_app_id(const string &app_id)
    {
        _app_id = app_id;
        return _app_id;
    }

    string get_apps_info_path()
    {
        return APP_ROOT_PATH "/apps/app.info";
    }

    /**
     *
     * app.info file, ini format
     * [basic]
     * version = 1 # integer
     *
     * [app_id]
     * name = app_name
     * name[locale] = app_name_translation
     * icon = app_icon relative path
     * exec = app_exec relative path
     * author = app_author
     * desc = app_desc
     * desc[locale] = app_desc_translation
     *
     * [app_id2]
     * ...
     */
    vector<APP_Info> &get_apps_info(bool ignore_launcher, bool ignore_app_store)
    {
        int ret = 0;
        int app_num = 0;
        int ini_version = 0;
        vector<string> app_ids;

        // create a vector of APP_Info
        static vector<APP_Info> apps_info;
        // clear the vector
        apps_info.clear();

        // read apps info from APP_ROOT_PATH/apps/app.info
        inifile::IniFile ini;
        ret = ini.Load(get_apps_info_path());
        if (ret != RET_OK)
        {
            printf("open app info failed: %d\n", ret);
            return apps_info;
        }
        ret = ini.GetIntValue("basic", "version", &ini_version);
        if (ret != RET_OK)
        {
            printf("get app info version failed: %d\n", ret);
            return apps_info;
        }
        // delete basic section, only app info left
        ini.DeleteSection("basic");
        // get app info
        app_num = ini.GetSections(&app_ids);
        for (int i = 0; i < app_num; ++i)
        {
            // get app info from section key value, including:
            // name, icon, version, exec, author, desc
            string app_id = app_ids[i];
            string app_name;
            string app_icon;
            string app_exec;
            string app_version;
            string app_author;
            string app_desc;
            if (app_id == "") // skip empty section
            {
                continue;
            }
            if (ignore_launcher && app_id == "launcher")
            {
                continue;
            }
            if (ignore_app_store && app_id == "app_store")
            {
                continue;
            }
            // print app_id
            ret = ini.GetStringValue(app_id, "name", &app_name);
            if (ret != RET_OK)
            {
                printf("get app %s name failed: %d\n", app_id.c_str(), ret);
                continue;
            }
            ret = ini.GetStringValue(app_id, "icon", &app_icon);
            if (ret != RET_OK)
            {
                printf("get app %s icon failed: %d\n", app_id.c_str(), ret);
                continue;
            }
            if (fs::isabs(app_icon) == false)
            {
                app_icon = get_app_path(app_id) + "/" + app_icon;
            }
            // check icon file exists, not exists, use default icon
            if (fs::exists(app_icon) == false)
            {
                app_icon = "/maixapp/share/icon/icon.json";
            }
            ret = ini.GetStringValue(app_id, "exec", &app_exec);
            if (ret != RET_OK)
            {
                printf("get app %s exec failed: %d\n", app_id.c_str(), ret);
                continue;
            }
            ret = ini.GetStringValue(app_id, "version", &app_version);
            if (ret != RET_OK)
            {
                printf("get app %s version failed: %d\n", app_id.c_str(), ret);
                continue;
            }
            // convert string version to int values
            ini.GetStringValue(app_id, "author", &app_author);
            ini.GetStringValue(app_id, "desc", &app_desc);
            // parse all translation
            map<string, string> names;
            map<string, string> descs;
            inifile::IniSection *kvs = ini.getSection(app_id);
            for (auto it = kvs->begin(); it != kvs->end(); ++it)
            {
                string key = it->key;
                string value = it->value;
                if (key.find("name[") == 0)
                {
                    string locale = key.substr(5, key.length() - 6);
                    names[locale] = value;
                }
                else if (key.find("desc[") == 0)
                {
                    string locale = key.substr(5, key.length() - 6);
                    descs[locale] = value;
                }
            }
            // create a APP_Info object
            APP_Info app_info(app_id, app_name, app_icon, app_version, app_exec, app_author, app_desc, names, descs);
            apps_info.push_back(app_info);
        }
        return apps_info;
    }

    app::APP_Info get_app_info(const std::string &app_id)
    {
        vector<APP_Info> info = get_apps_info();
        for(auto i : info)
        {
            if (i.id == app_id)
            {
                return i;
            }
        }
        throw err::Exception(err::ERR_ARGS, "app_id not found");
    }

    string get_app_data_path()
    {
        string path = "./data";
        if (fs::mkdir(path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", path.c_str());
        }
        return path;
    }

    string get_apps_path()
    {
        return string(APP_ROOT_PATH "/apps");
    }

    string get_app_path(const string &app_id)
    {
        if (app_id.empty())
            return ".";
        string path = string(APP_ROOT_PATH "/apps/" + app_id);
        return path;
    }

    string get_tmp_path()
    {
        string tmp_path = string(APP_ROOT_PATH "/tmp");
        if (fs::mkdir(tmp_path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", tmp_path.c_str());
        }
        return tmp_path;
    }

    /**
     * Get data path of share, shared data like picture and video will put in this directory
     */
    string get_share_path()
    {
        string path = string(APP_ROOT_PATH "/share");
        if (fs::mkdir(path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", path.c_str());
        }
        return path;
    }

    /**
     * Get picture path of share, shared picture will put in this directory
     */
    string get_picture_path()
    {
        string path = string(APP_ROOT_PATH "/share/picture");
        if (fs::mkdir(path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", path.c_str());
        }
        return path;
    }

    /**
     * Get video path of share, shared video will put in this directory
     */
    string get_video_path()
    {
        string path = string(APP_ROOT_PATH "/share/video");
        if (fs::mkdir(path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", path.c_str());
        }
        return path;
    }

    /**
     * Get font path of share, shared font will put in this directory
     */
    string get_font_path()
    {
        string path = string(APP_ROOT_PATH "/share/font");
        if (fs::mkdir(path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", path.c_str());
        }
        return path;
    }

    /**
     * Get icon path of share, shared icon will put in this directory
     */
    string get_icon_path()
    {
        string path = string(APP_ROOT_PATH "/share/icon");
        if (fs::mkdir(path, true, true) != err::ERR_NONE)
        {
            log::error("mkdir %s failed\n", path.c_str());
        }
        return path;
    }

    /**
     * Get system config path, ini format
     */
    string get_sys_config_path()
    {
        string path = string(APP_ROOT_PATH "/sys_conf.ini");
        return path;
    }

    static inline int create_sys_conf_load()
    {
        string path = get_sys_config_path();
        if (!fs::exists(path))
            sys_conf_ini.SaveAs(path);
        return sys_conf_ini.Load(path);
    }

    string get_sys_config_kv(const string &item, const string &key, const string &value, bool from_cache)
    {
        string _value = value;
        if (from_cache && sys_conf_ini_loaded)
        {
            // read from cache
            sys_conf_ini.GetStringValue(item, key, &_value);
        }
        else
        {
            // read from file
            int ret = create_sys_conf_load();
            if (ret != RET_OK)
            {
                log::error("open sys config failed: %d\n", ret);
                return value;
            }
            sys_conf_ini.GetStringValue(item, key, &_value);
            sys_conf_ini_loaded = true;
        }
        return _value;
    }

    err::Err set_sys_config_kv(const string &item, const string &key, const string &value, bool write_file)
    {
        if (!sys_conf_ini_loaded)
        {
            int ret = create_sys_conf_load();
            if (ret != RET_OK)
            {
                log::error("open sys config failed: %d\n", ret);
                return err::ERR_RUNTIME;
            }
            sys_conf_ini_loaded = true;
        }
        int ret = sys_conf_ini.SetStringValue(item, key, value);
        if (ret != RET_OK)
        {
            log::error("set sys config failed: %d\n", ret);
            return err::ERR_RUNTIME;
        }
        if (write_file)
        {
            ret = sys_conf_ini.Save();
            if (ret != RET_OK)
            {
                log::error("save sys config failed: %d\n", ret);
                return err::ERR_RUNTIME;
            }
        }
        return err::ERR_NONE;
    }

    string get_app_config_kv(const string &item, const string &key, const string &value, bool from_cache)
    {
        string _value = value;
        if (from_cache && app_conf_ini_loaded)
        {
            // read from cache
            app_conf_ini.GetStringValue(item, key, &_value);
        }
        else
        {
            // read from file
            string path = get_app_config_path();
            if (!fs::exists(path))
                app_conf_ini.SaveAs(path);
            int ret = app_conf_ini.Load(path);
            if (ret != RET_OK)
            {
                log::error("open app config failed: %d\n", ret);
                return value;
            }
            app_conf_ini.GetStringValue(item, key, &_value);
            app_conf_ini_loaded = true;
        }
        return _value;
    }

    err::Err set_app_config_kv(const string &item, const string &key, const string &value, bool write_file)
    {
        if (!app_conf_ini_loaded)
        {
            string path = get_app_config_path();
            if (!fs::exists(path))
                app_conf_ini.SaveAs(path);
            int ret = app_conf_ini.Load(path);
            if (ret != RET_OK)
            {
                log::error("open app config failed: %d\n", ret);
                return err::ERR_RUNTIME;
            }
            app_conf_ini_loaded = true;
        }
        int ret = app_conf_ini.SetStringValue(item, key, value);
        if (ret != RET_OK)
        {
            log::error("set app config failed: %d\n", ret);
            return err::ERR_RUNTIME;
        }
        if (write_file)
        {
            ret = app_conf_ini.Save();
            if (ret != RET_OK)
            {
                log::error("save app config failed: %d\n", ret);
                return err::ERR_RUNTIME;
            }
        }
        return err::ERR_NONE;
    }

    string get_app_config_path()
    {
        string path = get_app_data_path() + "/config.ini";
        return path;
    }

    static string get_exit_msg_path()
    {
        string path = string(APP_ROOT_PATH "/tmp/app_exit_msg.txt");
        return path;
    }

    err::Err set_exit_msg(err::Err code, const string &msg)
    {
        if (code == err::ERR_NONE)
        {
            return err::ERR_NONE;
        }
        // content: app_id\ncode\nmsg\n
        string path = get_exit_msg_path();
        FILE *fp = fopen(path.c_str(), "w");
        if (fp == NULL)
        {
            log::error("open exit msg file failed: %s\n", path.c_str());
            return code;
        }
        exit_code = code;
        exit_msg = msg;
        fprintf(fp, "%s\n%d\n%s\n", app_id().c_str(), code, msg.c_str());
        fclose(fp);
        return code;
    }

    tuple<string, err::Err, string> get_exit_msg(bool cache)
    {
        if (cache)
        {
            return make_tuple(app_id(), exit_code, exit_msg);
        }
        if (fs::exists(get_exit_msg_path()) == false)
        {
            return make_tuple("", err::ERR_NONE, "");
        }
        // content: app_id\ncode\nmsg\n
        string path = get_exit_msg_path();
        FILE *fp = fopen(path.c_str(), "r");
        if (fp == NULL)
        {
            log::error("open exit msg file failed: %s\n", path.c_str());
            return make_tuple("", err::ERR_NONE, "");
        }
        char app_id[256];
        int code;
        char msg[256];
        (void)!fscanf(fp, "%s\n%d\n", app_id, &code);
        (void)!fgets(msg, sizeof(msg), fp);
        fclose(fp);
        if (code == 0)
        { // remove exit msg file
            fs::remove(path.c_str());
        }
        return make_tuple(app_id, (err::Err)code, msg);
    }

    bool have_exit_msg(bool cache)
    {
        if (cache)
            return exit_code != err::ERR_NONE || exit_msg != "";
        string path = get_exit_msg_path();
        FILE *fp = fopen(path.c_str(), "r");
        if (fp == NULL)
        {
            return false;
        }
        char app_id[256];
        int code;
        char msg[256] = {0};
        (void)!fscanf(fp, "%s\n%d\n%s\n", app_id, &code, msg);
        fclose(fp);
        return code != err::ERR_NONE || msg[0] == 0;
    }

    bool need_exit()
    {
        return should_exit;
    }

    bool running()
    {
        return !should_exit;
    }

    void set_exit_flag(bool exit)
    {
        should_exit = exit;
    }

    static string get_app_start_info_path()
    {
        string path = string("/tmp/run_app.txt");
        return path;
    }

    /**
     * 要启动程序，退出时写入 `/tmp/run_app.txt`
     * 第一行： app 可执行文件路径，如果是 python 文件，则 `/xx/xx/main.py`
     * 第二行： app_id
     * 第三行： 传给 app 的参数，字符串类型
     * 程序可以调用 `maix.app.switch_app()` 函数切换应用，以上行为封装在里面了。
     * 被启动的程序可以通过`maix.app.get_start_param()`函数获得传参。
     */
    void switch_app(const string &app_id, int idx, const std::string &start_param)
    {
        if (idx < 0 && app_id == "")
        {
            log::error("switch app failed, app_id and idx must have one is valid\n");
            return;
        }
        vector<APP_Info> &apps_info = get_apps_info();
        std::string final_app_id = app_id;
        std::string final_app_path = "";
        if (idx >= 0)
        {
            if ((size_t)idx >= apps_info.size())
            {
                log::error("idx error, should < %lld, but %d", apps_info.size(), idx);
                throw err::Exception(err::ERR_ARGS, "idx error");
            }
            final_app_id = apps_info[idx].id;
            final_app_path = get_app_path(final_app_id) + "/" + apps_info[idx].exec;
        }
        else
        {
            final_app_id = app_id;
            for (auto i : apps_info)
            {
                if (final_app_id == i.id)
                {
                    final_app_path = get_app_path(final_app_id) + "/" + i.exec;
                    break;
                }
            }
        }
        // if switch to current app, just return.
        if(final_app_id == app::app_id())
        {
            return;
        }

        // inform this app to exit, code should check this flag by app::need_exit()
        set_exit_flag(true);

        // write app_id to file, launcher will read this file and start the app
        string path = get_app_start_info_path();
        FILE *fp = fopen(path.c_str(), "w");
        if (fp == NULL)
        {
            log::error("open app start info file failed: %s", path.c_str());
            return;
        }
        fprintf(fp, "%s\n%s\n%s\n", final_app_path.c_str(), final_app_id.c_str(), start_param.c_str());
        fclose(fp);
        // when this app exit, the launcher will check this file and start the app
    }

    const std::string get_start_param()
    {
        const char *env_p = std::getenv("APP_START_PARAM");
        if (env_p != nullptr)
        {
            return std::string(env_p);
        }
        else
        {
            return std::string(); // Return an empty string if the environment variable is not found
        }
    }

} // namespace maix::app
