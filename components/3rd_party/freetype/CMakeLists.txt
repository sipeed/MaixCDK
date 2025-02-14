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
list(APPEND ADD_REQUIREMENTS brotli)
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
# set (freetype_DIR freetype/lib/cmake/freetype4)
# find_package(freetype REQUIRED)
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

set(freetype_version_str "${CONFIG_FREETYPE_VERSION_MAJOR}.${CONFIG_FREETYPE_VERSION_MINOR}.${CONFIG_FREETYPE_VERSION_PATCH}")

# write build python version to CMAKE_BINARY_DIR/config/python_version.txt
# file(WRITE ${CMAKE_BINARY_DIR}/config/freetype_version.txt "${freetype_version_str}")

set(freetype_install_dir "${CMAKE_BINARY_DIR}/freetype_install")
set(freetype_include_dir "${freetype_install_dir}/include/freetype2" "${freetype_install_dir}/include")
set(freetype_h_path "${freetype_install_dir}/include/freetype2/freetype/freetype.h")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(freetype_lib_file "${freetype_install_dir}/lib/libfreetyped.a")
else()
    set(freetype_lib_file "${freetype_install_dir}/lib/libfreetype.a")
endif()
set(freetype_build_dir "${CMAKE_BINARY_DIR}/freetype_build")

list(APPEND ADD_INCLUDE ${freetype_include_dir})
list(APPEND ADD_STATIC_LIB ${freetype_lib_file})
set_property(SOURCE ${freetype_include_dir} PROPERTY GENERATED 1)
set_property(SOURCE ${freetype_h_path} PROPERTY GENERATED 1)
set_property(SOURCE ${freetype_lib_file} PROPERTY GENERATED 1)

set(freetype_unzip_path "${DL_EXTRACTED_PATH}/freetype_srcs")
set(freetype_src_path "${freetype_unzip_path}/freetype-${freetype_version_str}")
if(PLATFORM_M2DOCK)
    set(freetype_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_m2dock.cmake)
elseif(PLATFORM_MAIXCAM OR PLATFORM_MAIXCAM2)
    set(freetype_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux_cross.cmake)
elseif(PLATFORM_LINUX)
    set(freetype_toolchain_file ${CMAKE_CURRENT_LIST_DIR}/toolchain_linux.cmake)
else()
    message(FATAL_ERROR "No freetype toolchain config for this board, please edit to add freetype support for this board")
endif()
set(freetype_compile_cmd COMMAND mkdir -p ${freetype_build_dir} && cd ${freetype_build_dir} && SDK_PATH=${SDK_PATH} TOOLCHAIN_PATH=${CONFIG_TOOLCHAIN_PATH} TOOLCHAIN_PREFIX=${CONFIG_TOOLCHAIN_PREFIX} cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_PREFIX_PATH="${CMAKE_BINARY_DIR}/brotli_install/lib/pkgconfig" -DCMAKE_TOOLCHAIN_FILE=${freetype_toolchain_file} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${freetype_install_dir} -DFT_DISABLE_ZLIB=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_BROTLI=ON ${freetype_src_path} && make -j`nproc` install)


add_custom_command(OUTPUT ${freetype_h_path}
            ${freetype_compile_cmd}
            COMMENT "Building freetype-${freetype_version_str} ..."
            DEPENDS brotli
        )
list(APPEND ADD_FILE_DEPENDS ${freetype_h_path})

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

