---
title: Maix 串口协议内部实现规范
---

## 串口协议内部实现规范

Maix Comm Protocol 内部根据系统设置的通信方式（比如 uart），构建一个 UART 对象使用，并且调用 uart 的 register_comm_callback(obj, callback)（obj是一个UART对象指针）,
用户需要使用串口时构建UART对象构造函数会先检查是否已经注册过（比如判断已经注册过的obj使用的UART编号和即将初始化的UART编号是否相同），
如果没有注册过就普通构造，已经注册过那就先回调设置的callback函数,callback 负责释放obj（注意 uart 里面 callback后UART不要再使用register_comm_callback传进来的obj了），然后再构造。



