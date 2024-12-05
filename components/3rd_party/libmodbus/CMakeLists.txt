set(libmodbus_version_str "3.1.10")
set(libmodbus_unzip_path "${DL_EXTRACTED_PATH}/libmodbus_srcs")
set(src_path "${libmodbus_unzip_path}/libmodbus")
set(output_path "${libmodbus_unzip_path}/libmodbus-build")
set(log_file_path ${src_path}/build.log)

list(APPEND ADD_REQUIREMENTS)
list(APPEND ADD_FILE_DEPENDS ${output_path}/lib/libmodbus.a)
set_property(SOURCE ${python_h_path} PROPERTY GENERATED 1)
add_custom_command(OUTPUT ${output_path}/lib/libmodbus.a
    COMMAND cd ${src_path} && ./autogen.sh >> ${log_file_path} 2>&1 && CC=${CMAKE_C_COMPILER} ./configure --host=riscv64 --enable-static --prefix=${output_path} >> ${log_file_path} 2>&1 && make >> ${log_file_path} 2>&1 && make install >> ${log_file_path} 2>&1 || (echo "Error occurred during libmodbus generation, check ${log_file_path}" && exit 1)
    COMMENT "Generating libmodbus-v${libmodbus_version_str}"
)

set(libmodbus_include_dir "${output_path}/include")
list(APPEND ADD_INCLUDE ${libmodbus_include_dir})
set_property(SOURCE ${libmodbus_include_dir} PROPERTY GENERATED 1)

set(libmodbus_static_lib_file ${output_path}/lib/libmodbus.a)
list(APPEND ADD_STATIC_LIB ${libmodbus_static_lib_file})
set_property(SOURCE ${libmodbus_static_lib_file} PROPERTY GENERATED 1)


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
# set(ffmpeg_dynamic_lib_file ${src_path}/lib/libavcodec.so
#                             ${src_path}/lib/libavdevice.so
#                             ${src_path}/lib/libavfilter.so
#                             ${src_path}/lib/libavformat.so
#                             ${src_path}/lib/libavresample.so
#                             ${src_path}/lib/libavutil.so
#                             ${src_path}/lib/libpostproc.so
#                             ${src_path}/lib/libswresample.so
#                             ${src_path}/lib/libswscale.so
#                                 )
# list(APPEND ADD_DYNAMIC_LIB ${ffmpeg_dynamic_lib_file})
# list(APPEND ADD_DIST_LIB_IGNORE ${ffmpeg_dynamic_lib_file})
# set_property(SOURCE ${ffmpeg_dynamic_lib_file} PROPERTY GENERATED 1)
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
