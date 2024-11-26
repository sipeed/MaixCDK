MaixCDK FAQ
===

You can also find FAQ from:
* [MaixPy FAQ](https://wiki.sipeed.com/maixpy/doc/en/faq.html)
* [MaixCAM FAQ](https://wiki.sipeed.com/hardware/zh/maixcam/faq.html)
* [MaixPy 源码 FAQ](https://wiki.sipeed.com/maixpy/doc/zh/source_code/faq.html)

## 常见编译错误的通用解决方法

* **提示解压失败等错误**: 可以尝试删除掉`dl/pkgs`和`dl/extracted`目录下的对应文件让重新编译下载即可。
* **编译报错**：
  * 执行`maixcdk build --verbose` 查看哪里报错，**仔细看报错日志**一步一步探寻问题。
  * 执行 `maixcdk distclean` 清理临时文件后重新编译。
  * 到[github 提交记录](https://github.com/sipeed/MaixCDK/commits/main/)处看到每个提交的自动测试是否通过（绿色的勾勾✅就是通过，红色叉叉❌就是失败），可以将本地代码切换到测试通过的提交✅再编译（本地执行`git checkout 提交号`比如`git checkout 3aba2fe3fa9de9f638bb9cb34eca0c2e0f5f3813`， 如果切换失败请自行搜索 git 用法或者直接从头来过，注意备份自己修改的代码）。

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
