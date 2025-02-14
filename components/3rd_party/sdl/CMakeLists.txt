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

# write build python version to CMAKE_BINARY_DIR/config/python_version.txt
# file(WRITE ${CMAKE_BINARY_DIR}/config/sdl_version.txt "${CONFIG_SDL_VERSION_MAJOR}.${CONFIG_SDL_VERSION_MINOR}.${CONFIG_SDL_VERSION_PATCH}")


# skip compile for local compile
if(PLATFORM_LINUX AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
    # find local sdl
    find_package(SDL2)
    list(APPEND ADD_INCLUDE ${SDL2_INCLUDE_DIRS})
    list(APPEND ADD_REQUIREMENTS ${SDL2_LIBRARIES})
    if(NOT SDL2_INCLUDE_DIRS)
        message(WARNING "can not find sdl2 locally, will download source code to compile, you can install it by 'sudo apt install libsdl2-dev'")
    endif()
endif()
if(NOT ADD_INCLUDE)
    set(sdl_version_str "${CONFIG_SDL_VERSION_MAJOR}.${CONFIG_SDL_VERSION_MINOR}.${CONFIG_SDL_VERSION_PATCH}")
    set(sdl_host_install_dir "${CMAKE_BINARY_DIR}/sdl_host_install")
    if(PLATFORM_LINUX)
        set(sdl_target_install_dir ${sdl_host_install_dir})
    else()
        set(sdl_target_install_dir "${CMAKE_BINARY_DIR}/sdl_target_install")
        if(PLATFORM_M2DOCK)
            set(arch "armv7l")
            set(host_prefix "arm-openwrt-linux")
        elseif(PLATFORM_MAIXCAM)
            set(arch "riscv64")
            string(LENGTH "${CONFIG_TOOLCHAIN_PREFIX}" toolchain_prefix_len)
            math(EXPR toolchain_prefix_len "${toolchain_prefix_len} - 1")
            string(SUBSTRING "${CONFIG_TOOLCHAIN_PREFIX}" 0 ${toolchain_prefix_len} host_prefix)
        elseif(PLATFORM_MAIXCAM2)
            set(arch "aarch64")
            string(LENGTH "${CONFIG_TOOLCHAIN_PREFIX}" toolchain_prefix_len)
            math(EXPR toolchain_prefix_len "${toolchain_prefix_len} - 1")
            string(SUBSTRING "${CONFIG_TOOLCHAIN_PREFIX}" 0 ${toolchain_prefix_len} host_prefix)
        else()
            message(FATAL_ERROR "please edit here to add support for this board")
        endif()
    endif()
    set(sdl_include_dir "${sdl_target_install_dir}/include/SDL2" "${sdl_target_install_dir}/include")
    set(sdl_h_path "${sdl_target_install_dir}/include/SDL2/SDL.h")

    list(APPEND ADD_INCLUDE ${sdl_include_dir})
    set_property(SOURCE ${sdl_include_dir} PROPERTY GENERATED 1)
    set_property(SOURCE ${sdl_h_path} PROPERTY GENERATED 1)

    if(CONFIG_LIBS_LINK_STATIC)
        set(static_shared_flag "--enable-shared=no --enable-static")
        set(sdl_lib_file "${sdl_target_install_dir}/lib/libSDL2.a")
    else()
        set(static_shared_flag "--enable-shared=yes")
        set(sdl_lib_file "${sdl_target_install_dir}/lib/libSDL2.so")
        list(APPEND ADD_DYNAMIC_LIB ${sdl_lib_file})
    endif()
    set_property(SOURCE ${sdl_lib_file} PROPERTY GENERATED 1)

    set(sdl_unzip_path "${DL_EXTRACTED_PATH}/sdl_srcs")
    set(sdl_src_path "${sdl_unzip_path}/SDL2-${sdl_version_str}")

    if(PLATFORM_LINUX)
        set(sdl_compile_cmd COMMAND set -x && cd ${sdl_src_path} && make distclean || true && chmod +x ./configure && ./configure --prefix=${sdl_target_install_dir} ${static_shared_flag} --with-pic && echo "-- compile now" && make -j`nproc` && make install)
    else()
        set(sdl_compile_cmd COMMAND set -x && cd ${sdl_src_path} && make distclean || true && chmod +x ./configure && ./configure --host=${host_prefix} --build=${arch} --prefix=${sdl_target_install_dir} ${static_shared_flag} --with-pic && make -j`nproc` && make install)
    endif()


    add_custom_command(OUTPUT ${sdl_h_path}
                ${sdl_compile_cmd}
                COMMENT "Building SDL2-${sdl_version_str} ..."
            )
    list(APPEND ADD_FILE_DEPENDS ${sdl_h_path})
endif()

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

