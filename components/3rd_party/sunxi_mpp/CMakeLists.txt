if(PLATFORM_MAIXCAM)
set(sunxi_mpp_version_str "${CONFIG_SUNXI_MPP_VERSION_MAJOR}.${CONFIG_SUNXI_MPP_VERSION_MINOR}.${CONFIG_SUNXI_MPP_VERSION_PATCH}")
set(sunxi_mpp_unzip_path "${DL_EXTRACTED_PATH}/sunxi_mpp")
set(sunxi_mpp_src_path "${sunxi_mpp_unzip_path}/sunxi-mpp-${sunxi_mpp_version_str}")

############### Add include ###################
set(sunxi_mpp_include_dir ${sunxi_mpp_src_path}/include/algo_aw/
                                ${sunxi_mpp_src_path}/include/rtsp/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/
                                ${sunxi_mpp_src_path}/include/mpp/system/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/config/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/include/media/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/include/media/utils/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/include/net/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/include/utils/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/midware/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/midware/decoding/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/midware/decoding/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/midware/encoding/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/midware/encoding/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/AudioLib/osal/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/aec_lib/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/aec_lib/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/agc_lib/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/agc_lib/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/drc_lib/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/drc_lib/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/include_FsWriter/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/include_demux/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/include_muxer/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/include_stream/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libADAS/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libADAS/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libISE/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libISE/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libMODSoft/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libMODSoft/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libResample/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libResample/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/lib_aw_ai_core/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/lib_aw_ai_core/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/lib_aw_ai_mt/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/lib_aw_ai_mt/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libcedarc/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libcedarc/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libcedarx/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libcedarx/libcore/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libcedarx/libcore/common/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libcedarx/libcore/common/iniparser/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/include/V4l2Camera/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/include/V4l2Camera/linux/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/include/device/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/isp_dev/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/libisp/isp_tuning/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/textEncLib/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/LIBRARY/textEncLib/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/include/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/include/audio/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/include/component/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/include/include_render/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/include/utils/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/media/include/videoIn/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/sample/
                                ${sunxi_mpp_src_path}/include/mpp/middleware/sample/configfileparser/
                                ${sunxi_mpp_src_path}/include/mpp/system/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/crypto/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/cutils/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/database/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/hardware/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/log/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/lua/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/openssl/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/sys/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/tinyhttp/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/utils/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/vo/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/include/wifi/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/libion/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/libion/include/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/rgb_ctrl/
                                ${sunxi_mpp_src_path}/include/mpp/system/public/smartlink/
)
list(APPEND ADD_INCLUDE ${sunxi_mpp_include_dir})
set_property(SOURCE ${sunxi_mpp_include_dir} PROPERTY GENERATED 1)
# list(APPEND ADD_PRIVATE_INCLUDE "")
###############################################

############ Add source files #################
# list(APPEND ADD_SRCS  "src/main.c"
#                       "src/test.c"
#     )
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
# set (sunxi_mpp_DIR sunxi_mpp/lib/cmake/sunxi_mpp4)
# find_package(sunxi_mpp REQUIRED)
###############################################

