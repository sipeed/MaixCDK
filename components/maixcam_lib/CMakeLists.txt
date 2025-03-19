
list(APPEND ADD_REQUIREMENTS basic ini)
list(APPEND ADD_INCLUDE "include")

if(PLATFORM_MAIXCAM)
    message(STATUS "PLATFORM_MAIXCAM")
    # sophgo-middleware
    set(middleware_src_path "${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/sophgo-middleware/sophgo-middleware")

    if(CONFIG_MAIXCAM_LIB_COMPILE_FROM_SOURCE)
        set(source_dir "maixcdk_maixcam_lib")
        append_srcs_dir(ADD_SRCS "${source_dir}")
        # nn
        list(APPEND ADD_INCLUDE "../nn/include")
        list(APPEND ADD_INCLUDE "../vision/include")
        append_srcs_dir(ADD_SRCS "${source_dir}/nn")
        list(APPEND ADD_REQUIREMENTS cvi_tpu)

        # media_server
        append_srcs_dir(ADD_SRCS "${source_dir}/media_server")
        list(APPEND ADD_REQUIREMENTS media_server)

        # middleware
        list(APPEND ADD_SRCS        "${source_dir}/middleware/sophgo_middleware.cpp")
        set_property(SOURCE ${middleware_src_dir} PROPERTY GENERATED 1)

        list(APPEND ADD_REQUIREMENTS sophgo-middleware)
        list(APPEND ADD_DEFINITIONS_PRIVATE -Wl,--exclude-libs,ALL)

        register_component(DYNAMIC)
    else()
        list(APPEND ADD_DYNAMIC_LIB "lib/libmaixcam_lib.so")
        list(APPEND ADD_DIST_LIB_IGNORE "lib/libmaixcam_lib.so")
        list(APPEND ADD_REQUIREMENTS sophgo-middleware)

        register_component()
    endif()
elseif(PLATFORM_MAIXCAM2)
    # ax630c
    list(APPEND ADD_REQUIREMENTS ax620e_msp)

    list(APPEND ADD_INCLUDE "../vision/include")
    set(source_dir "maixcdk_maixcam_lib/maixcam2")

    if(CONFIG_MAIXCAM_LIB_COMPILE_FROM_SOURCE)
        append_srcs_dir(ADD_SRCS "${source_dir}")
        register_component(DYNAMIC)
    else()
        list(APPEND ADD_DYNAMIC_LIB "lib/libmaixcam2_lib.so")
        list(APPEND ADD_DIST_LIB_IGNORE "lib/libmaixcam2_lib.so")
        register_component()
    endif()
endif()

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
# list(APPEND ADD_REQUIREMENTS basic ini)
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

