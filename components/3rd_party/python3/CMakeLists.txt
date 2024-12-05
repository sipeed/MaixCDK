############### Add include ###################
# list(APPEND ADD_INCLUDE "include"
#     )
# list(APPEND ADD_PRIVATE_INCLUDE "")
###############################################

############ Add source files #################
# list(APPEND ADD_SRCS  "src/main.c"
#                       "src/test.c"
#     )
# append_srcs_dir(ADD_SRCS "src")       # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test2.c")
# FILE(GLOB_RECURSE EXTRA_SRC  "src/*.c")
# FILE(GLOB EXTRA_SRC  "src/*.c")
# list(APPEND ADD_SRCS  ${EXTRA_SRC})
# aux_source_directory(src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
# set(ADD_ASM_SRCS "src/asm.S")
# list(APPEND ADD_SRCS ${ADD_ASM_SRCS})
# SET_PROPERTY(SOURCE ${ADD_ASM_SRCS} PROPERTY LANGUAGE C) # set .S  ASM file as C language
# SET_SOURCE_FILES_PROPERTIES(${ADD_ASM_SRCS} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D BBBBB")
###############################################

###### Add required/dependent components ######
# list(APPEND ADD_REQUIREMENTS basic)
# list(APPEND ADD_FILE_DEPENDS include/axx.h)
# set_property(SOURCE ${python_h_path} PROPERTY GENERATED 1)
# add_custom_command(OUTPUT include/axx.h
#             COMMAND echo "" > include/axx.h
#             COMMENT "Generating axx.h ..."
#         )
###############################################

###### Add link search path for requirements/libs ######
# list(APPEND ADD_LINK_SEARCH_PATH "${CONFIG_TOOLCHAIN_PATH}/lib")
# list(APPEND ADD_REQUIREMENTS pthread m)  # add system libs, pthread and math lib for example here
# set (OpenCV_DIR opencv/lib/cmake/opencv4)
# find_package(OpenCV REQUIRED)
###############################################

############ Add static libs ##################
# list(APPEND ADD_STATIC_LIB "lib/libtest.a")
###############################################

############ Add dynamic libs ##################
# list(APPEND ADD_DYNAMIC_LIB "lib/arch/v831/libmaix_nn.so"
#                             "lib/arch/v831/libmaix_cam.so"
# )
###############################################

#### Add compile option for this component ####
#### Just for this component, won't affect other 
#### modules, including component that depend 
#### on this component
# list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

#### Add compile option for this component
#### Add components that depend on this component
# list(APPEND ADD_DEFINITIONS -DAAAAA222=1
#                             -DAAAAA333=1)
###############################################

############ Add static libs ##################
#### Update parent's variables like CMAKE_C_LINK_FLAGS
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
###############################################

set(compile_for_host 0)

if(CONFIG_PYTHON_VERSION_MAJOR STREQUAL "0")
    if(PLATFORM_LINUX)
        set(compile_for_host 1)
        set(py_version_cmd COMMAND ${python} --version)
        execute_process(${py_version_cmd} OUTPUT_VARIABLE py_version_str RESULT_VARIABLE cmd_res)
        if(NOT cmd_res EQUAL 0)
            message(FATAL_ERROR "Execute ${py_version_cmd} failed: ${py_version_str}")
        endif()
        string(REPLACE "Python " "" py_version_str ${py_version_str})
        string(REPLACE "\n" "" py_version_str ${py_version_str})
        string(REPLACE "." ";" py_version_list ${py_version_str})
        list(GET py_version_list 0 CONFIG_PYTHON_VERSION_MAJOR)
        list(GET py_version_list 1 CONFIG_PYTHON_VERSION_MINOR)
        list(GET py_version_list 2 CONFIG_PYTHON_VERSION_PATCH)
    elseif(PLATFORM_M2DOCK OR PLATFORM_MAIXCAM)
        set(CONFIG_PYTHON_VERSION_MAJOR "3")
        set(CONFIG_PYTHON_VERSION_MINOR "11")
        set(CONFIG_PYTHON_VERSION_PATCH "6")
    else()
        message(FATAL_ERROR "no default python version for this board, please set it manually in menuconfig")
    endif()
