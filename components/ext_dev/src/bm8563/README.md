# BM8563 RTC Library for MaixCDK

This component is based on [tuupola/bm8563](https://github.com/tuupola/bm8563), which is licensed under the [MIT License](./tuupola_bm8563/LICENSE). We have modified some of the code to adapt it to the MaixCDK platform.

## Usage

Here is a simple example demonstrating how to set and read the time from the RTC:

```cpp
#include "maix_bm8563.h"
#include "maix_basic.hpp"

using namespace maix;

int _main(int argc, char* argv[])
{
    auto bm8653 = ext_dev::bm8563::BM8563();

    while (!app::need_exit()) {
        time::sleep_ms(1000);
        auto rt = bm8653.datetime();
        log::info("bm8653: %d-%d-%d %d:%d:%d\n\n", rt[0], rt[1], rt[2], rt[3], rt[4], rt[5]);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    // Catch signal and process
    sys::register_default_signal_handle();

    // Use CATCH_EXCEPTION_RUN_RETURN to catch exception,
    // if we don't catch exception, when program throw exception, the objects will not be destructed.
    // So we catch exception here to let resources be released(call objects' destructor) before exit.
    CATCH_EXCEPTION_RUN_RETURN(_main, -1, argc, argv);
}

```

## Acknowledgments

- Original library by [tuupola](https://github.com/tuupola)
- Inspiration and guidance from the open-source community

---