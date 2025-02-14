set(src_path "${CMAKE_CURRENT_SOURCE_DIR}/omv")

############### Add include ###################
set(omv_include_dir "."
                    "${src_path}/alloc"
                    "${src_path}"
                    "${src_path}/imlib"
                    "${src_path}/common"
                    "${src_path}/ports/linux"
                    "${src_path}/ports/linux/cmsis"
                    "${src_path}/ports/linux/oofatfs"
                    "${src_path}/ports/linux/py")
list(APPEND ADD_INCLUDE ${omv_include_dir})
set_property(SOURCE ${omv_include_dir} PROPERTY GENERATED 1)
# list(APPEND ADD_PRIVATE_INCLUDE "")
###############################################

############ Add source files #################
list(APPEND ADD_SRCS    # DO NOT ADD "${src_path}/alloc/framebuffer.c" of omv, it is not supported.
                        "${src_path}/alloc/fb_alloc.c"
                        "${src_path}/alloc/umm_malloc.c"
                        "${src_path}/alloc/unaligned_memcpy.c"
                        "${src_path}/alloc/xalloc.c"
                        "${src_path}/ports/linux/py/mpprint.c"
                        "${src_path}/ports/linux/py/obj.c"
                        "${src_path}/ports/linux/py/runtime.c"
                        "${src_path}/imlib/imlib.c"
                        "${src_path}/imlib/hough.c"
                        "${src_path}/imlib/lsd.c"
                        "${src_path}/imlib/fmath.c"
                        "${src_path}/imlib/xyz_tab.c"
                        "${src_path}/imlib/mathop.c"
                        "${src_path}/imlib/filter.c"
                        "${src_path}/imlib/clahe.c"
                        "${src_path}/imlib/isp.c"
                        "${src_path}/imlib/draw.c"
                        "${src_path}/imlib/font.c"
                        "${src_path}/imlib/yuv.c"
                        "${src_path}/imlib/bayer.c"
                        "${src_path}/imlib/blob.c"
                        "${src_path}/imlib/collections.c"
                        "${src_path}/imlib/lab_tab.c"
                        "${src_path}/imlib/sincos_tab.c"
                        "${src_path}/imlib/binary.c"
                        "${src_path}/imlib/phasecorrelation.c"
                        "${src_path}/imlib/fft.c"
                        "${src_path}/imlib/apriltag.c"
                        "${src_path}/imlib/rectangle.c"
                        "${src_path}/imlib/line.c"
                        "${src_path}/imlib/stats.c"
                        "${src_path}/imlib/qrcode.c"
                        "${src_path}/imlib/dmtx.c"
                        "${src_path}/imlib/zbar.c"
                        "${src_path}/imlib/template.c"
                        "${src_path}/imlib/integral.c"
                        "${src_path}/imlib/eye.c"
                        "${src_path}/imlib/lbp.c"
                        "${src_path}/imlib/edge.c"
                        "${src_path}/imlib/hog.c"
                        "${src_path}/imlib/pool.c"
                        "${src_path}/imlib/lab_tab.c"
                        "${src_path}/imlib/yuv_tab.c"
                        "${src_path}/common/array.c"
)
set_property(SOURCE ${ADD_SRCS} PROPERTY GENERATED 1)
set_property(SOURCE "${src_path}/alloc" PROPERTY GENERATED 1)
# append_srcs_dir(ADD_SRCS "src")       # append source file in src dir to var ADD_SRCS
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
# set (omv_DIR omv/lib/cmake/omv4)
# find_package(omv REQUIRED)
###############################################

############ Add static libs ##################

###############################################

############ Add dynamic libs ##################

###############################################

#### Add compile option for this component ####
#### Just for this component, won't affect other
#### modules, including component that depend
#### on this component
list(APPEND ADD_DEFINITIONS_PRIVATE -w)
if(PLATFORM_MAIXCAM2)
    list(APPEND ADD_DEFINITIONS_PRIVATE -D__ARM_ARCH=0)
endif()
#### Add compile option for this component
#### Add components that depend on this component
# list(APPEND ADD_DEFINITIONS -DAAAAA222=1
#                             -DAAAAA333=1)
###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

