set(omv_version_str "${CONFIG_OMV_VERSION_MAJOR}.${CONFIG_OMV_VERSION_MINOR}.${CONFIG_OMV_VERSION_PATCH}")
set(omv_filename "omv-${omv_version_str}.zip")
set(omv_unzip_path "${DL_EXTRACTED_PATH}/omv")
set(src_path "${omv_unzip_path}/omv-${omv_version_str}")
if(${omv_version_str} VERSION_EQUAL "1.0.0")
    set(omv_url "https://files.catbox.moe/2c84zd.zip")
    set(omv_file_sha256sum "4ef0f61febe32886e5d85bb47f661e5d3455f5fe79b3edaacbe833a22566e985")
elseif(${omv_version_str} VERSION_EQUAL "1.0.1")
    set(omv_url "https://files.catbox.moe/gcyi07.zip")
    set(omv_file_sha256sum "37f5b8ae6072cdef38678effc07cc6dee223bbbbedd79616a506c303d6cd30f1")
elseif(${omv_version_str} VERSION_EQUAL "1.0.2")
    set(omv_url "https://files.catbox.moe/20evr3.zip")
    set(omv_file_sha256sum "1aee6d293e955a448aa5bdbd3d8f613c2126a4a49d129aeb1a0fbad22f66d773")
elseif(${omv_version_str} VERSION_EQUAL "1.0.3")
    set(omv_url "https://files.catbox.moe/idfwis.zip")
    set(omv_file_sha256sum "f6654992a52006fd2e01302fa778a6cee1e99e1f9f78a9eea9f759e4f6199d82")
elseif(${omv_version_str} VERSION_EQUAL "1.0.4")
    set(omv_url "https://files.catbox.moe/3za319.zip")
    set(omv_file_sha256sum "c36c3b8a0ac141be6d7843f129602ffb600a1f596391cd810c1044e20fbbf472")
elseif(${omv_version_str} VERSION_EQUAL "1.0.5")
    set(omv_url "https://files.catbox.moe/d36bwl.zip")
    set(omv_file_sha256sum "04075965c44f9957efb425880b6a221bda8b049230bff857340306d938d54ec7")
elseif(${omv_version_str} VERSION_EQUAL "1.0.6")
    set(omv_url "https://files.catbox.moe/5iyqf2.zip")
    set(omv_file_sha256sum "c025d8438ac814229b0c099d18cee77bdd2296ae24173f69a8e46b5ab71700e6")
elseif(${omv_version_str} VERSION_EQUAL "1.0.7")
    set(omv_url "https://files.catbox.moe/ik2hdj.zip")
    set(omv_file_sha256sum "450d3b97287055679d73c35e6f9d67a192c68b7ac3f8a0f7c6bf7fa3df48b743")
elseif(${omv_version_str} VERSION_EQUAL "1.0.8")
    set(omv_url "https://files.catbox.moe/26rfr9.zip")
    set(omv_file_sha256sum "a651cd2988ec7b25a80c41466aed55210162bc679c2b8711211525b970bdf417")
elseif(${omv_version_str} VERSION_EQUAL "1.0.9")
    set(omv_url "https://files.catbox.moe/ypel5x.zip")
    set(omv_file_sha256sum "770680d55f501a095e846bb8727d9fe0ad6a446be8fbc1671b61da0788977475")
elseif(${omv_version_str} VERSION_EQUAL "1.0.10")
    set(omv_url "https://files.catbox.moe/olys7p.zip")
    set(omv_file_sha256sum "e3b6d04a5379be52b226434ef0dfce404faf91955dfab0386a189684b99940bd")
endif()
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
# list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

#### Add compile option for this component
#### and components denpend on this component
# list(APPEND ADD_DEFINITIONS -DAAAAA222=1
#                             -DAAAAA333=1)
###############################################
list(APPEND ADD_FILE_DOWNLOADS "{
    'url': '${omv_url}',
    'urls': [],
    'sites': [],
    'sha256sum': '${omv_file_sha256sum}',
    'filename': '${omv_filename}',
    'path': 'omv',
    'check_files': ['omv-${omv_version_str}']
    }"
)

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

