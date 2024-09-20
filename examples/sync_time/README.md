Sync Time Project based on MaixCDK
====

English
---
Synchronize network time to the system time, and then synchronize the system time to the RTC.

* By default, it looks for the `assets/ntp_config.yaml` configuration file in the current directory. If it cannot be found, it uses the built-in NTP address.

    The content of `ntp_config.yaml` is as follows:

    ```yaml
    Config:
        - retry: 3
        - total_timeout_ms: 10000

    NtpServers:
        - host: "ntp.xxx1.com"
          port: 123
        - host: "ntp1.xxx2.com"
          port: 234
        - host: "ntp2.xxx3.com"
          port: 456
    ```

* Parameters
    ```
    Usage:
    ./sync_time [OPTION...]

    -b, --blocking  Run in blocking mode.
    -d, --debug     Enable debugging
    -h, --help      Print usage

    ```

    Background time synchronization uses `sync_time`, and logs will be output to `sync_time.log` in the same directory as `sync_time`.

    Blocking wait for time synchronization completion uses `sync_time -b` or `sync_time --blocking`, and logs will be output to `stdout`.

    Check the deviation between RTC time and system time using `sync_time -d` or `sync_time --debug`, and logs will be output to `stdout`.
    This operation only synchronizes network time to the system time. The RTC has a minimum precision of seconds, and the current RTC time and system time will be output to `stdout` each time the RTC updates the second time.


中文
---

将网络时间同步到系统时间, 然后将系统时间同步到RTC中.

* 默认查找当前目录下的 `assets/ntp_config.yaml` 配置文件, 如果未能找到则使用内置的 NTP 地址.

    ntp_config.yaml 内容形如:

    ```yaml
    Config:
        - retry: 3
        - total_timeout_ms: 10000

    NtpServers:
        - host: "ntp.xxx1.com"
          port: 123
        - host: "ntp1.xxx2.com"
          port: 234
        - host: "ntp2.xxx3.com"
          port: 456
    ```

* 参数
    ```
    Usage:
    ./sync_time [OPTION...]

    -b, --blocking  Run in blocking mode.
    -d, --debug     Enable debugging
    -h, --help      Print usage

    ```

    后台同步时间使用 `sync_time`, 日志将会输出到 `sync_time` 相同目录下的 `sync_time.log` 中.

    阻塞等待时间同步完成使用 `sync_time -b` 或 `sync_time --blocking`, 日志将会输出到 `stdout` 中.

    检查RTC时间与系统时间的误差使用 `sync_time -d` 或 `sync_time --debug`, 日志将会输出到 `stdout` 中.
    该操作仅同步网络时间到系统时间中, 该 RTC 最小精度为秒, 在每次 RTC 更新秒钟时间时将当前 RTC 时间和系统时间输出到 `stdout`.


This is a project based on MaixCDK, build method please visit [MaixCDK](https://github.com/sipeed/MaixCDK)

