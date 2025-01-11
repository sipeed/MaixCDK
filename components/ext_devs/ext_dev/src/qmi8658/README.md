# Qmi8658c Library for MaixCDK

This component is based on [ALICHOUCHENE/Qmi8658c](https://github.com/ALICHOUCHENE/Qmi8658c), which is licensed under the [MIT License](./ALICHOUCHENE_Qmi8658c/LICENSE). We have modified some of the code to adapt it to the MaixCDK platform.

## Example

Here is a simple example demonstrating how to initialize and read data from the QMI8658C IMU:

```cpp
#include "maix_basic.hpp"
#include "main.h"
#include "maix_qmi8658.hpp"

using namespace maix;

int _main(int argc, char* argv[])
{
    ext_dev::qmi8658::QMI8658 qmi8658;

    while (!app::need_exit()) {
        auto res = qmi8658.read();
        printf("\n");
        log::info("------------------------");
        printf("acc x:  %f\t", res[0]);
        printf("acc y:  %f\t", res[1]);
        printf("acc z:  %f\n", res[2]);
        printf("gyro x: %f\t", res[3]);
        printf("gyro y: %f\t", res[4]);
        printf("gyro z: %f\n", res[5]);
        printf("temp:   %f\n", res[6]);
        log::info("------------------------\n");
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

- Original library by [ALICHOUCHENE](https://github.com/ALICHOUCHENE)
- Inspiration and guidance from the open-source community

---