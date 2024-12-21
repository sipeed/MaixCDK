/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2023.9.8: Add framework, create this file.
 */

#pragma once

#include <string>
#include <vector>
#include <map>

namespace maix::sys
{
    /**
     * Get system version
     * @return version string, e.g. "maixcam-2024-08-13-maixpy-v4.4.20"
     * @maixpy maix.sys.os_version
     */
    std::string os_version();

    /**
     * Get MaixPy version, if get failed will return empty string.
     * @return version  string, e.g. "4.4.21"
     * @maixpy maix.sys.maixpy_version
     */
    std::string maixpy_version();


    /**
     * Get runtime version
     * @return current runtime version
     * @maixpy maix.sys.runtime_version
     */
    std::string runtime_version();

    /**
     * Get device configs, we also say board configs. e.g. for MaixCAM it read form /boot/board
     * @param cache read config from cache(if exists, or will call device_configs first internally) if true,
     * if false, always read fron config file.
     * @return device config,json format
     * @throw If board config file error will throw out exception(err.Exception)
     * @maixpy maix.sys.device_configs
     */
    std::map<std::string, std::string> device_configs(bool cache = true);

    /**
     * Get device id
     * @param cache read id from cache(if exists, or will call device_configs first internally) if true,
     * if false, always read fron config file.
     * @return device id, e.g. "maixcam" "maixcam_pro"
     * @maixpy maix.sys.device_id
     */
    std::string device_id(bool cache = true);

    /**
     * Get device name
     * @param cache read id from cache(if exists, or will call device_configs first internally) if true,
     * if false, always read fron config file.
     * @return device name, e.g. "MaixCAM" "MaixCAM-Pro"
     * @maixpy maix.sys.device_name
     */
    std::string device_name(bool cache = true);

    /**
     * Get host name
     * @return host name, e.g. "maixcam-2f9f"
     * @maixpy maix.sys.host_name
     */
    std::string host_name();

    /**
     * Get host domain
     * @return host domain, e.g. "maixcam-2f9f.local"
     * @maixpy maix.sys.host_domain
     */
    std::string host_domain();

    /**
     * Get ip address
     * @return ip address, dict type, e.g. {"eth0": "192.168.0.195", "wlan0": "192.168.0.123", "usb0": "10.47.159.1"}
     * @maixpy maix.sys.ip_address
     */
    std::map<std::string, std::string> ip_address();

    /**
     * Get mac address
     * @return mac address, dict type, e.g. {"eth0": "00:0c:29:2f:9f:00", "wlan0": "00:0c:29:2f:9f:01", "usb0": "00:0c:29:2f:9f:02"}
     * @maixpy maix.sys.mac_address
     */
    std::map<std::string, std::string> mac_address();

    /**
     * Get device key, can be unique id of device
     * @return device key, 32 bytes hex string, e.g. "1234567890abcdef1234567890abcdef"
     * @maixpy maix.sys.device_key
     */
    std::string device_key();

    /**
     * Get memory info
     * @return memory info, dict type, e.g. {"total": 1024, "used": 512, "hw_total": 256*1024*1024}
     *          total: total memory size in Byte.
     *           used: used memory size in Byte.
     *       hw_total: total memory size in Byte of hardware, the total <= hw_total，
     *                 OS kernel may reserve some memory for some hardware like camera, npu, display etc.
     * @maixpy maix.sys.memory_info
     */
    std::map<std::string, int> memory_info();

    /**
     * Bytes to human readable string
     * @param bytes: bytes size，e.g. 1234B = 1234/1024 = 1.205 KB
     * @param precision: decimal precision, default 2
     * @param base: base number, default 1024
     * @param unit: unit string, e.g. "B"
     * @param sep: separator string, e.g. " "
     * @return human readable string, e.g. "1.21 KB"
     * @maixpy maix.sys.bytes_to_human
     */
    std::string bytes_to_human(unsigned long long bytes, int precision = 2, int base = 1024, const std::string &unit = "B", const std::string &sep = " ");

    /**
     * Get CPU frequency
     * @return CPU frequency, dict type, e.g. {"cpu0": 1000000000, "cpu1": 1000000000}
     * @maixpy maix.sys.cpu_freq
     */
    std::map<std::string, unsigned long> cpu_freq();

    /**
     * Get CPU temperature
     * @return CPU temperature, unit dgree, dict type, e.g. {"cpu": 50.0, "cpu0": 50, "cpu1": 50}
     * @maixpy maix.sys.cpu_temp
     */
    std::map<std::string, float> cpu_temp();

    /**
     * Get CPU usage
     * @return CPU usage, dict type, e.g. {"cpu": 50.0, "cpu0": 50, "cpu1": 50}
     * @maixpy maix.sys.cpu_usage
     */
    std::map<std::string, float> cpu_usage();

    /**
     * Get NPU frequency
     * @return NPU frequency, dict type, e.g. {"npu0": 500000000}
     * @maixpy maix.sys.npu_freq
     */
    std::map<std::string, unsigned long> npu_freq();

    /**
     * Get disk usage
     * @param path: disk path, default "/"
     * @return disk usage, dict type, e.g. {"total": 1024, "used": 512}
     * @maixpy maix.sys.disk_usage
     */
    std::map<std::string, unsigned long long> disk_usage(const std::string &path = "/");

    /**
     * Get disk partition and mount point info
     * @param only_disk only return real disk, tempfs sysfs etc. not return, default true.
     * @return disk partition and mount point info, list type, e.g. [{"device": "/dev/mmcblk0p1", "mountpoint": "/mnt/sdcard", "fstype": "vfat"}]
     * @maixpy maix.sys.disk_partitions
     */
    std::vector<std::map<std::string, std::string>> disk_partitions(bool only_disk = true);

    /**
     * register default signal handle
     * @maixpy maix.sys.register_default_signal_handle
    */
    void register_default_signal_handle();

    /**
     * Power off device
     * @maixpy maix.sys.poweroff
    */
    void poweroff();

    /**
     * Power off device and power on
     * @maixpy maix.sys.reboot
    */
    void reboot();

} // namespace maix::sys

