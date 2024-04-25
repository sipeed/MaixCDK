set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)

set(GNU_MACHINE "arm-openwrt-linux" CACHE STRING "GNU compiler triple")

# if(NOT CMAKE_INSTALL_PREFIX)
#     set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/freetype_install)
# endif()

# toolchain path must the same as sdk_path/platforms/xxxx.yaml
set(SDK_PATH $ENV{SDK_PATH}) # toolchain cmake file can not read var from cmake cmd's -D option, so use env var instead
if(NOT TOOLCHAIN_PATH)
    if($ENV{TOOLCHAIN_PATH} STREQUAL "")
        set(TOOLCHAIN_PATH "${SDK_PATH}/dl/extracted/toolchains/maixcam/toolchain-sunxi-musl/toolchain/bin")
    else()
        set(TOOLCHAIN_PATH $ENV{TOOLCHAIN_PATH})
    endif()
endif()
set(prefix "arm-openwrt-linux-")
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${prefix}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${prefix}g++)



