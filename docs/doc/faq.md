MaixCDK FAQ
===

## Downloading ippicv_2021.8_lnx_intel64_20230330_general.tgz takes a long time or fail

Manually download according to the log's url, and put it into:
`MaixCDK -> components -> opencv -> opencv4 -> .cache -> ippicv`
The file name is `43219bdc7e3805adcbe3a1e2f1f3ef3b-ippicv_2021.8_lnx_intel64_20230330_general.tgz`,
File name and url can also be found in `MaixCDK/components/3rd_party/opencv/opencv4/3rdparty/ippicv/ippicv.cmake`

So the same as `ade` cache file `.cache/ade/4f93a0844dfc463c617d83b09011819a-v0.1.2b.zip`

## Exception: parse_api_from_header **.hpp error: 'members'

API comment not complete.
e.g.

```cpp
/**
 * Class for communication protocol
 */
class CommProtocol
{
    /**
     * Read data to buffer, and try to decode it as maix.protocol.MSG object
     * @return decoded data, if nullptr, means no valid frame found.
     *         Attentioin, delete it after use in C++.
     * @maixpy maix.comm.CommProtocol.get_msg
     */
    protocol::MSG *get_msg();
}
```

Here `class CommProtocol` not add `@maixpy maix.comm.CommProtocol` but its method `get_msg` add it.
So we add `@maixpy maix.comm.CommProtocol` to `class CommProtocol` comment will fix this error.
