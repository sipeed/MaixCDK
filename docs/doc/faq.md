MaixCDK FAQ
===

You can also find FAQ from:
* [MaixPy FAQ](https://wiki.sipeed.com/maixpy/doc/en/faq.html)
* [MaixCAM FAQ](https://wiki.sipeed.com/hardware/zh/maixcam/faq.html)
* [MaixPy souce code FAQ](https://wiki.sipeed.com/maixpy/doc/zh/source_code/faq.html)


## Common Compilation Errors and General Solutions

* **Errors such as extraction failures**: Try deleting the corresponding files in the `dl/pkgs` and `dl/extracted` directories, then recompile and download again.

* **Compilation errors**:
  * Run `maixcdk build --verbose` to see where the error occurs. **Carefully check the error log** and investigate the issue step by step.
  * Run `maixcdk distclean` to clean temporary files, then try recompiling.
  * Visit the [GitHub commit history](https://github.com/sipeed/MaixCDK/commits/main/) to check whether the automated tests for each commit passed (a green checkmark ✅ means it passed, a red cross ❌ means it failed). You can switch your local code to a commit that passed the tests ✅ and recompile. Use the command `git checkout <commit-hash>`, for example: `git checkout 3aba2fe3fa9de9f638bb9cb34eca0c2e0f5f3813`. If switching fails, you may need to search for Git usage instructions or start over, but make sure to back up any changes you made to your code.

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
