set(RtspServer_version_str "${CONFIG_RTSP_SERVER_VERSION_MAJOR}.${CONFIG_RTSP_SERVER_VERSION_MINOR}.${CONFIG_RTSP_SERVER_VERSION_PATCH}")
set(RtspServer_unzip_path "${DL_EXTRACTED_PATH}/RtspServer_srcs")
set(src_path "${RtspServer_unzip_path}/RtspServer")
############### Add include ###################
list(APPEND ADD_INCLUDE         ".")
set(RtspServer_include_dir      "${src_path}/src"
                                "${src_path}/src/3rdpart")
list(APPEND ADD_INCLUDE ${RtspServer_include_dir})
set_property(SOURCE ${RtspServer_include_dir} PROPERTY GENERATED 1)
###############################################

############ Add source files #################
append_srcs_dir(ADD_SRCS        "${src_path}/src/net"
                                "${src_path}/src/xop")
# aux_source_directory("src" ADD_SRCS)
# set_property(SOURCE ${ADD_SRCS} PROPERTY GENERATED 1)

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
# list(APPEND ADD_REQUIREMENTS)
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
# set (RtspServer_DIR RtspServer/lib/cmake/RtspServer4)
# find_package(RtspServer REQUIRED)
###############################################

############ Add static libs ##################
# set(RtspServer_static_lib_file
#                             ${src_path}/lib/libavutil.a
#                             ${src_path}/lib/libavformat.a
#                             ${src_path}/lib/libavcodec.a
#                             ${src_path}/lib/libavfilter.a
#                                 )
# list(APPEND ADD_STATIC_LIB ${RtspServer_static_lib_file})
# set_property(SOURCE ${RtspServer_static_lib_file} PROPERTY GENERATED 1)
# message(STATUS "===========================RtspServer_static_lib_file: ${RtspServer_static_lib_file}")
###############################################

############ Add dynamic libs ##################

###############################################

#### Add compile option for this component ####
#### Just for this component, won't affect other
#### modules, including component that depend
#### on this component
# list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

#### Add compile option for this component
#### Add components that depend on this component
# list(APPEND ADD_DEFINITIONS -D__ERROR__=00*10000000+__LINE__*1000 -DOS_LINUX)

###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()
