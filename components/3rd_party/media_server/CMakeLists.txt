set(media_server_version_str "${CONFIG_MEDIA_SERVER_VERSION_MAJOR}.${CONFIG_MEDIA_SERVER_VERSION_MINOR}.${CONFIG_MEDIA_SERVER_VERSION_PATCH}")
set(media_server_unzip_path "${DL_EXTRACTED_PATH}/media_server")
set(src_path "${media_server_unzip_path}/media_server-${media_server_version_str}")

############### Add include ###################
set(media_server_private_include_dir    "${src_path}/include/avcodec/avbsf/include"
                                        "${src_path}/include/avcodec/avcodec/include"
                                        "${src_path}/include/avcodec/h264/include"
                                        "${src_path}/include/avcodec/h265/include"

                                        "${src_path}/include/sdk/include"
                                        "${src_path}/include/sdk/libaio/include"
                                        "${src_path}/include/sdk/libhttp/include"
                                        "${src_path}/include/sdk/libice/include"

                                        "${src_path}/include/media-server/libdash/include"
                                        "${src_path}/include/media-server/libice/include"
                                        "${src_path}/include/media-server/libflv/include"
                                        "${src_path}/include/media-server/libhls/include"
                                        "${src_path}/include/media-server/libmkv/include"
                                        "${src_path}/include/media-server/libmov/include"
                                        "${src_path}/include/media-server/libmpeg/include"
                                        "${src_path}/include/media-server/librtmp/include"
                                        "${src_path}/include/media-server/librtp/include"
                                        "${src_path}/include/media-server/librtsp/include"
                                        "${src_path}/include/media-server/libsip/include")
set(media_server_include_dir            "inc" "${src_path}")
list(APPEND ADD_INCLUDE ${media_server_include_dir} ${media_server_private_include_dir})
set_property(SOURCE ${media_server_include_dir} PROPERTY GENERATED 1)
set_property(SOURCE ${media_server_private_include_dir} PROPERTY GENERATED 1)
###############################################

############ Add source files #################
aux_source_directory("src" ADD_SRCS)
# append_srcs_dir(ADD_SRCS ".")       # append source file in src dir to var ADD_SRCS
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
# set (media_server_DIR media_server/lib/cmake/media_server4)
# find_package(media_server REQUIRED)
###############################################

############ Add static libs ##################
list(APPEND ADD_STATIC_LIB

                                        "${src_path}/lib/libdash.a"
                                        "${src_path}/lib/libhls.a"
                                        "${src_path}/lib/libmkv.a"
                                        "${src_path}/lib/libmov.a"
                                        "${src_path}/lib/libmpeg.a"
                                        "${src_path}/lib/librtmp.a"
                                        "${src_path}/lib/libflv.a"
                                        "${src_path}/lib/librtp.a"
                                        "${src_path}/lib/librtsp.a"
                                        "${src_path}/lib/libsip.a"

                                        "${src_path}/lib/libavbsf.a"
                                        "${src_path}/lib/libavcodec.a"
                                        "${src_path}/lib/libh264.a"
                                        "${src_path}/lib/libh265.a"

                                        "${src_path}/lib/libhttp.a"
                                        "${src_path}/lib/libice.a"
                                        "${src_path}/lib/libsdk.a"


)
set_property(SOURCE ${ADD_STATIC_LIB} PROPERTY GENERATED 1)
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
list(APPEND ADD_DEFINITIONS -D__ERROR__=00*10000000+__LINE__*1000 -DOS_LINUX)

###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

