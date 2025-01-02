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

if(PLATFORM_LINUX)
    set(compile_from_src 0)
else()
    set(compile_from_src 1)
endif()
if(CONFIG_OPENSSL_COMPILE_FROM_SOURCE)
    set(compile_from_src 1)
endif()

if(compile_from_src)
    if(NOT ADD_INCLUDE) # compile from source code
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpmachine OUTPUT_VARIABLE arch)
        string(REGEX REPLACE "([^-]+)-.*" "\\1" arch "${arch}")
        if(PLATFORM_MAIXCAM)
            string(LENGTH "${CONFIG_TOOLCHAIN_PREFIX}" toolchain_prefix_len)
            math(EXPR toolchain_prefix_len "${toolchain_prefix_len} - 1")
            string(SUBSTRING "${CONFIG_TOOLCHAIN_PREFIX}" 0 ${toolchain_prefix_len} host_prefix)
            set(host_prefix "${host_prefix}-")
        elseif(PLATFORM_LINUX)
            set(host_prefix "")
        else()
            message(FATAL_ERROR "openssle compile not support this platform yet, please edit CMakeLists.txt to support")
        endif()

        set(target_install_dir "${CMAKE_BINARY_DIR}/openssl_target_install")
        set(target_include_dir "${target_install_dir}/include")
        set(target_lib_file "${target_install_dir}/lib/libssl.so.3" "${target_install_dir}/lib/libcrypto.so.3")
        set(target_h_path "${target_install_dir}/include/openssl/ssl.h")
        list(APPEND ADD_INCLUDE ${target_include_dir})
        list(APPEND ADD_DYNAMIC_LIB ${target_lib_file})
        set_property(SOURCE ${target_include_dir} PROPERTY GENERATED 1)
        set_property(SOURCE ${target_h_path} PROPERTY GENERATED 1)
        set_property(SOURCE ${target_lib_file} PROPERTY GENERATED 1)

        set(openssl_src_path "${DL_EXTRACTED_PATH}/openssl/openssl-3.0.12")
        set(openssl_install_path "${target_install_dir}")
        set(openssl_compile_cmd COMMAND set -x && cd ${openssl_src_path} && echo "-- compile openssl now" && make distclean || true && chmod +x ./Configure && ./Configure linux-generic32 shared  -DL_ENDIAN --openssldir=${openssl_install_path} --prefix=${openssl_install_path} && export PATH=${CONFIG_TOOLCHAIN_PATH}:$ENV{PATH} &&  make CC="${host_prefix}"gcc RANLIB="${host_prefix}"ranlib LD="${host_prefix}"ld MAKEDEPPROG="${host_prefix}"gcc PROCESSOR=${arch} -j`nproc` && make install_sw)

        add_custom_command(OUTPUT ${target_h_path}
                    ${openssl_compile_cmd}
                    COMMENT "Building openssl ..."
                )
        list(APPEND ADD_FILE_DEPENDS ${target_h_path})
    endif()
else()
    list(APPEND ADD_REQUIREMENTS openssl)
endif()

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