endif()

# write build python version to CMAKE_BINARY_DIR/config/python_version.txt
file(WRITE ${CMAKE_BINARY_DIR}/config/python_version.txt "${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}.${CONFIG_PYTHON_VERSION_PATCH}")

execute_process(COMMAND python3-config --includes OUTPUT_VARIABLE host_include_str RESULT_VARIABLE cmd_res)
if(NOT cmd_res EQUAL 0)
    message(FATAL_ERROR "Execute python3-config --includes failed: ${host_include_str}")
endif()
# convert include strings to string list, string format like -I/usr/include/python3.10 -I/usr/include/python3.10
string(REPLACE "-I" "" host_include_str ${host_include_str})
string(REPLACE "\n" "" host_include_str ${host_include_str})
string(REPLACE " " ";" host_include_str ${host_include_str})
list(REMOVE_DUPLICATES host_include_str)
message("Python3 include dir: ${host_include_str}")

if(NOT compile_for_host)
    set(python_version_str "${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}.${CONFIG_PYTHON_VERSION_PATCH}")
    set(py_ver_short "${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}")

    # use prebuilt python for maixcam
    if(PLATFORM_MAIXCAM AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
        if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/python_target_install")
            set(python_lib_dir "${CMAKE_CURRENT_LIST_DIR}/python_target_install")
        elseif(CONFIG_TOOLCHAIN_PATH MATCHES "musl")
            set(python_lib_dir "${DL_EXTRACTED_PATH}/python3/python3_lib_maixcam_musl_3.11.6")
        endif()
        if(python_lib_dir)
            set(python_dl_lib ${python_lib_dir}/lib/libpython${py_ver_short}.so)
            list(APPEND ADD_INCLUDE ${python_lib_dir}/include/python${py_ver_short})
            list(APPEND ADD_DYNAMIC_LIB ${python_dl_lib})
            set_property(SOURCE ${python_dl_lib} PROPERTY GENERATED 1)
        endif()
    endif()

    if(NOT ADD_INCLUDE) # compile from source code
        # check host python version, if equal to target python version, use host python, or compile python for host and target
        set(py_version_cmd COMMAND ${python} --version)
        execute_process(${py_version_cmd} OUTPUT_VARIABLE py_version_str RESULT_VARIABLE cmd_res)
        if(NOT cmd_res EQUAL 0)
            message(FATAL_ERROR "Execute ${py_version_cmd} failed: ${py_version_str}")
        endif()
        string(REPLACE "Python " "" py_version_str ${py_version_str})
        string(REPLACE "\n" "" py_version_str ${py_version_str})
        string(REPLACE "." ";" py_version_list ${py_version_str})
        list(GET py_version_list 0 py_version_major)
        list(GET py_version_list 1 py_version_minor)
        if(py_version_major EQUAL CONFIG_PYTHON_VERSION_MAJOR AND py_version_minor EQUAL CONFIG_PYTHON_VERSION_MINOR)
            set(compile_host_python 0)
            set(python_host_install_dir "")
            set(host_python_path ${python})
            set(host_python_bin_dir "")
            message("Host Python match target Python version, will use host python directly")
        else()
            set(compile_host_python 1)
            set(python_host_install_dir "${CMAKE_BINARY_DIR}/python_host_install")
            set(host_python_path ${python_host_install_dir}/bin/python3)
            set(host_python_bin_dir ${python_host_install_dir}/bin)
            message(WARNING "Host python version ${py_version_str} is not equal to target python version ${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}.${CONFIG_PYTHON_VERSION_PATCH}, will compile python for host first, this will takes a long tme\n"
                            "It's recommended to install python version ${py_ver_short} first to skip compile host python to makes build faster\n"
                            "To install Python ${py_ver_short} by conda(https://docs.anaconda.com/free/miniconda/index.html):\n\tconda create -n python${py_ver_short} python=python${py_ver_short}"
            )
            # sleep 3s
            execute_process(COMMAND sleep 1)
            message("build after 2s")
            execute_process(COMMAND sleep 1)
            message("build after 1s")
            execute_process(COMMAND sleep 1)
        endif()
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE arch)
        string(REGEX REPLACE "([^-]+)-.*" "\\1" arch "${arch}")
        if(PLATFORM_LINUX)
            set(python_target_install_dir ${python_host_install_dir})
        else()
            set(python_target_install_dir "${CMAKE_BINARY_DIR}/python_target_install")
            if(PLATFORM_M2DOCK)
                set(host_prefix "arm-openwrt-linux")
            elseif(PLATFORM_MAIXCAM)
                string(LENGTH "${CONFIG_TOOLCHAIN_PREFIX}" toolchain_prefix_len)
                math(EXPR toolchain_prefix_len "${toolchain_prefix_len} - 1")
                string(SUBSTRING "${CONFIG_TOOLCHAIN_PREFIX}" 0 ${toolchain_prefix_len} host_prefix)
            else()
                message(FATAL_ERROR "please edit here to add support for this board")
            endif()
        endif()
        set(python_include_dir "${python_target_install_dir}/include/python${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}")
        set(python_h_path "${python_include_dir}/Python.h")
        set(python_lib_file "${python_target_install_dir}/lib/libpython${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}.so")


        list(APPEND ADD_INCLUDE ${python_include_dir})
        list(APPEND ADD_DYNAMIC_LIB ${python_lib_file})
        set_property(SOURCE ${python_include_dir} PROPERTY GENERATED 1)
        set_property(SOURCE ${python_h_path} PROPERTY GENERATED 1)
        set_property(SOURCE ${python_lib_file} PROPERTY GENERATED 1)

        set(python_unzip_path "${DL_EXTRACTED_PATH}/python_srcs")
        set(python_src_path "${python_unzip_path}/Python-${python_version_str}")
        set(zlib_src_path "${DL_EXTRACTED_PATH}/zlib/zlib-1.3")
        set(zlib_install_path "${python_target_install_dir}")
        set(openssl_src_path "${DL_EXTRACTED_PATH}/openssl/openssl-3.0.12")
        set(openssl_install_path "${python_target_install_dir}")

        if(PLATFORM_LINUX)
            set(zlib_compile_cmd COMMAND set -x && cd ${zlib_src_path} && make distclean || true && echo "-- compile zlib for python now" && chmod +x ./configure && ./configure --prefix=${zlib_install_path} && make -j`nproc` && make install)
            set(openssl_compile_cmd COMMAND set -x && cd ${openssl_src_path} && make distclean || true && echo "-- compile openssl for python now" && chmod +x ./Configure && ./Configure linux-generic32 shared  -DL_ENDIAN --openssldir=${openssl_install_path} --prefix=${openssl_install_path} && make -j`nproc` && make install_sw)
            set(python_compile_cmd_target COMMAND set -x && cd ${python_src_path} && make distclean || true && chmod +x ./configure && CFLAGS="-I${zlib_install_path}/include" LDFLAGS="-L${zlib_install_path}/lib" CPPFLAGS="-I${zlib_install_path}/include" ./configure --prefix=${python_target_install_dir} --with-openssl="${openssl_install_path}" --enable-shared --enable-optimizations && echo "-- compile python now" && make -j`nproc` && make install)
        else()
            set(zlib_compile_cmd COMMAND set -x && cd ${zlib_src_path} && echo "-- compile zlib for python now" && make distclean || true && chmod +x ./configure && export PATH=${CONFIG_TOOLCHAIN_PATH}:${host_python_bin_dir}:$ENV{PATH} && CHOST="${host_prefix}" CC="${host_prefix}-gcc" CXX="${host_prefix}-g++" AR="${host_prefix}-ar" LD="${host_prefix}-ld" RANLIB="${host_prefix}-ranlib" ./configure --prefix=${zlib_install_path} && make -j`nproc` && make install)
            set(openssl_compile_cmd COMMAND set -x && cd ${openssl_src_path} && echo "-- compile openssl for python now" && make distclean || true && chmod +x ./Configure && ./Configure linux-generic32 shared  -DL_ENDIAN --openssldir=${openssl_install_path} --prefix=${openssl_install_path} && export PATH=${CONFIG_TOOLCHAIN_PATH}:${host_python_bin_dir}:$ENV{PATH} &&  make CC="${host_prefix}"-gcc RANLIB="${host_prefix}"-ranlib LD="${host_prefix}"-ld MAKEDEPPROG="${host_prefix}"-gcc PROCESSOR=${arch} -j`nproc` && make install_sw)
            if(compile_host_python)
                set(python_compile_cmd_host COMMAND set -x && cd ${python_src_path} && echo "-- compile python for host now" && make distclean || true && chmod +x ./configure && CFLAGS="-I${zlib_install_path}/include" LDFLAGS="-L${zlib_install_path}/lib" CPPFLAGS="-I${zlib_install_path}/include" ./configure --prefix=${python_host_install_dir} --with-openssl="${openssl_install_path}" --enable-shared && make -j`nproc` && make install && make distclean)
            endif()
            set(python_compile_cmd_target COMMAND set -x && cd ${python_src_path} && echo "-- compile python for target now" && export PATH=${CONFIG_TOOLCHAIN_PATH}:${host_python_bin_dir}:$ENV{PATH} && export LD_LIBRARY_PATH=${python_host_install_dir}/lib:$ENV{LD_LIBRARY_PATH} && CFLAGS="-I${zlib_install_path}/include" CPPFLAGS="-I${zlib_install_path}/include" LDFLAGS="-L${zlib_install_path}/lib" ./configure --host=${host_prefix} --build=${arch} --prefix=${python_target_install_dir} --with-build-python=${host_python_path} --with-openssl="${openssl_install_path}" --enable-shared --enable-optimizations --enable-ipv6 ac_cv_file__dev_ptmx=\"yes\" ac_cv_file__dev_ptc=\"no\" && make -j`nproc` && make install)
        endif()

        add_custom_command(OUTPUT ${python_h_path}
                    ${zlib_compile_cmd}
                    ${openssl_compile_cmd}
                    ${python_compile_cmd_host}
                    ${python_compile_cmd_target}
                    COMMENT "Building Python${python_version_str} ..."
                )
        list(APPEND ADD_FILE_DEPENDS ${python_h_path})
    endif()
else()
    list(APPEND ADD_INCLUDE ${host_include_str})
    # find libpython${py_version_str}.so
    execute_process(COMMAND python3-config --ldflags OUTPUT_VARIABLE ldflags_str RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Execute python3-config --ldflags failed: ${ldflags_str}")
    endif()
    # find all -L to get all lib search path, and remove all -L
    string(REGEX MATCHALL "-L[^ ]*" lib_search_path ${ldflags_str})
    string(REPLACE "-L" "" lib_search_path "${lib_search_path}")
    string(REPLACE "\n" "" lib_search_path "${lib_search_path}")
    string(REPLACE " " ";" lib_search_path "${lib_search_path}")
    list(REMOVE_DUPLICATES lib_search_path)
    set(version_str_major_minor "${CONFIG_PYTHON_VERSION_MAJOR}.${CONFIG_PYTHON_VERSION_MINOR}")
    # find libpython${version_str_major_minor}.so in lib search path
    foreach(lib_path ${lib_search_path})
        if(EXISTS "${lib_path}/libpython${version_str_major_minor}.so")
            message("Find libpython${version_str_major_minor}.so in ${lib_path}")
            list(APPEND ADD_DYNAMIC_LIB "${lib_path}/libpython${version_str_major_minor}.so")
            break()
        endif()
    endforeach()
endif()

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

