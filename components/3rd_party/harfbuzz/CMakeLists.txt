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
# set (harfbuzz_DIR harfbuzz/lib/cmake/harfbuzz4)
# find_package(harfbuzz REQUIRED)
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

list(APPEND ADD_REQUIREMENTS freetype yaml)

set(harfbuzz_version_str "${CONFIG_HARFBUZZ_VERSION_MAJOR}.${CONFIG_HARFBUZZ_VERSION_MINOR}.${CONFIG_HARFBUZZ_VERSION_PATCH}")

# write build python version to CMAKE_BINARY_DIR/config/python_version.txt
# file(WRITE ${CMAKE_BINARY_DIR}/config/harfbuzz_version.txt "${harfbuzz_version_str}")

if(PLATFORM_MAIXCAM AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
    if(CONFIG_TOOLCHAIN_PATH MATCHES "musl")
        set(harfbuzz_lib_dir "${DL_EXTRACTED_PATH}/harfbuzz/harfbuzz_maixcam_musl_v8.2.1")
        set(harfbuzz_include_dir "${harfbuzz_lib_dir}/include/harfbuzz" "${harfbuzz_lib_dir}/include")
        set(harfbuzz_lib_file "${harfbuzz_lib_dir}/lib/libharfbuzz.a")
        set_property(SOURCE ${harfbuzz_lib_file} PROPERTY GENERATED 1)
        set_property(SOURCE ${harfbuzz_include_dir} PROPERTY GENERATED 1)
        list(APPEND ADD_INCLUDE ${harfbuzz_include_dir})
        list(APPEND ADD_STATIC_LIB ${harfbuzz_lib_file})
    endif()
elseif(PLATFORM_MAIXCAM2 AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
    set(harfbuzz_lib_dir "${DL_EXTRACTED_PATH}/harfbuzz/harfbuzz_maixcam2_glibc_v8.2.1")
    set(harfbuzz_include_dir "${harfbuzz_lib_dir}/include/harfbuzz" "${harfbuzz_lib_dir}/include")
    set(harfbuzz_lib_file "${harfbuzz_lib_dir}/lib/libharfbuzz.a")
    set_property(SOURCE ${harfbuzz_lib_file} PROPERTY GENERATED 1)
    set_property(SOURCE ${harfbuzz_include_dir} PROPERTY GENERATED 1)
    list(APPEND ADD_INCLUDE ${harfbuzz_include_dir})
    list(APPEND ADD_STATIC_LIB ${harfbuzz_lib_file})
endif()

if(NOT ADD_INCLUDE)
    set(harfbuzz_build_dir "${CMAKE_BINARY_DIR}/harfbuzz_build")
    set(harfbuzz_install_dir "${CMAKE_BINARY_DIR}/harfbuzz_install")
    set(harfbuzz_include_dir "${harfbuzz_install_dir}/include/harfbuzz" "${harfbuzz_install_dir}/include")
    set(harfbuzz_h_path "${harfbuzz_install_dir}/include/harfbuzz/harfbuzz.h")
    set(harfbuzz_lib_file "${harfbuzz_install_dir}/lib/libharfbuzz.a")

    list(APPEND ADD_INCLUDE ${harfbuzz_include_dir})
    list(APPEND ADD_STATIC_LIB ${harfbuzz_lib_file})
    set_property(SOURCE ${harfbuzz_include_dir} PROPERTY GENERATED 1)
    set_property(SOURCE ${harfbuzz_h_path} PROPERTY GENERATED 1)
    set_property(SOURCE ${harfbuzz_lib_file} PROPERTY GENERATED 1)

    set(harfbuzz_unzip_path "${DL_EXTRACTED_PATH}/harfbuzz_srcs")
    set(harfbuzz_src_path "${harfbuzz_unzip_path}/harfbuzz-${harfbuzz_version_str}")

    if(PLATFORM_M2DOCK)
        set(harfbuzz_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_m2dock.cmake)
    elseif(PLATFORM_MAIXCAM OR PLATFORM_MAIXCAM2)
        set(harfbuzz_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux_cross.cmake)
    elseif(PLATFORM_LINUX)
        set(harfbuzz_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux.cmake)
    else()
        message(FATAL_ERROR "No harfbuzz toolchain config for this board, please edit to add harfbuzz support for this board")
    endif()
    set(harfbuzz_compile_cmd COMMAND mkdir -p ${harfbuzz_build_dir} && cd ${harfbuzz_build_dir} && VERSION=${harfbuzz_version_str} MAIXCDK_BUILD_DIR=${CMAKE_BINARY_DIR} SDK_PATH=${SDK_PATH} TOOLCHAIN_PATH=${CONFIG_TOOLCHAIN_PATH} TOOLCHAIN_PREFIX=${CONFIG_TOOLCHAIN_PREFIX} cmake -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY -DHB_HAVE_FREETYPE=ON -DCMAKE_FIND_ROOT_PATH="${CMAKE_BINARY_DIR}/freetype_install" -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_TOOLCHAIN_FILE=${harfbuzz_toolchain_file} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${harfbuzz_install_dir} ${harfbuzz_src_path} && make -j`nproc` install)


    add_custom_command(OUTPUT ${harfbuzz_h_path}
                ${harfbuzz_compile_cmd}
                COMMENT "Building harfbuzz-${harfbuzz_version_str} ..."
                DEPENDS freetype
            )
    list(APPEND ADD_FILE_DEPENDS ${harfbuzz_h_path})
endif()

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

