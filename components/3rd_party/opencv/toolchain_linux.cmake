set(CMAKE_SYSTEM_NAME Linux)
# CMAKE_SYSTEM_PROCESSOR from command `uname -m`
execute_process(COMMAND uname -m OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR OUTPUT_STRIP_TRAILING_WHITESPACE)

# set(GNU_MACHINE "arm-openwrt-linux" CACHE STRING "GNU compiler triple")

# if(NOT CMAKE_INSTALL_PREFIX)
#     set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/opencv4_install)
# endif()

# set(SDK_PATH $ENV{SDK_PATH}) # toolchain cmake file can not read var from cmake cmd's -D option, so use env var instead
# if(NOT TOOLCHAIN_PATH)
#     if($ENV{TOOLCHAIN_PATH} STREQUAL "")
#         set(TOOLCHAIN_PATH "${SDK_PATH}/dl/extracted/toolchains/m2dock/toolchain-sunxi-musl/toolchain/bin")
#     else()
#         set(TOOLCHAIN_PATH $ENV{TOOLCHAIN_PATH})
#     endif()
# endif()
# set(prefix "arm-openwrt-linux-")
# set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/${prefix}gcc)
# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/${prefix}g++)
# include(FindPkgConfig)

# include("$ENV{OPENCV_SRC_DIR}/platforms/linux/arm.toolchain.cmake")

