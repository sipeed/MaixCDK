#pragma once

#ifndef UVC_SERVER_HPP
#define UVC_SERVER_HPP

#include <bits/stdc++.h>
#include <functional>




#include <stdint.h>

typedef int uvc_fill_mjpg_cb_t(void *buf, uint32_t *size);

class UvcServer {
public:
    // 获取唯一实例
    static UvcServer* getInstance() {
        std::call_once(_onceFlag, []() {
            _instance = new UvcServer();
        });
        return _instance;
    }

    // 设置回调函数，可以是普通函数，也可以是 lambda 或其他可调用对象
    void set_cb(std::function<uvc_fill_mjpg_cb_t> cb) {
        this->_cb = cb;
    }

    void run();
    void stop();

    // 提供给 C 的回调接口
    static int c_callback(void *buf, uint32_t *size) {
        if (_instance && _instance->_cb) {
            return _instance->_cb(buf, size);  // 调用注册的 C++ 回调
        }
        return -1;  // 如果没有设置回调，返回错误码
    }

private:
    UvcServer() {}  // 构造函数私有化
    ~UvcServer() { this->stop(); } // 析构函数私有化

    // 禁止拷贝构造和赋值
    UvcServer(const UvcServer&) = delete;
    UvcServer& operator=(const UvcServer&) = delete;

    static UvcServer *_instance;  // 使用 _instance 管理实例
    static std::once_flag _onceFlag;  // 确保单例只初始化一次
    
    std::unique_ptr<std::thread> uvc_daemon_thrd;
    std::function<uvc_fill_mjpg_cb_t> _cb;  // 存储回调函数
};

#endif