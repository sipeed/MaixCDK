---

title: Maix Communicate Protocol
update:
- date: 2023-11-28
  author: neucrack
  version: 1.0.0
  content: Designed and implemented the protocol documentation and code API

---

> This article is translated from Chinese by ChatGPT, may have error, Pull Request is welcome!


## Introduction to Maix Serial Protocol

Devices running MaixCDK or MaixPy can be controlled not only directly via the device's touch screen and buttons but also as modules using `UART` (serial port) or `I2C`.

> Since I2C operates in a master-slave mode, the protocol adopts a request-response format: `Request` + `Response`.  
> For UART protocol, some functions support active reporting. Please refer to the specific protocol details.

The communication protocol is explained in detail below.

## How to Use Maix Device

Upon startup, go to the `Settings` application, and select the communication interface under `Communication` settings:

* **Serial Port**: Uses the serial port for communication. The specific serial port depends on the board, with a default baud rate of `115200`.
* **TCP**: Starts a TCP service on the Maix device, allowing the master device to connect via TCP. The default port is `5555`.

Once the master device is connected to the Maix device, it can communicate following the protocol, where the master device acts as the primary device, and the Maix device as the slave.

**Development on Maix Device Side**:

You can easily implement command responses using the provided `maix.comm.CommProtocol` class. Refer to the examples [comm_protocol](../../../examples/protocol_demo) (C++) or [comm_protocol.py](https://github.com/sipeed/MaixPy/blob/main/examples/protocol) (MaixPy) for details.

**Development on Master Device Side**:

The master device can implement the protocol based on the chip and programming language. Template code is provided in the appendix.

---

## Data Frame

|               | header (4B LE) | data len (4B LE)(flags+cmd+body+crc) | flags (1B) | cmd (1B) | body (nB) | CRC16_IBM (2B LE) (all previous) |
| ------------- | -------------- | ------------------------------------ | ---------- | -------- | --------- | ---------------------------------- |
| Decimal Example | 3148663466     | 9                                    | 0          | 1        | hello     | 17451                             |
| Hex Example     | 0xBBACCAAA     | 0x00000009                           | 0x00       | 0x01     | hello     | 0x442B                             |
| Byte Stream (Hex) | AA CA AC BB   | 09 00 00 00                         | 00         | 01       | 68 65 6C 6C 6F | 2B 44                       |

> **Note**: Multi-byte data uses little-endian (LE) encoding. For example, `data_len` is `0x00000006`, with `06` in the lowest byte, so it is sent first as `0x06`, followed by `0x00 0x00 0x00`.
> Strings are sent in order, e.g., `hello` is sent as `h`, then `e`, `l`, `l`, `o`.
> In this example, the final data sent is: `AA CA AC BB 09 00 00 00 00 01 68 65 6C 6C 6F 2B 44`.

- `header`: 4-byte header marking the start of a frame, fixed as `0xAA 0xCA 0xAC 0xBB`, sent starting with `0xAA`.
- `data len`: 4-byte data length, including the length of `flags`, `cmd`, `body`, and `CRC`.
- `flags`: 1-byte flag where each bit indicates:
  - MSB `is_resp`: Indicates if it is a response. `0` for request, `1` for response or active report (requires the third highest bit set to `1`).
  - Second highest bit `resp_ok`:
    - For requests: Reserved.
    - For responses: `1` for success, `0` for failure.
  - Third highest bit `is_report`:
    - For requests: Reserved.
    - For responses: `1` for active report, `0` for response message.
  - Bits 4-6: Reserved for future use.
  - Lowest 2 bits `version`: Protocol version, updated only for incompatible changes. Compatible changes do not require a version update. The current version is `0`.
- `cmd`: 1-byte command type, with several predefined commands listed in [Command Definitions](#command-definitions). Custom commands range from `0` to `maix.protocol.CMD.CMD_APP_MAX`.
- `body`: Variable length, `< (2^32 - 1)`. Different commands have different `body` content, detailed below for each command.
- `crc16`: 2-byte CRC check value using the `CRC IBM` method, verifying data integrity during transmission. See [C Code Implementation](#C-CRC16) or [Python CRC16](#Python-CRC16).

For encoding and decoding, see [Appendix: Code](#Appendix-Code).

---

### Request and Response

#### Sending Request

Data direction: MCU or other master device -> Maix device

Set the highest bit of `flags` (`is_resp`) to `0`. The `body` content is determined by `cmd`.

#### Receiving Response

Data direction: Maix device -> MCU or other master device

Set the highest bit of `flags` (`is_resp`) to `1`.

- For a successful response: Set `resp_ok` to `1`, and the `body` content is determined by `cmd`. An empty `body` is the simplest successful response.
- For a failed response:
  - Set `resp_ok` to `0`.
  - The first byte of `body` is the error code. Refer to [MaixCDK maix.err.Err](../../../components/basic/include/maix_err.hpp) for error codes.
  - The following bytes in `body` contain the error message in `UTF-8` encoding, preferably in plain English for better compatibility.

Each request should have a corresponding response, either successful (`RESP_OK`) or failed (`RESP_ERR`). If `RESP_OK` has no specified `body`, it is empty.


### Active Data Reporting

Data direction: Maix device -> MCU or other master device

The master device must first enable active reporting using the `CMD_SET_REPORT` command (only supported for specific commands).

> For example, if the APP supports the `CMD_GET_TEMP` command to get temperature data, the master can use the `CMD_SET_REPORT` command to enable periodic temperature reporting. If the APP does not support active reporting for `CMD_GET_TEMP`, it will respond with `CMD_ERROR`.

Set the highest bit of `flags` (`is_resp`) to `1` and the third bit `is_report` to `1`. The `body` content depends on the `cmd`.

---

## Examples

Example:  

The MCU connects to the Maix device via the serial port using the default baud rate of `115200`.

The MCU requests the list of applications from the Maix device. The steps are as follows:

1. According to the protocol specification, the MCU sends a request with `cmd` set to `0xF9` (`CMD_APP_LIST`), and an empty `body`. Actual byte stream:
   `AA CA AC BB 04 00 00 00 01 F9 C9 77`.

2. The Maix device responds with `cmd` set to `0xF9` and `flags` set to `0xC1` (with `|0x80` for response and `|0x40` for success). The `body` contains the application list. If there are two applications, the actual byte stream is:
   `AA CA AC BB 0A 00 00 00 C1 F9 02 66 61 63 65 00 66 61 63 65 00 F6 06`.

---

## Command Definitions

The `cmd` values in the protocol frame are defined as follows:

| Command Name    | Value | Description                   |
| --------------- | ----- | ----------------------------- |
| CMD_APP_MAX     | 0xC8  | Maximum value for custom commands (exclusive) |
| CMD_SET_REPORT  | 0xF8  | Set active reporting          |
| CMD_APP_LIST    | 0xF9  | Query application list        |
| CMD_START_APP   | 0xFA  | Start an application          |
| CMD_EXIT_APP    | 0xFB  | Exit the current application  |
| CMD_CUR_APP_INFO| 0xFC  | Query current application info|
| CMD_APP_INFO    | 0xFD  | Query specific application info|
| CMD_KEY         | 0xFE  | Simulate key press message    |
| CMD_TOUCH       | 0xFF  | Simulate touch message        |

Note that `CMD_APP_MAX` defines the range for custom commands, from `0` up to (but not including) `CMD_APP_MAX`.

Each `cmd` has specific request and response data and a unique `body`. Detailed explanations for each command are provided below:

---

### CMD_SET_REPORT

Enables or disables active reporting for specific commands.

Active reporting includes:
1. Event-based reporting, such as reporting detected faces.
2. Periodic reporting, such as reporting temperature every 5 seconds.

If both periodic and event-based reporting are enabled, the timer restarts after an event report.

#### Request

`body`:

|     | cmd (1B)      | on_off (1B) | event (1B) | timer (4B) |
| --- | ------------- | ----------- | ---------- | ---------- |
| Description | Command for active reporting | Enable (1) or disable (0) | Event reporting (1 for enable, 0 for disable) | Timer for periodic reporting in ms (set to 0 if not needed) |
| Example | 0x02       | 0x01        | 1          | 5000       |

#### Response

If the command supports active reporting, respond with `RESP_OK`; otherwise, respond with `RESP_ERR`.

`body`: None

---

### CMD_APP_LIST

Queries the list of available applications.

#### Request

`body`: None

#### Response

`body`:

|     | number (1B) | app1       | ... | app n info |
| --- | ----------- | ---------- | --- | -----------|
| Description | Number of applications | id1 (null-terminated string) | ... | idn        |
| Example     | 0x02          | 'face\0' | ... | 'appn\0'   |

---

### CMD_CUR_APP_INFO

Gets the current application information.

#### Request

`body`: None

#### Response

`body`:

|     | idx (1B) | app info (id + name + brief) |
| --- | -------- | ---------------------------- |
| Description | Application index | Application info (id, name in UTF-8, brief description in UTF-8) |
| Example     | 0x00   | 'face\0face\0face detect\0' |

---

### CMD_APP_INFO

Queries specific application information.

#### Request

`body`:

|     | idx (1B) | app_id (nB) |
| --- | -------- | ------------|
| Description | Application index | Application ID |
| Example     | 0x02    | 'face' |

Either `idx` or `app_id` can be specified:
* `idx`: Application index (starts from 0). Setting to `0xFF` means it is not specified.
* `app_id`: Application ID. If `idx` is specified, `app_id` can be omitted.

#### Response

`body`:

|     | idx (1B) | app info (id + name + brief) |
| --- | -------- | ---------------------------- |
| Example | 0x00   | 'face\0face\0face detect\0' |

---

### CMD_START_APP

Requests to start a specified application. This command will exit the current application and start the specified one.

* If the APP receives this command, it must call the API `maix.app.switch_app` to switch the application.
> There is a risk if the APP does not correctly implement the response to this command, prompting the developer to implement it.

* If the Launcher receives this command, it will start the specified application.

#### Request

`body`:

|    | idx (1B) | app_id (nB) | app_func (nB) |
| ---| -------- | ----------- | ------------- |
| Description | Application index (starts from 0, set to `0xFF` if unspecified) | Application ID (optional if `idx` is specified) | Function to execute if the application has multiple functions |
| Example     | 0x02        | 'scan'       | 'qrcode'   |

#### Response

`body`: None

---

### CMD_EXIT_APP

Requests to exit the current application.

#### Request

`body`: None

#### Response

`body`: None

---

### CMD_KEY

Sends a simulated key press request.

#### Request

`body`:

| key (4B) | value (1B) |
| -------- | ---------- |
| Key value (little-endian) | 0x01 (pressed), 0x00 (released), 0x02 (long press) |

#### Response

`body`: None

---

### CMD_TOUCH

Sends a simulated touch request.

#### Request

`body`:

| x       | y       | event (1B) |
| ------- | ------- | ---------- |
| x-coordinate | y-coordinate | Event type |

Event types:
* 0x00: Press
* 0x01: Release
* 0x02: Move

#### Response

`body`: None


## Application (APPS) Protocol Specification

### Camera

Commands:

| Command | Value | Description | Response |
| ---- | ---- | --- | ---- |
| CMD_SNAP | 0x01 | Take a photo |

### Classifier

TODO:

`body` Description:
The request consists of a single byte representing the command, as shown in the table below:

| Command | Value | Description | Response |
| ---- | ---- | --- | ---- |
| CMD_RECOGNIZE | 0x01 | Recognize object | CMD_APP_CMD |

Response:
* Response for command `CMD_RECOGNIZE`: The response `cmd` is `CMD_APP_CMD`, and the `body` is structured as follows:

| CMD_RECOGNIZE | id (2B uint16 LE) | prob (4B float LE) | name |
| --- | --- | ----- | ------ |
| CMD_RECOGNIZE value | Recognized id (index), little-endian | Probability, float, little-endian | Name, UTF-8 encoded |

### Face Detection

TODO:

`body` Description:
The request consists of a single byte representing the command, as shown in the table below:

| Command | Value | Description | Response |
| ---- | ---- | --- | ---- |
| CMD_POS | 0x01 | Detect face | CMD_APP_CMD |

Response:
* Response for command `CMD_POS`: The response `cmd` is `CMD_APP_CMD`, and the `body` is structured as follows:

| CMD_POS | face num (2B LE) | prob (4B float LE) | x (2B LE) | y (2B LE) | w (2B LE) | h (2B LE) | ... |
| --- | --- | ----- | ------ | --- | --- | ---- | --- |
| CMD_POS value | Number of detected faces | Probability, float, little-endian | Top-left x-coordinate of the face box | Top-left y-coordinate of the face box | Width of the face box | Height of the face box | Remaining faces... |

### Face Recognition

TODO:

`body` Description:
The request consists of a single byte representing the command, as shown in the table below:

| Command | Value | Description | Response |
| ---- | ---- | --- | ---- |
| CMD_FACES | 0x01 | Recognize faces | CMD_APP_CMD |
| CMD_USERS | 0x02 | Query all users | CMD_APP_CMD |
| CMD_RECORD | 0x03 | Record a face | CMD_APP_CMD |
| CMD_REMOVE | 0x04 | Remove a face | CMD_APP_CMD |

Request:

`CMD_APP_CMD` plus an additional byte for `app_cmd`. Some commands have extra parameters, as follows:

* Request for command `CMD_RECORD`:

| CMD_RECORD | user name |
| --- | --- |
| CMD_RECORD value | Name of the user being recorded |

* Request for command `CMD_REMOVE`:

| CMD_REMOVE | user idx (2B int16) | user name |
| --- | --- | --- |
| CMD_REMOVE value | User index, 2 bytes, little-endian | Name of the user to be removed |

Either index or username can be provided.

Response:

* Response for command `CMD_FACES`: The response `cmd` is `CMD_APP_CMD`, and the `body` is structured as follows:

| CMD_FACES | face num (2B LE) | id (2B) | name_len (1B) | name | prob (4B float LE) | x (2B LE) | y (2B LE) | w (2B LE) | h (2B LE) | ... |
| --- | --- | ----- | ------ | --- | --- | ---- | --- | --- | --- | --- |
| CMD_FACES value | Number of detected faces | Face ID (index) | Length of the name | Name, UTF-8 encoded | Probability, float, little-endian | Top-left x-coordinate of the face box | Top-left y-coordinate of the face box | Width of the face box | Height of the face box | Remaining faces... |

* Response for command `CMD_USERS`: The response `cmd` is `CMD_APP_CMD`, and the `body` is structured as follows:

| CMD_USERS | user num (2B LE) | name_len (1B) | name | ... |
| --- | --- | ----- | ------ | --- |
| CMD_USERS value | Number of users | Length of the username | Name, UTF-8 encoded | Remaining usernames... |

* Response for command `CMD_RECORD`: `CMD_OK` or `CMD_ERROR`

* Response for command `CMD_REMOVE`: `CMD_OK` or `CMD_ERROR`

## Appendix: Code

### C CRC16 IBM

```c
unsigned short crc16_IBM(unsigned char *ptr, int len)
{
    unsigned int i;
    unsigned short crc = 0x0000;

    while(len--)
    {
        crc ^= *ptr++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }

    return crc;
}
```

Alternatively, using a lookup table:
```c
const unsigned int crc16_table[256] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
    0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
    0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
    0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
    0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
    0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
    0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
    0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
    0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
    0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
    0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
    0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
    0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
    0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
    0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
    0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
    0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
    0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
    0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
    0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
    0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
    0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
    0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
    0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
    0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
    0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
    0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
    0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
    0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
};


unsigned short crc16_IBM(const unsigned char *ptr,int len)
{
    unsigned short crc = 0x0000;

    while(len--)
    {
        crc = (crc >> 8) ^ crc16_table[(crc ^ *ptr++) & 0xff];
    }

    return (crc);
}

```
