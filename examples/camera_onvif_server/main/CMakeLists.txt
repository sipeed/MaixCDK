set(WSSE_ON 1)
set(DAEMON_NAME "onvif_server")
# set(DAEMON_MAJOR_VERSION 2)
# set(DAEMON_MINOR_VERSION 0)
# set(DAEMON_PATCH_VERSION 0)
set(DAEMON_PID_FILE_NAME  ${DAEMON_NAME}.pid)
set(DAEMON_LOG_FILE_NAME  ${DAEMON_NAME}.log)
set(DAEMON_NO_CHDIR       1)
set(DAEMON_NO_CLOSE_STDIO 1)
set(gsoap_version_str "${CONFIG_GSOAP_VERSION_MAJOR}.${CONFIG_GSOAP_VERSION_MINOR}")
set(GSOAP_INSTALL ${DL_EXTRACTED_PATH}/gsoap_srcs/gsoap-${gsoap_version_str})
set(ONVIF_SERVD_INSTALL ${CMAKE_CURRENT_LIST_DIR}/../onvif_srvd)
set(GENERATED_INSTALL ${CMAKE_CURRENT_LIST_DIR}/../generated)

############### Add include ###################
list(APPEND ADD_INCLUDE "include"
    )

list(APPEND ADD_PRIVATE_INCLUDE     ${ONVIF_SERVD_INSTALL}/src/
                                    )

list(APPEND ADD_PRIVATE_INCLUDE     ${GSOAP_INSTALL}/gsoap
                                    ${GSOAP_INSTALL}/gsoap/custom
                                    )
list(APPEND ADD_PRIVATE_INCLUDE     ${GENERATED_INSTALL})

list(APPEND ADD_PRIVATE_INCLUDE     ${GSOAP_INSTALL}/gsoap/plugin)
###############################################

############ Add source files #################
append_srcs_dir(ADD_SRCS "src")       # append source file in src dir to var ADD_SRCS

# onvif_srvd
append_srcs_dir(ADD_SRCS     ${ONVIF_SERVD_INSTALL}/src)

list(REMOVE_ITEM ADD_SRCS    ${ONVIF_SERVD_INSTALL}/src/onvif_srvd.cpp)

# soap
append_srcs_dir(ADD_SRCS     ${GENERATED_INSTALL})
list(APPEND ADD_SRCS        ${GSOAP_INSTALL}/gsoap/custom/duration.c)
list(APPEND ADD_SRCS        ${GSOAP_INSTALL}/gsoap/dom.cpp
                            ${GSOAP_INSTALL}/gsoap/stdsoap2.cpp)
if (${WSSE_ON})
    list(APPEND ADD_SRC     ${GSOAP_INSTALL}/gsoap/plugin/wsseapi.c
                            ${GSOAP_INSTALL}/gsoap/plugin/mecevp.c
                            ${GSOAP_INSTALL}/gsoap/plugin/smdevp.c
                            ${GSOAP_INSTALL}/gsoap/plugin/wsaapi.c
    )
endif()
SET_PROPERTY(SOURCE ${ADD_SRCS} PROPERTY LANGUAGE CXX)

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
list(APPEND ADD_REQUIREMENTS basic openssl nn vision)
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

#### Add compile option for this component ####
#### Just for this component, won't affect other 
#### modules, including component that depend 
#### on this component
list(APPEND ADD_DEFINITIONS_PRIVATE
-DDAEMON_NAME="${DAEMON_NAME}"
# -DDAEMON_MAJOR_VERSION=${DAEMON_MAJOR_VERSION}
# -DDAEMON_MINOR_VERSION=${DAEMON_MINOR_VERSION}
# -DDAEMON_PATCH_VERSION=${DAEMON_PATCH_VERSION}
-DDAEMON_PID_FILE_NAME="${DAEMON_PID_FILE_NAME}"
-DDAEMON_LOG_FILE_NAME="${DAEMON_LOG_FILE_NAME}"
-DDAEMON_NO_CHDIR=${DAEMON_NO_CHDIR}
-DDAEMON_NO_CLOSE_STDIO=${DAEMON_NO_CLOSE_STDIO}
)
if (${WSSE_ON})
list(APPEND ADD_DEFINITIONS_PRIVATE -DWITH_OPENSSL -DWITH_DOM)
endif()

#### Add compile option for this component
#### and components depend on this component
# list(APPEND ADD_DEFINITIONS -DAAAAA222=1
#                             -DAAAAA333=1)
###############################################

############ Add static libs ##################
#### Update parent's variables like CMAKE_C_LINK_FLAGS
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
###############################################

######### Add files need to download #########
# list(APPEND ADD_FILE_DOWNLOADS "{
# 'url': 'https://*****/abcde.tar.xz',
# 'urls': [],  # backup urls, if url failed, will try urls
# 'sites': [], # download site, user can manually download file and put it into dl_path
# 'sha256sum': '',
# 'filename': 'abcde.tar.xz',
# 'path': 'toolchains/xxxxx',
# 'check_files': []
# }"
# )
#
# then extracted file in ${DL_EXTRACTED_PATH}/toolchains/xxxxx,
# you can directly use then, for example use it in add_custom_command
##############################################
# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()
