# Config enable component2 or not in Kconfig
################# Add include #################
list(APPEND ADD_INCLUDE "include"
                        "include/base"
)
if(PLATFORM_LINUX)
    list(APPEND ADD_PRIVATE_INCLUDE "port/linux")
    list(APPEND ADD_PRIVATE_INCLUDE "port/linux_common")
elseif(PLATFORM_MAIXCAM)
    list(APPEND ADD_PRIVATE_INCLUDE "port/maixcam")
    list(APPEND ADD_PRIVATE_INCLUDE "port/linux_common")
elseif(PLATFORM_MAIXCAM2)
    list(APPEND ADD_PRIVATE_INCLUDE "port/maixcam2")
    list(APPEND ADD_PRIVATE_INCLUDE "port/linux_common")
endif()
list(APPEND ADD_PRIVATE_INCLUDE "include_private")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib2.c"
#     )
# FILE(GLOB_RECURSE EXTRA_SRC  "src/*.c")
# FILE(GLOB EXTRA_SRC  "src/*.c")
# list(APPEND ADD_SRCS  ${EXTRA_SRC})
# aux_source_directory(src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
append_srcs_dir(ADD_SRCS "src/ByteTrack")
if(PLATFORM_LINUX)
    append_srcs_dir(ADD_SRCS "port/linux")
    append_srcs_dir(ADD_SRCS "port/linux_common")
elseif(PLATFORM_MAIXCAM)
    append_srcs_dir(ADD_SRCS "port/maixcam")
    append_srcs_dir(ADD_SRCS "port/linux_common")
elseif(PLATFORM_MAIXCAM2)
    append_srcs_dir(ADD_SRCS "port/maixcam2")
    append_srcs_dir(ADD_SRCS "port/linux_common")
endif()
# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
# set(ADD_ASM_SRCS "src/asm.S")
# list(APPEND ADD_SRCS ${ADD_ASM_SRCS})
# SET_PROPERTY(SOURCE ${ADD_ASM_SRCS} PROPERTY LANGUAGE C) # set .S  ASM file as C language
# SET_SOURCE_FILES_PROPERTIES(${ADD_ASM_SRCS} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp -D BBBBB")
###############################################


###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS basic opencv opencv_freetype websocket peripheral)
list(APPEND ADD_REQUIREMENTS zbar omv)
if(PLATFORM_LINUX)
    list(APPEND ADD_REQUIREMENTS sdl)
elseif(PLATFORM_MAIXCAM)
    list(APPEND ADD_REQUIREMENTS FFmpeg maixcam_lib RtspServer)
    if(NOT CONFIG_MAIXCAM_LIB_COMPILE_FROM_SOURCE)
        set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group vision/libvision.a -lmaixcam_lib -Wl,--end-group" PARENT_SCOPE)
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--start-group vision/libvision.a -lmaixcam_lib -Wl,--end-group" PARENT_SCOPE)
    endif()
elseif(PLATFORM_MAIXCAM2)
list(APPEND ADD_REQUIREMENTS FFmpeg maixcam_lib RtspServer)
if(NOT CONFIG_MAIXCAM_LIB_COMPILE_FROM_SOURCE)
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group vision/libvision.a -lmaixcam_lib -Wl,--end-group" PARENT_SCOPE)
# set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wl,--start-group vision/libvision.a -lmaixcam_lib -Wl,--end-group" PARENT_SCOPE)
endif()
endif()
list(APPEND ADD_REQUIREMENTS eigen voice)
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
register_component()

