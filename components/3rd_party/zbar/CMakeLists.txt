set(zbar_version_str "${CONFIG_ZBAR_VERSION_MAJOR}.${CONFIG_ZBAR_VERSION_MINOR}")
set(unzip_path ".")
set(src_path "${unzip_path}/ZBar-${zbar_version_str}")

################# Add include #################
list(APPEND ADD_INCLUDE         "./include"
    )
list(APPEND ADD_PRIVATE_INCLUDE "./private_include"
                                "${src_path}/include"
                                "${src_path}/zbar")
###############################################
############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
aux_source_directory("${src_path}/zbar" ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
aux_source_directory("${src_path}/zbar/decoder" ADD_SRCS)
aux_source_directory("${src_path}/zbar/qrcode" ADD_SRCS)
aux_source_directory("./src" ADD_SRCS)
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
list(REMOVE_ITEM ADD_SRCS "${src_path}/zbar/jpeg.c")
list(REMOVE_ITEM ADD_SRCS "${src_path}/zbar/svg.c")
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
list(APPEND ADD_DEFINITIONS_PRIVATE -w)

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
