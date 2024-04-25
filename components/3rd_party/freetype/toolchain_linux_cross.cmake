set(CMAKE_SYSTEM_NAME Linux)

set(toolchain $ENV{TOOLCHAIN_PATH})
set(prefix $ENV{TOOLCHAIN_PREFIX})

string(LENGTH "${toolchain}" toolchain_prefix_len)
math(EXPR toolchain_prefix_len "${toolchain_prefix_len} - 1")
string(SUBSTRING "${prefix}" 0 ${toolchain_prefix_len} prefix_no_minus)
# set(GNU_MACHINE "${prefix_no_minus}" CACHE STRING "GNU compiler triple")

execute_process(COMMAND ${toolchain}/${prefix}gcc -dumpmachine OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
string(REGEX REPLACE "([^-]+)-.*" "\\1" CMAKE_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")

set(CMAKE_C_COMPILER ${toolchain}/${prefix}gcc)
set(CMAKE_CXX_COMPILER ${toolchain}/${prefix}g++)

# if ${toolchain}/../sysroot exists, set CMAKE_SYSROOT
if(EXISTS ${toolchain}/../sysroot)
    set(CMAKE_SYSROOT ${toolchain}/../sysroot)
endif()


