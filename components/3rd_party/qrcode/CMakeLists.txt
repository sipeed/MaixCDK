
set(version_str "2.1.0")
set(unzip_path "${DL_EXTRACTED_PATH}/zxing-cpp")
set(src_path "${unzip_path}/zxing-cpp-${version_str}")
set(file_sha256sum "6d54e403592ec7a143791c6526c1baafddf4c0897bb49b1af72b70a0f0c4a3fe")

################# Add include #################
list(APPEND ADD_INCLUDE "${src_path}/core/src"
                        "${src_path}/core/src/aztec"
                        "${src_path}/core/src/datamatrix"
                        "${src_path}/core/src/libzueci"
                        "${src_path}/core/src/maxicode"
                        "${src_path}/core/src/oned"
                        "${src_path}/core/src/pdf417"
                        "${src_path}/core/src/qrcode"
    )
# list(APPEND ADD_PRIVATE_INCLUDE "include_private")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
aux_source_directory("${src_path}/core/src" ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
aux_source_directory("${src_path}/core/src/aztec" ADD_SRCS)
aux_source_directory("${src_path}/core/src/datamatrix" ADD_SRCS)
aux_source_directory("${src_path}/core/src/libzueci" ADD_SRCS)
aux_source_directory("${src_path}/core/src/maxicode" ADD_SRCS)
aux_source_directory("${src_path}/core/src/oned" ADD_SRCS)
aux_source_directory("${src_path}/core/src/pdf417" ADD_SRCS)
aux_source_directory("${src_path}/core/src/qrcode" ADD_SRCS)
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
###############################################

###### Add required/dependent components ######
# list(APPEND ADD_REQUIREMENTS component1)
###############################################

###### Add link search path for requirements/libs ######
# list(APPEND ADD_LINK_SEARCH_PATH "${CONFIG_TOOLCHAIN_PATH}/lib")
# list(APPEND ADD_REQUIREMENTS m)  # add system libs, pthread or m(math) lib for example
###############################################

############ Add static libs ##################
# if(CONFIG_COMPONENT1_INCLUDE_STATIC_LIB)
#     list(APPEND ADD_STATIC_LIB "lib/libtest.a")
# endif()
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
