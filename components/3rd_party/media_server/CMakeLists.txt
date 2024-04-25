set(media_server_version_str "${CONFIG_MEDIA_SERVER_VERSION_MAJOR}.${CONFIG_MEDIA_SERVER_VERSION_MINOR}.${CONFIG_MEDIA_SERVER_VERSION_PATCH}")
set(media_server_filename "media_server-${media_server_version_str}.zip")
set(media_server_unzip_path "${DL_EXTRACTED_PATH}/media_server")
set(src_path "${media_server_unzip_path}/media_server-${media_server_version_str}")
if(${media_server_version_str} VERSION_EQUAL "1.0.0")
    set(media_server_url "https://files.catbox.moe/c7rklp.zip")
    set(media_server_file_sha256sum "dfe405cedbf3969fbf1f252634e7b1c71e8bb3e2d0857db2fe7e70189474ecd1")
elseif(${media_server_version_str} VERSION_EQUAL "1.0.1")
    set(media_server_url "https://files.catbox.moe/acqhv1.zip")
    set(media_server_file_sha256sum "ce06dc3d03b6036165956e60afd5eec5bfd37e7746e4b427f2099732478ecc22")
endif()
############### Add include ###################
set(media_server_private_include_dir
                    "${src_path}/librtsp/source/server"
                    "${src_path}/sdk/include"
                    "${src_path}/sdk/libhttp/include"
                    "${src_path}/librtp/include"
                    "${src_path}/libmpeg/include"
                    "${src_path}/libflv/include"
                    "${src_path}/libmkv/include"
                    "${src_path}/avcodec/avcodec/include"
                    "${src_path}/avcodec/avbsf/include"
                    "${src_path}/libmov/include"
                    "${src_path}/include"
                    "${src_path}/media")
# list(APPEND ADD_PRIVATE_INCLUDE ${media_server_private_include_dir})

set(media_server_include_dir "${src_path}"
                            ".")

list(APPEND ADD_INCLUDE ${media_server_include_dir}
${media_server_private_include_dir})
set_property(SOURCE ${media_server_include_dir} PROPERTY GENERATED 1)
set_property(SOURCE ${media_server_private_include_dir} PROPERTY GENERATED 1)

###############################################

############ Add source files #################
list(APPEND ADD_SRCS
)

append_srcs_dir(ADD_SRCS "${src_path}"
                         "${src_path}/media")       # append source file in src dir to var ADD_SRCS
set_property(SOURCE ${ADD_SRCS} PROPERTY GENERATED 1)
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
list(APPEND ADD_STATIC_LIB "${src_path}/libs/librtsp.a"
                            "${src_path}/libs/libhttp.a"
                            "${src_path}/libs/libflv.a"
                            "${src_path}/libs/libmov.a"
                            "${src_path}/libs/librtp.a"
                            "${src_path}/libs/libmpeg.a"
                            "${src_path}/libs/libsdk.a"
                            "${src_path}/libs/libavcodec.a"
                            "${src_path}/libs/libavbsf.a")
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
#### and components denpend on this component
list(APPEND ADD_DEFINITIONS -D__ERROR__=00*10000000+__LINE__*1000)

###############################################
list(APPEND ADD_FILE_DOWNLOADS "{
    'url': '${media_server_url}',
    'urls': [],
    'sites': [],
    'sha256sum': '${media_server_file_sha256sum}',
    'filename': '${media_server_filename}',
    'path': 'media_server',
    'check_files': ['media_server-${media_server_version_str}']
    }"
)

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

