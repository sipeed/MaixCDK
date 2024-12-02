
# if include exist, then add it to ADD_INCLUDE for local debug
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/cvi_tpu_lib)
    list(APPEND ADD_INCLUDE "cvi_tpu_lib/include")
    # if toolchain path string contain musl, then use musl libc
    if(${CONFIG_TOOLCHAIN_PATH} MATCHES "musl")
        message(STATUS "cvi_tpu lib use musl libc")
        list(APPEND ADD_STATIC_LIB  "cvi_tpu_lib/lib_musl/libcvimath-static.a"
                                    "cvi_tpu_lib/lib_musl/libcviruntime-static.a"
                                    "cvi_tpu_lib/lib_musl/libcvikernel-static.a"
            )
    else()
        message(STATUS "cvi_tpu lib use glibc")
        list(APPEND ADD_STATIC_LIB "cvi_tpu_lib/lib_glibc/libcvimath-static.a"
                                "cvi_tpu_lib/lib_glibc/libcviruntime-static.a"
                                "cvi_tpu_lib/lib_glibc/libcvikernel-static.a"
        )
    endif()
elseif(PLATFORM_MAIXCAM AND NOT CONFIG_COMPONENTS_COMPILE_FROM_SOURCE)
    if(CONFIG_TOOLCHAIN_PATH MATCHES "musl" OR CONFIG_TOOLCHAIN_PATH MATCHES "glibc")
        set(cvi_tpu_lib_parent_dir "${DL_EXTRACTED_PATH}/cvi_tpu")
        file(GLOB ALL_SUBDIRS LIST_DIRECTORIES true "${cvi_tpu_lib_parent_dir}/*")
        if(ALL_SUBDIRS)
            list(GET ALL_SUBDIRS 0 FIRST_SUBDIR)
            get_filename_component(FIRST_SUBDIR_NAME "${FIRST_SUBDIR}" NAME)
            set(cvi_tpu_lib_dir "${cvi_tpu_lib_parent_dir}/${FIRST_SUBDIR_NAME}")
        else()
            message(WARNING "No subdirectories found in ${DIRECTORY_PATH}")
        endif()
        set(harfbuzz_include_dir "${cvi_tpu_lib_dir}/include")
        set_property(SOURCE ${harfbuzz_include_dir} PROPERTY GENERATED 1)
    endif()
    if(NOT cvi_tpu_lib_dir)
        message(FATAL_ERROR "cvi_tpu not support this toolchain")
    endif()
    if(CONFIG_TOOLCHAIN_PATH MATCHES "musl")
        set(harfbuzz_lib_file "${cvi_tpu_lib_dir}/lib_musl/libcvimath-static.a"
                              "${cvi_tpu_lib_dir}/lib_musl/libcviruntime-static.a"
                              "${cvi_tpu_lib_dir}/lib_musl/libcvikernel-static.a"
        )
    elseif(CONFIG_TOOLCHAIN_PATH MATCHES "glibc")
        set(harfbuzz_lib_file "${cvi_tpu_lib_dir}/lib_glibc/libcvimath-static.a"
                            "${cvi_tpu_lib_dir}/lib_glibc/libcviruntime-static.a"
                            "${cvi_tpu_lib_dir}/lib_glibc/libcvikernel-static.a"
        )
    endif()
    if(harfbuzz_lib_file)
        set_property(SOURCE ${harfbuzz_lib_file} PROPERTY GENERATED 1)
        list(APPEND ADD_INCLUDE ${harfbuzz_include_dir})
        list(APPEND ADD_STATIC_LIB ${harfbuzz_lib_file})
    endif()
endif()
if(PLATFORM_MAIXCAM AND NOT ADD_INCLUDE)
    message(FATAL_ERROR "cvi_tpu_lib not found please check")
endif()

register_component()



# Config enable component2 or not in Kconfig
################# Add include #################
# list(APPEND ADD_INCLUDE "include"
#     )
# list(APPEND ADD_PRIVATE_INCLUDE "include_private")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib2.c"
#     )
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

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
# register_component()

