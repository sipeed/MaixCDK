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
# set (brotli_DIR brotli/lib/cmake/brotli4)
# find_package(brotli REQUIRED)
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



set(version_str "${CONFIG_BROTLI_VERSION_MAJOR}.${CONFIG_BROTLI_VERSION_MINOR}.${CONFIG_BROTLI_VERSION_PATCH}")

# write build python version to CMAKE_BINARY_DIR/config/python_version.txt
# file(WRITE ${CMAKE_BINARY_DIR}/config/brotli_version.txt "${version_str}")

set(install_dir "${CMAKE_BINARY_DIR}/brotli_install")
set(include_dir "${install_dir}/include/brotli" "${install_dir}/include")
set(h_path "${install_dir}/include/brotli/decode.h")
set(lib_file "${install_dir}/lib/libbrotlidec.a" "${install_dir}/lib/libbrotlicommon.a" "${install_dir}/lib/libbrotlienc.a")
set(build_dir "${CMAKE_BINARY_DIR}/brotli_build")

list(APPEND ADD_INCLUDE ${include_dir})
list(APPEND ADD_STATIC_LIB ${lib_file})
set_property(SOURCE ${include_dir} PROPERTY GENERATED 1)
set_property(SOURCE ${h_path} PROPERTY GENERATED 1)
set_property(SOURCE ${lib_file} PROPERTY GENERATED 1)

set(brotli_unzip_path "${DL_EXTRACTED_PATH}/brotli_srcs")
set(brotli_src_path "${brotli_unzip_path}/google-brotli-${version_str}")
if(version_str EQUAL "1.1.0")
    set(brotli_src_path "${brotli_unzip_path}/google-brotli-ed738e8")
else()
    message(FATAL_ERROR "No brotli version ${version_str} support, please edit to add brotli support for this version")
endif()


if(PLATFORM_M2DOCK)
    set(brotli_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_m2dock.cmake)
elseif(PLATFORM_MAIXCAM OR PLATFORM_MAIXCAM2)
    set(brotli_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux_cross.cmake)
elseif(PLATFORM_LINUX)
    set(brotli_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux.cmake)
else()
    message(FATAL_ERROR "No brotli toolchain config for this board, please edit to add brotli support for this board")
endif()
set(brotli_compile_cmd COMMAND mkdir -p ${build_dir} && cd ${build_dir} && SDK_PATH=${SDK_PATH} TOOLCHAIN_PATH=${CONFIG_TOOLCHAIN_PATH} TOOLCHAIN_PREFIX=${CONFIG_TOOLCHAIN_PREFIX} cmake -DBROTLI_DISABLE_TESTS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_TOOLCHAIN_FILE=${brotli_toolchain_file} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${install_dir} ${brotli_src_path} && make -j`nproc` install)


add_custom_command(OUTPUT ${h_path}
            ${brotli_compile_cmd}
            COMMENT "Building brotli-${version_str} ..."
        )
list(APPEND ADD_FILE_DEPENDS ${h_path})

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