############ Add static libs ##################
set(sunxi_mpp_static_lib_file   "${sunxi_mpp_src_path}/lib/libaw_mpp.a"
                                "${sunxi_mpp_src_path}/lib/libcedarxdemuxer.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_aac_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_id3v2_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_mp3_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_mov_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_mpg_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_ts_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_wav_parser.a"
                                "${sunxi_mpp_src_path}/lib/libcdx_file_stream.a"
                                "${sunxi_mpp_src_path}/lib/libISP.a"
                                "${sunxi_mpp_src_path}/lib/libisp_ini.a"
                                "${sunxi_mpp_src_path}/lib/libisp_dev.a"
                                "${sunxi_mpp_src_path}/lib/libiniparser.a"
                                "${sunxi_mpp_src_path}/lib/libisp_ae.a"
                                "${sunxi_mpp_src_path}/lib/libisp_af.a"
                                "${sunxi_mpp_src_path}/lib/libisp_afs.a"
                                "${sunxi_mpp_src_path}/lib/libisp_awb.a"
                                "${sunxi_mpp_src_path}/lib/libisp_base.a"
                                "${sunxi_mpp_src_path}/lib/libisp_gtm.a"
                                "${sunxi_mpp_src_path}/lib/libisp_iso.a"
                                "${sunxi_mpp_src_path}/lib/libisp_math.a"
                                "${sunxi_mpp_src_path}/lib/libisp_md.a"
                                "${sunxi_mpp_src_path}/lib/libisp_pltm.a"
                                "${sunxi_mpp_src_path}/lib/libisp_rolloff.a"
                                "${sunxi_mpp_src_path}/lib/libaacenc.a"
                                "${sunxi_mpp_src_path}/lib/libmp3enc.a"
                                "${sunxi_mpp_src_path}/lib/libaac.a"
                                "${sunxi_mpp_src_path}/lib/libmp3.a"
                                "${sunxi_mpp_src_path}/lib/libwav.a"
                                "${sunxi_mpp_src_path}/lib/libAec.a"
                                "${sunxi_mpp_src_path}/lib/libDrc.a"
                                "${sunxi_mpp_src_path}/lib/libAgc.a"
                                "${sunxi_mpp_src_path}/lib/libawnn_det.a"
                                "${sunxi_mpp_src_path}/lib/libaac_muxer.a"
                                "${sunxi_mpp_src_path}/lib/libffavutil.a"
                                "${sunxi_mpp_src_path}/lib/libFsWriter.a"
                                "${sunxi_mpp_src_path}/lib/libmp3_muxer.a"
                                "${sunxi_mpp_src_path}/lib/libmp4_muxer.a"
                                "${sunxi_mpp_src_path}/lib/libmpeg2ts_muxer.a"
                                "${sunxi_mpp_src_path}/lib/libmuxers.a"
                                "${sunxi_mpp_src_path}/lib/libraw_muxer.a"
)
list(APPEND ADD_STATIC_LIB ${sunxi_mpp_static_lib_file})
set_property(SOURCE ${sunxi_mpp_static_lib_file} PROPERTY GENERATED 1)
###############################################

############ Add dynamic libs ##################
set(sunxi_mpp_dynamic_lib_file  "${sunxi_mpp_src_path}/lib/libglog.so"
                                "${sunxi_mpp_src_path}/lib/libasound.so"
                                "${sunxi_mpp_src_path}/lib/libz.so"
                                "${sunxi_mpp_src_path}/lib/liblog.so"
                                "${sunxi_mpp_src_path}/lib/libcdc_base.so"
                                "${sunxi_mpp_src_path}/lib/libmedia_utils.so"
                                "${sunxi_mpp_src_path}/lib/libcdx_common.so"
                                "${sunxi_mpp_src_path}/lib/libcdx_base.so"
                                "${sunxi_mpp_src_path}/lib/libunwind.so"
                                "${sunxi_mpp_src_path}/lib/libion.so"
                                "${sunxi_mpp_src_path}/lib/libcdx_parser.so"
                                "${sunxi_mpp_src_path}/lib/libcdx_stream.so"
                                "${sunxi_mpp_src_path}/lib/libmedia_utils.so"
                                "${sunxi_mpp_src_path}/lib/libMemAdapter.so"
                                "${sunxi_mpp_src_path}/lib/libVE.so"
                                "${sunxi_mpp_src_path}/lib/libcedarxstream.so"
                                "${sunxi_mpp_src_path}/lib/libsample_confparser.so"
                                "${sunxi_mpp_src_path}/lib/libcdx_common.so"
                                "${sunxi_mpp_src_path}/lib/libcdx_base.so"
                                "${sunxi_mpp_src_path}/lib/libResample.so"
                                "${sunxi_mpp_src_path}/lib/libaw_g726dec.so"
                                "${sunxi_mpp_src_path}/lib/libadecoder.so"
                                "${sunxi_mpp_src_path}/lib/libawh264.so"
                                "${sunxi_mpp_src_path}/lib/libawh265.so"
                                "${sunxi_mpp_src_path}/lib/libawmjpeg.so"
                                "${sunxi_mpp_src_path}/lib/libcedarx_aencoder.so"
                                "${sunxi_mpp_src_path}/lib/libvenc_codec.so"
                                "${sunxi_mpp_src_path}/lib/libvenc_base.so"
                                "${sunxi_mpp_src_path}/lib/libvdecoder.so"
                                "${sunxi_mpp_src_path}/lib/libvideoengine.so"
                                "${sunxi_mpp_src_path}/lib/libcedarxrender.so"
                                "${sunxi_mpp_src_path}/lib/libhwdisplay.so"
                                "${sunxi_mpp_src_path}/lib/libvencoder.so"
)
list(APPEND ADD_DYNAMIC_LIB ${sunxi_mpp_dynamic_lib_file})
set_property(SOURCE ${sunxi_mpp_dynamic_lib_file} PROPERTY GENERATED 1)
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

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()
endif() # if(PLATFORM_MAIXCAM)

