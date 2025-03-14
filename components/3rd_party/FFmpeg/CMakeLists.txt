set(ffmpeg_version_str "${CONFIG_FFMPEG_VERSION_MAJOR}.${CONFIG_FFMPEG_VERSION_MINOR}.${CONFIG_FFMPEG_VERSION_PATCH}")
set(ffmpeg_unzip_path "${DL_EXTRACTED_PATH}/ffmpeg_srcs")
if(PLATFORM_MAIXCAM2)
    set(src_path "${ffmpeg_unzip_path}/ffmpeg_ax630c_libs")
else()
    set(src_path "${ffmpeg_unzip_path}/ffmpeg")
endif()
############### Add include ###################
set(ffmpeg_include_dir          "${src_path}/include"
                                "."
                                )
list(APPEND ADD_INCLUDE ${ffmpeg_include_dir})
set_property(SOURCE ${ffmpeg_include_dir} PROPERTY GENERATED 1)
###############################################

############ Add source files #################
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
# set (ffmpeg_DIR ffmpeg/lib/cmake/ffmpeg4)
# find_package(ffmpeg REQUIRED)
###############################################

############ Add static libs ##################
# set(ffmpeg_static_lib_file
#                             ${src_path}/lib/libavutil.a
#                             ${src_path}/lib/libavformat.a
#                             ${src_path}/lib/libavcodec.a
#                             ${src_path}/lib/libavfilter.a
#                                 )
# list(APPEND ADD_STATIC_LIB ${ffmpeg_static_lib_file})
# set_property(SOURCE ${ffmpeg_static_lib_file} PROPERTY GENERATED 1)
# message(STATUS "===========================ffmpeg_static_lib_file: ${ffmpeg_static_lib_file}")
###############################################

############ Add dynamic libs ##################
if (PLATFORM_MAIXCAM)
    set(ffmpeg_dynamic_lib_file ${src_path}/lib/libavcodec.so
                                ${src_path}/lib/libavdevice.so
                                ${src_path}/lib/libavfilter.so
                                ${src_path}/lib/libavformat.so
                                ${src_path}/lib/libavresample.so
                                ${src_path}/lib/libavutil.so
                                ${src_path}/lib/libpostproc.so
                                ${src_path}/lib/libswresample.so
                                ${src_path}/lib/libswscale.so
                                    )
    list(APPEND ADD_DYNAMIC_LIB ${ffmpeg_dynamic_lib_file})
    set_property(SOURCE ${ffmpeg_dynamic_lib_file} PROPERTY GENERATED 1)
elseif(PLATFORM_MAIXCAM2)
    set(ffmpeg_dynamic_lib_file ${src_path}/lib/libavcodec.so
                                ${src_path}/lib/libavdevice.so
                                ${src_path}/lib/libavfilter.so
                                ${src_path}/lib/libavformat.so
                                # ${src_path}/lib/libavresample.so.4
                                ${src_path}/lib/libavutil.so
                                ${src_path}/lib/libswresample.so
                                ${src_path}/lib/libswscale.so)
    list(APPEND ADD_DYNAMIC_LIB ${ffmpeg_dynamic_lib_file})
    set_property(SOURCE ${ffmpeg_dynamic_lib_file} PROPERTY GENERATED 1)
    list(APPEND ADD_DIST_LIB_IGNORE ${ffmpeg_dynamic_lib_file})
else()
    set(ffmpeg_dynamic_lib_file ${src_path}/lib/libavcodec.so
                                ${src_path}/lib/libavdevice.so
                                ${src_path}/lib/libavfilter.so
                                ${src_path}/lib/libavformat.so
                                ${src_path}/lib/libavresample.so
                                ${src_path}/lib/libavutil.so
                                ${src_path}/lib/libpostproc.so
                                ${src_path}/lib/libswresample.so
                                ${src_path}/lib/libswscale.so)
    list(APPEND ADD_DYNAMIC_LIB ${ffmpeg_dynamic_lib_file})
    set_property(SOURCE ${ffmpeg_dynamic_lib_file} PROPERTY GENERATED 1)
endif()
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
