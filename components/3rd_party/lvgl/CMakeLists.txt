
set(version_str "${CONFIG_LVGL_VERSION_MAJOR}.${CONFIG_LVGL_VERSION_MINOR}.${CONFIG_LVGL_VERSION_PATCH}")
set(unzip_dir_name "lvgl-${version_str}")

set(unzip_path "${DL_EXTRACTED_PATH}/lvgl_srcs/${unzip_dir_name}")
set(src_path "${unzip_path}/lvgl") # MaixCDK/dl/extracted/lvgl_srcs/lvgl-9.0.0/lvgl

################# Add include #################
list(APPEND ADD_INCLUDE "conf"
                        "."
                        "include"
                        "driver"
                        "${unzip_path}"
                        "${src_path}"
                        "${src_path}/src/"
                        "${src_path}/src/core"
                        "${src_path}/src/display"
                        "${src_path}/src/draw"
                        "${src_path}/src/drivers"
                        "${src_path}/src/font"
                        "${src_path}/src/indev"
                        "${src_path}/src/layouts"
                        "${src_path}/src/libs"
                        "${src_path}/src/misc"
                        "${src_path}/src/osal"
                        "${src_path}/src/others"
                        "${src_path}/src/stdlib"
                        "${src_path}/src/themes"
                        "${src_path}/src/tick"
                        "${src_path}/src/widgets"
                        "${src_path}/src"
    )
# list(APPEND ADD_PRIVATE_INCLUDE "include_private")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
# aux_source_directory(src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
FILE(GLOB_RECURSE EXTRA_SRC  "${src_path}/src/*.c")
LIST(APPEND ADD_SRCS ${EXTRA_SRC})
append_srcs_dir(ADD_SRCS "driver")
append_srcs_dir(ADD_SRCS "src")


# set_property(SOURCE ${src_path} PROPERTY GENERATED 1)

list(APPEND ADD_REQUIREMENTS basic vision)


# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
###############################################

###### Add required/dependent components ######
# list(APPEND ADD_REQUIREMENTS component1)
###############################################

###### Add link search path for requirements/libs ######
# list(APPEND ADD_LINK_SEARCH_PATH "${CONFIG_TOOLCHAIN_PATH}/lib")
list(APPEND ADD_REQUIREMENTS rlottie)
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
list(APPEND ADD_DEFINITIONS -DLV_CONF_INCLUDE_SIMPLE -DLV_LVGL_H_INCLUDE_SIMPLE)
# if(CONFIG_LV_CONF_SKIP)
#     list(APPEND ADD_DEFINITIONS -DLV_CONF_SKIP)
# endif()
###############################################

############ Add static libs ##################
#### Update parent's variables like CMAKE_C_LINK_FLAGS
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

