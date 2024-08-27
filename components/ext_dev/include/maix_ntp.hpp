/**
 * @author iawak9lkm
 * @copyright Sipeed Ltd 2024-
 * @license Apache 2.0
 * @update 2024.8.23: Add framework, create this file.
 */

#pragma once
#include "maix_basic.hpp"
#include <vector>

namespace maix::ext_dev::ntp {

/**
 * @brief Retrieves time from an NTP server
 *
 * This function fetches the current time from the specified NTP server and port,
 * returning a tuple containing the time details.
 *
 * @param host The hostname or IP address of the NTP server.
 * @param port The port number of the NTP server. Use -1 for the default port 123.
 * @param retry The number of retry attempts. Must be at least 1.
 * @param timeout_ms The timeout duration in milliseconds. Must be non-negative.
 * @return A list of 6 elements: [year, month, day, hour, minute, second]
 *
 * @maixpy maix.ext_dev.ntp.time
 */

std::vector<int> time(std::string host, int port=-1, uint8_t retry=3, int timeout_ms=0);

/**
 * @brief Retrieves time from an NTP server using a configuration file
 *
 * This function reads the configuration from a YAML file to fetch the current time
 * from a list of specified NTP servers, returning a tuple containing the time details.
 *
 * @param path The path to the YAML configuration file, which should include:
 *  - Config:
 *      - retry: Number of retry attempts (must be at least 1)
 *      - total_timeout_ms: Total timeout duration in milliseconds (must be non-negative)
 *  - NtpServers:
 *      - host: Hostname or IP address of the NTP server
 *      - port: Port number of the NTP server (use 123 for default)
 *
 * Example YAML configuration:
 *  Config:
 *   - retry: 3
 *   - total_timeout_ms: 10000
 *
 *  NtpServers:
 *   - host: "pool.ntp.org"
 *     port: 123
 *   - host: "time.nist.gov"
 *     port: 123
 *   - host: "time.windows.com"
 *     port: 123
 *
 * @return A list of 6 elements: [year, month, day, hour, minute, second]
 *
 * @maixpy maix.ext_dev.ntp.time_with_config
 */
std::vector<int> time_with_config(std::string path);

/**
 * @brief Retrieves time from an NTP server and synchronizes the system time
 *
 * This function fetches the current time from the specified NTP server and port,
 * then synchronizes the system time with the retrieved time.
 *
 * @param host The hostname or IP address of the NTP server.
 * @param port The port number of the NTP server. Use 123 for the default port.
 * @param retry The number of retry attempts. Must be at least 1.
 * @param timeout_ms The timeout duration in milliseconds. Must be non-negative.
 * @return A list of 6 elements: [year, month, day, hour, minute, second]
 *
 * @maixpy maix.ext_dev.ntp.sync_sys_time
 */
std::vector<int> sync_sys_time(std::string host, int port=-1, uint8_t retry=3, int timeout_ms=0);

/**
 * @brief Retrieves time from an NTP server using a configuration file and synchronizes the system time
 *
 * This function reads the configuration from a YAML file to fetch the current time
 * from a list of specified NTP servers, then synchronizes the system time with the retrieved time.
 *
 * @param path The path to the YAML configuration file, which should include:
 *  - Config:
 *      - retry: Number of retry attempts (must be at least 1)
 *      - total_timeout_ms: Total timeout duration in milliseconds (must be non-negative)
 *  - NtpServers:
 *      - host: Hostname or IP address of the NTP server
 *      - port: Port number of the NTP server (use 123 for default)
 *
 * Example YAML configuration:
 *  Config:
 *   - retry: 3
 *   - total_timeout_ms: 10000
 *
 *  NtpServers:
 *   - host: "pool.ntp.org"
 *     port: 123
 *   - host: "time.nist.gov"
 *     port: 123
 *   - host: "time.windows.com"
 *     port: 123
 *
 * @return A vector of integers containing the time details: [year, month, day, hour, minute, second]
 *
 * @maixpy maix.ext_dev.ntp.sync_sys_time_with_config
 */
std::vector<int> sync_sys_time_with_config(std::string path);


}