set(onnxruntime_version_str "${CONFIG_ONNXRUNTIME_VERSION_MAJOR}.${CONFIG_ONNXRUNTIME_VERSION_MINOR}.${CONFIG_ONNXRUNTIME_VERSION_PATCH}")
set(onnxruntime_unzip_path "${DL_EXTRACTED_PATH}/onnxruntime_srcs")
if(PLATFORM_MAIXCAM)
set(src_path "${onnxruntime_unzip_path}/maixcam_onnxruntime_v${onnxruntime_version_str}")
elseif(PLATFORM_MAIXCAM2)
set(src_path "${onnxruntime_unzip_path}/maixcam2_onnxruntime_v${onnxruntime_version_str}")
endif()
################# Add include #################
if(PLATFORM_MAIXCAM)
    list(APPEND ADD_INCLUDE "${src_path}/include")
elseif(PLATFORM_MAIXCAM2)
    list(APPEND ADD_INCLUDE "${src_path}/include/onnxruntime")
endif()

# list(APPEND ADD_PRIVATE_INCLUDE "include_private")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
# aux_source_directory("inifile2/src" ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
###############################################

###### Add required/dependent components ######
# list(APPEND ADD_REQUIREMENTS component1)
###############################################

###### Add link search path for requirements/libs ######
# list(APPEND ADD_LINK_SEARCH_PATH "${CONFIG_TOOLCHAIN_PATH}/lib")
# list(APPEND ADD_REQUIREMENTS m)  # add system libs, pthread or m(math) lib for example
list(APPEND ADD_DYNAMIC_LIB "${src_path}/lib/libonnxruntime.so.1")
list(APPEND ADD_DIST_LIB_IGNORE "${src_path}/lib/libonnxruntime.so.1")

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


