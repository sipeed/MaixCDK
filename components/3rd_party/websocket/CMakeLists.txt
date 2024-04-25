list(APPEND ADD_FILE_DOWNLOADS "{
        'url': 'https://master.dl.sourceforge.net/project/websocket.mirror/0.8.2/WebSocket%2B%2B_0.8.2.tar.gz?viasf=1',
        'urls': [],
        'sites': ['https://github.com/zaphoyd/websocketpp/archive/refs/tags/0.8.2.tar.gz'],
        'sha256sum': '8b1773ea2832751071ac19d2708314d68316dd3916434c7dc0dd58cef14d51cd',
        'filename': 'WebSocketpp_0.8.2.tar.gz',
        'path': 'websocketpp',
        'rename': {'zaphoyd-websocketpp-0e42417': 'websocketpp-0.8.2'},
        'check_files': ['websocketpp-0.8.2']
    }"
)

set(websocketpp_root "${DL_EXTRACTED_PATH}/websocketpp/websocketpp-0.8.2")

if(CONFIG_WEBSOCKET_USE_BOOST_ASIO)
    list(APPEND ADD_REQUIREMENTS boost_system)
    list(APPEND ADD_DEFINITIONS -DASIO_STANDALONE=0)
else()
list(APPEND ADD_REQUIREMENTS asio)
    list(APPEND ADD_DEFINITIONS -DASIO_STANDALONE=1)
endif()

################# Add include #################
list(APPEND ADD_INCLUDE "${websocketpp_root}"
    )
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
#### and components denpend on this component
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
# 'urls': [],
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

