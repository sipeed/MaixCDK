---
title: Maix communicate Protocol
update:
  - date: 2023-11-28
    author: neucrack
    version: 1.0.0
    content: 设计并编写协议文档和代码 API 实现
---

## Maix 串口协议简介


运行 MaixCDK 或者 MaixPy 的设备， 除了可以直接使用设备上的触摸屏和按键操作外， 还可以作为模块，可以使用 `UART`（串口） 或者 `I2C` 进行通信控制。

> 因为 I2C 为主从模式，所以协议都采用问答模式，一问一答，即`Request`+`Response`
> 对于 UART 协议， 部分功能支持配置主动上报，具体请看协议

后文会详细阐述通信协议内容。

## Maix 设备端如何使用

开机进入 `设置` 应用，在 `通信` 设置里面选择通信接口，有：
* **串口**：选择后会使用串口进行通信，串口根据板子决定，波特率默认为`115200`。
* **TCP**：选择后 Maix 设备会启动一个 TCP 服务，主控端可以通过 TCP 连接 Maix 设备，端口默认为`5555`。

然后主控连接上 Maix 设备后，就可以通过协议进行通信了， 主控作为主设备， Maix 设备作为从设备。

**Maix 设备端开发**：

用提供的`maix.comm.CommProtocol`类可以很方便实现命令响应，具体参考例程 [comm_protocol](../../../examples/protocol_demo)（C++）或者 [comm_protocol.py](https://github.com/sipeed/MaixPy/blob/main/examples/protocol)（MaixPy）。


**主控端开发**：

主控端根据芯片和开发语言可自行实现协议，在文末附录中有提供模板代码。


## 数据帧

|               | header(4B LE) |  data len(4B LE)(flags+cmd+body+crc) |flags(1B)  | cmd(1B) | body(nB)     | CRC16_IBM(2B LE)(前面所有) |
| ------------- | ---------- | ---------------------------- | ------------------ | ------- | ---------------- | ---------------------- |
| 十进制示例      | 3148663466 | 9                            | 0                  | 1       | hello            | 17451        |
| 十六进制示例    | 0xBBACCAAA | 0x00000009                   |  0x00              | 0x01    | hello             | 0x442B       |
| 字节流(十六进制)| AA CA AC BB | 09 00 00 00                 | 00                 | 01      |  68 65 6c 6c 6F   | 2B 44 |

> **注意**这里超过一个字节的数据均采用小端(LE)编码，比如这里 `data_len` 为 `0x00000006`，`06`在最低位，发送时需要先发送`0x06`再发送`0x00` `0x00` `0x00`。
> 字符串就按照顺序发即可，比如`hello`，先发`h`再发`e` `l` `l` `o`。
> 这里的例子，最终发送数据： `AA CA AC BB 09 00 00 00 00 01 68 65 6c 6c 6F 2B 44`。

* `header`: 头，4 字节，用来标识一帧的开始，固定为 4 字节 `0xAA 0xCA 0xAC 0xBB`, 先发`0xAA`
* `data len`: 数据长度， 4 字节， 包含了 `flags` `cmd` + `body` + `CRC` 的长度。
* `flags`: 一个字节，各个位分别表示：
  * 最高位 `is_resp`:  是否是响应，`0`代表发送请求，`1`代表 响应 或者 主动上报（第三高位需要置`1`）。
  * 第二高位 `resp_ok`:
    * 对于请求，保留位。
    * 对于响应，`1`表示成功，`0`表示失败。
  * 第三高位 `is_report`：
    * 对于请求，保留位。
    * 对于响应，`1`表示主动上报，`0`表示为响应消息。
  * 第 4~6 高位： 保留以后使用。
  * 低 2位 `version`: 协议版本，以后修改不兼容的协议才会更改这个版本号，如果修改协议后与当前版本协议兼容则这个版本号不需要升级。目前的版本为 `0`。
* `cmd`: 基本命令类型，1 字节，已经预先定义了几个命令，看后文[命令定义](#命令定义)，这些预定义命令的值从 255 逐渐递减，APP 自定义的命令可以从 `0 ~ maix.protocol.CMD.CMD_APP_MAX`。
* `body`: 内容，变长, 长度只要 `< (2^32 - 1)` 即可，不同的命令对应不同的`body`，在后面会对每个命令进行详细说明
* `crc16`: CRC 校验值，2 字节， 用以校验帧数据在传输过程中是否出错，使用 `CRC IBM` 方法计算，可以参见[C 代码实现](#C-CRC16)，或者 [Python CRC16](#Python-CRC16)。和`data_len`同样两个字节小端存放，传输时先传输低位再传输高位。

编解码的代码见[附录：代码](#附录：代码)

## 请求和响应

### 发送方主动请求

数据方向为： 单片机或其它主控设备 -> Maix 设备

`flags`最高位`is_resp`设置为`0`， `body` 内容由`cmd`决定;

### 接收方响应

数据方向为： Maix 设备 -> 单片机或其它主控设备

`flags`最高位`is_resp`设置为`1`。

* 对于成功响应：`resp_ok` 设置为 `1`, `body` 内容由`cmd`决定, 其中最简单的成功响应即 `body` 为空。
* 对是失败响应：
  * `resp_ok`设置为 `0`。
  * `body` 第一个字节为错误码, 具体的错误码见[MaixCDK maix.err.Err](../../../components/basic/include/maix_err.hpp)。
  * `body` 后面的字节为错误字符串信息，`UTF-8` 编码，一般情况下建议用纯英文，提高兼容性。

每个请求均应有对应的响应，即执行成功或者失败，后面均用`RESP_OK`和`RESP_ERR`来代替执行成功和失败两种响应。
`RESP_OK`的 `body` 字节不说明就是没有，有会单独进行说明。

### 主动上报数据

数据方向为： Maix 设备 -> 单片机或其它主控设备

需要主控先通过`CMD_SET_REPORT`命令设置需要主动上报的数据（需要对应的命令支持）
> 比如 APP 支持 `CMD_GET_TEMP` 命令获取温度，主控可以通过 `CMD_SET_REPORT` 命令设置主动定时上报温度，然后 APP 收到这个命令后，就会定时上报温度，如果 APP 不支持设置`CMD_GET_TEMP`命令的主动上报，则会响应`CMD_ERROR`。

`flags`最高位`is_resp`设置为`1`，第三位`is_report` 设置为 `1`，`body` 内容由`cmd`决定。


## 实例

举例：

单片机与 Maix 设备通过串口连接，默认使用`115200`波特率。

单片机向 Maix 设备请求获取 应用列表，步骤：

1. 根据下方的协议说明，单片机通过串口发送`cmd`为`0xF9`的请求，即`CMD_APP_LIST`，`body`为空。实际字节流：
`AA CA AC BB  04 00 00 00  01 F9  C9 77`。

2. Maix 设备收到请求后，返回`cmd`为`0xF9`, `flags`为`0x01 | 0x80 | 0x40 => 0xC1`的响应（响应`flags`最高为`1`所以有`|0x80`, 成功响应第二高位为`1`所以有`|0x40`），`body`内容为应用列表，具体内容见下方协议说明。假如有两个 APP,实际字节流：
`AA CA AC BB  0A 00 00 00  C1 F9  02  66 61 63 65 00  66 61 63 65 00  F6 06`。


## 命令定义

协议帧中的`cmd`取值如下：

| 命令名            | 值   |       含义      |
| ---------------- | ---- | -------------- |
|CMD_APP_MAX       | 0xC8 | 应用可自定义命令最大值（不含） |
|CMD_SET_REPORT    | 0xF8 | 设置主动上报     |
|CMD_APP_LIST      | 0xF9 | 应用列表查询     |
|CMD_START_APP     | 0xFA | 启动应用        |
|CMD_EXIT_APP      | 0xFB | 退出应用        |
|CMD_CUR_APP_INFO  | 0xFC | 当前应用信息查询  |
|CMD_APP_INFO      | 0xFD | 应用信息查询     |
|CMD_KEY           | 0xFE | 按键模拟消息     |
|CMD_TOUCH         | 0xFF | 触摸模拟消息     |

注意这里有个 `CMD_APP_MAX`， 任何 APP 自定义的命令都应该在`0 ~ CMD_APP_MAX`之间，包含`0`，不包含`CMD_APP_MAX`。

不同的`cmd`对应不同的请求和响应数据， 以及不同的`body`，详细协议（主要阐述`cmd`和`body`）如下：

### CMD_SET_REPORT

开启或关闭命令主动上报。
主动上报包括：
1. 有事件时主动上报，比如检测到人脸时主动上报人脸信息。
2. 定时上报，比如每隔 5s 上报一次温度。

如果定时上报和事件上报同时开启，则事件上报后，定时器重新计时。

#### 请求

`body`：

|     | cmd(1B)             | on_off(1B)   | event(1B) | timer(4B) |
| --- | ------------------- |  ----------- | --------- |---------- |
| 解释 | 需要开启主动上报的命令  | 开关(1 开， 0 关) | 事件上报（1 开， 0 关）  | 定时上报，单位 ms，不需要定时则设置为 0 |
| 例子 |  0x02               |  0x01 | 5000 |

#### 响应

如果相应的命令支持主动上报，则响应`RESP_OK`，否则响应`RESP_ERR`，均由应用自行决定。

`body`： 无


### CMD_APP_LIST

获取应用列表

#### 请求

`body` 为空

#### 响应

`body`:

|     | number(1B) | app1 | ... | app n info |
| --- | ---------- | ---------- | ---------- | ----- |
| 解释 | 应用数量    | id1 + '\0'结尾的字符串 | ... | idn |
| 例子 |  0x02      | 'face\0' | ... | 'appn\0' |


### CMD_CUR_APP_INFO

获取当前应用信息

#### 请求

`body` 为空

#### 响应

`body`:

|     | idx(1B) | app info(id + name + brief)|
| --- | ------- | ---------- |
| 解释 | 应用序号 | 应用信息（id， 名字(UTF-8 编码)，简介（UTF-8 编码）） |
| 例子 | 0x00    | 'face\0face\0face detect\0' |

### CMD_APP_INFO

获取指定应用信息

#### 请求

`body` ：

|     | idx(1B) | app_id(nB) |
| --- |-------- | ------ |
| 解释 | 应用序号 | 应用 ID |
| 例子 | 0x02    | 'face' |

`idx` 和 `app_id` 二选一即可
* `idx`: 应用序号（从0开始），设置为 0xFF 表示不设置
* `app_id`: 应用 ID，如果 idx 设置了，可以不用设置

#### 响应

`body`:

|     | idx(1B) | app info(id + name + brief)|
| --- | ------- | ---------- |
| 解释 | 应用序号 | 应用信息（id， 名字(UTF-8 编码)，简介（UTF-8 编码）） |
| 例子 |   0x00  | 'face\0face\0face detect\0' |



### CMD_START_APP

请求启动指定应用，执行此命令会退出当前应用，然后启动指定应用。
* 如果是 APP 收到这个命令，则它需要调用 API `maix.app.switch_app` 切换 APP。
> 这里存在一个风险，就是如果 APP 没有正确实现响应这个命令，则可以敦促开发者实现这个命令。

* 如果时 Launcher 收到这个命令后会启动对应的应用。

#### 请求

`body`：

|    |  idx(1B)                           | app_id(nB) | app_func(nB) |
| ------- | ---------------------------------- | ------ | --------- |
| 解释 | 应用序号（从0开始），设置为 0xFF 表示不设置 | 应用 ID，如果 idx 设置了，可以不用设置 | 该应用存在多个功能时用于指定应用启动时执行的功能 |
| 例子 | 0x02    | 'scan' | 'qrcode' |

`idx` 、 `app_id` 和 `app_func` 三个参数如何使用：

* 不带参数启动应用: 单独设置 `idx` 或者 `app_id`

* 带参数启动应用:

  > `idx` + `app_func`，设置 `idx` 后，解释器会将后续的第一个字符串视为 `app_func`，如果有多个字符串将会返回错误。

  > `app_id` + `app_func`，不设置 `idx`，解释器会在后续字符串中查找 `app_id` 和 `app_func`，不符合条件将会返回错误。


#### 响应

`body`: 无


### CMD_EXIT_APP

请求退出当前应用

#### 请求

无 `body`

#### 响应

`body`: 无


### CMD_KEY

发送模拟按键请求

#### 请求

`body`：

| key(4B) | value(1B) |
| ------------- | --------- |
| 键值(小端)    | 取值：0x01(/0x00/0x02) |

* `key`: 键值，4 字节，发送时需要按小端编码，比如`0x00000001` 发送时的字节流为`0x01 0x00 0x00 0x00`，支持的取值为：
  * 38: "up"
  * 40: "down"
  * 37: "left"
  * 39: "right"
  * 108: "enter"
  * 27: "esc"
  * 0x01010101: "ok"
  * 0x02020202: "ret"
  * 0x03030303: "pre"
  * 0x04040404: "next"
* `value`: 按键值， 1 字节
  * 0x01: 按下
  * 0x00: 释放
  * 0x02: 长按

#### 响应

`body`: 无


### CMD_TOUCH

发送模拟触摸请求

#### 请求

`body`：

| x     | y      | event(1B) |
| ----- | ------ | --------- |
| x 坐标 | y 坐标 | 事件|

event 取值：
* 0x00: 按下
* 0x01: 抬起
* 0x02: 移动


#### 响应

`body`：无


## 应用（APPS）协议说明

### 相机

命令:

|  命令 | 取值 | 含义 | 响应 |
| ---- | ---- | --- | ---- |
| CMD_SNAP    | 0x01 | 拍照 |

### 分类器

TODO:

`body`说明：
请求只有一个字节， 代表了命令， 具体如下表：

|  命令 | 取值 | 含义 | 响应 |
| ---- | ---- | --- | ---- |
| CMD_RECOGNIZE    | 0x01 | 识别物体 | CMD_APP_CMD |

响应：
* 命令 `CMD_RECOGNIZE` 的响应： 响应 `cmd`为 `CMD_APP_CMD`, `body`:

| CMD_RECOGNIZE | id(2B uint16 LE) | prob(4B float LE) | name |
| --- | --- | ----- | ------ |
| CMD_RECOGNIZE 值  | 识别到的 id（下标），小端 | 概率， 浮点型， 小端 | 名字， UTF-8 编码 |


### 人脸检测

TODO:

`body`说明：
请求只有一个字节， 代表了命令， 具体如下表：

|  命令 | 取值 | 含义 | 响应 |
| ---- | ---- | --- | ---- |
| CMD_POS    | 0x01 | 检测人脸 | CMD_APP_CMD |

响应：
* 命令 `CMD_POS` 的响应： 响应 `cmd`为 `CMD_APP_CMD`, `body`:

| CMD_POS | face num(2B LE) | prob(4B float LE) | x(2B LE) | y(2B LE) | w(2B LE) | h(2B LE) | ... |
| --- | --- | ----- | ------ | --- | --- | ---- | --- |
| CMD_POS 值 | 检测到的人脸数量 | 概率， 浮点型， 小端 | 人脸框左上角横坐标 | 人脸框左上角纵坐标 | 人脸框宽 | 人脸框高 | 剩下的人脸... |

### 人脸识别

TODO:

`body`说明：
请求只有一个字节， 代表了命令， 具体如下表：

|  命令 | 取值 | 含义 | 响应 |
| ---- | ---- | --- | ---- |
| CMD_FACES    | 0x01 | 识别人脸 | CMD_APP_CMD |
| CMD_USERS    | 0x02 | 查询所有用户 | CMD_APP_CMD |
| CMD_RECORD    | 0x03 | 录入人脸 | CMD_APP_CMD |
| CMD_REMOVE    | 0x04 | 删除人脸 | CMD_APP_CMD |

请求

`CMD_APP_CMD`加一个字节的`app_cmd`，个别命令有额外的参数，如下：

* 命令 `CMD_RECORD` 的请求

| CMD_RECORD | user name |
| --- | --- |
| CMD_RECORD 值 | 录制的用户名 |

* 命令 `CMD_REMOVE` 的请求

| CMD_REMOVE | user idx(2B int16) | user name |
| --- | --- | --- |
| CMD_REMOVE 值 | 用户下标， 两字节，小端 | 要删除的用户名 |

下标和用户名二选一



响应：

* 命令 `CMD_FACES` 的响应： 响应 `cmd`为 `CMD_APP_CMD`, `body`:

| CMD_FACES |  face num(2B LE) | id(2B) | name_len(1B) | name | prob(4B float LE) | x(2B LE) | y(2B LE) | w(2B LE) | h(2B LE) | ... |
| --- | --- | ----- | ------ | --- | --- | ---- | --- | --- | --- | --- |
| CMD_FACES 值 | 检测到的人脸数量 | 人脸 ID（下标） | 名字长度 | 名字， UTF-8 编码 | 概率， 浮点型， 小端 | 人脸框左上角横坐标 | 人脸框左上角纵坐标 | 人脸框宽 | 人脸框高 | 剩下的人脸... |

* 命令 `CMD_USERS` 的响应： 响应 `cmd`为 `CMD_APP_CMD`, `body`:

| CMD_USERS | user num(2B LE) | name_len(1B) | name | ... |
| --- | --- | ----- | ------ | --- |
| CMD_USERS 值 | 用户数量 | 用户名长度 | 名字， UTF-8 编码 | 剩下的用户名... |

* 命令 `CMD_RECORD` 的响应： `CMD_OK` 或者 `CMD_ERROR`

* 命令 `CMD_REMOVE` 的响应： `CMD_OK` 或者 `CMD_ERROR`



## 附录：代码

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

或者查表法：
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


