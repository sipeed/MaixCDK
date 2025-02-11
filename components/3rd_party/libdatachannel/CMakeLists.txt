set(srcs_path "${DL_EXTRACTED_PATH}/libdatachannel_srcs/libdatachannel")
################# Add include #################
list(APPEND ADD_INCLUDE "${srcs_path}/include"
    )
list(APPEND ADD_PRIVATE_INCLUDE "${srcs_path}/include/rtc" "${srcs_path}/src")
###############################################

############## Add source files ###############
if(PLATFORM_MAIXCAM)
list(APPEND ADD_SRCS  "private/atomic.cpp")
endif()
aux_source_directory("${srcs_path}/src" ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
aux_source_directory("${srcs_path}/src/impl" ADD_SRCS)
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
###############################################

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS openssl json libjuice libsrtp plog usrsctp)
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
list(APPEND ADD_DEFINITIONS_PRIVATE -DJUICE_STATIC -DRTC_ENABLE_MEDIA=1 -DRTC_ENABLE_WEBSOCKET=1 -DRTC_EXPORTS -DRTC_SYSTEM_JUICE=0 -DRTC_SYSTEM_SRTP=0 -DUSE_GNUTLS=0 -DUSE_NICE=0 -D_GNU_SOURCE -Ddatachannel_EXPORTS -w)

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

