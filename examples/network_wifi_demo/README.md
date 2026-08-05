# WiFi command-line demo

This example exercises the public `maix::network::wifi::Wifi` API for scanning,
status inspection, STA mode, and AP mode. It never prints STA or AP passwords.

## Build for MaixCAM2

From this example directory, run:

```sh
maixcdk build -p maixcam2 --toolchain-id default
```

The executable is generated at `build/network_wifi_demo`. The release directory
under `dist/` contains the files to copy to a MaixCAM2. See the main MaixCDK
documentation for deployment and package-installation options.

After copying the release directory to the device, enter that directory and run:

```sh
./network_wifi_demo --help
```

The MaixCAM2 image must also contain the three coordinated rootfs updates from
`.trellis/tasks/08-04-maixcam2-ap-mode/artifacts/system-files/`: `wifi.sh`,
`wifi.service`, and `udhcpc.service`. Follow that artifact README's install,
`daemon-reload`, validation, and rollback instructions before device testing.
The unit update is what makes both `sta-connect` and AP-to-STA `ap-stop` start
the STA DHCP client; it also preserves DHCP for the image's no-marker default
STA mode.

## Commands

```text
./network_wifi_demo scan
./network_wifi_demo status
./network_wifi_demo sta-connect <ssid> <password> [wait] [timeout]
./network_wifi_demo sta-disconnect
./network_wifi_demo ap-start <ssid> <password> [mode] [channel] [ip] [netmask] [hidden]
./network_wifi_demo ap-stop
```

On MaixCAM2, `ap-stop` transitions from AP mode to the default STA mode and
starts `wifi.service`, making the WiFi device available again. It does not
modify credential files during the stop transition; use `sta-connect` to select
the STA network and credentials.

If `wlan0` is not discoverable when `sta-connect` or `ap-start` is requested on
MaixCAM2, the demo still calls the public API with the known `wlan0` interface.
`Wifi.connect()` and `Wifi.start_ap()` then start `wifi.service` once and wait up
to about five seconds for the interface before continuing. A service-start
failure or interface timeout is reported as an API error and produces a nonzero
exit code. Parsing and AP argument validation happen before this recovery, so
invalid input does not start the service. `scan`, `status`, `sta-disconnect`, and
`ap-stop` retain discovery-driven behavior and never start the service merely
because no interface was found. The demo itself does not call `systemctl`.
This recovery covers external service control, a driver fault, or system startup
ordering. It is not part of normal disconnect: on MaixCAM2, `sta-disconnect`
disconnects only the current association while keeping `wifi.service`, the
driver, and `wlan0` alive. A subsequent `scan` therefore works without a service
recovery step. On MaixCAM2, `is_connected()` reads the supplicant state rather
than stale route data, so it becomes false after this disconnect. `ap-stop` also
leaves the interface discoverable because it starts STA mode itself.

The optional arguments use the same defaults as the public APIs:

| Command | Argument | Default |
| --- | --- | --- |
| `sta-connect` | `wait` | `true` |
| `sta-connect` | `timeout` | `60` seconds |
| `ap-start` | `mode` | `g` |
| `ap-start` | `channel` | `0` |
| `ap-start` | `ip` | `192.168.66.1` |
| `ap-start` | `netmask` | `255.255.255.0` |
| `ap-start` | `hidden` | `false` |

Boolean arguments accept `true`, `false`, `1`, or `0`. Optional arguments are
positional, so include the preceding defaults when setting a later option.
The STA connection timeout must be a non-negative decimal integer.
An empty password (`''`) selects an open STA network or open AP.

## Device examples

Replace the uppercase placeholders below with test values. The example does not
contain real network credentials:

```sh
./network_wifi_demo scan
./network_wifi_demo status

./network_wifi_demo sta-connect YOUR_STA_SSID 'YOUR_STA_PASSWORD'
./network_wifi_demo status
./network_wifi_demo sta-disconnect
./network_wifi_demo scan

./network_wifi_demo ap-start DEMO_AP 'YOUR_AP_PASSWORD' g 6
./network_wifi_demo status
./network_wifi_demo ap-stop
```

To test an open, hidden AP while explicitly showing every positional default:

```sh
./network_wifi_demo ap-start OPEN_DEMO_AP '' g 0 192.168.66.1 255.255.255.0 true
./network_wifi_demo ap-stop
```

After `ap-start`, verify from another client that the SSID visibility, password,
channel, and assigned `192.168.66.0/24` address match the selected arguments.
Then use `ap-stop` to start STA mode again on MaixCAM2.

Passwords passed on a command line may be retained by the shell history or be
temporarily visible to local process-inspection tools. Use dedicated test
credentials on development devices.
