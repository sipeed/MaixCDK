set(maixcam2_msp_version_str "${CONFIG_MAIXCAM2_MSP_VERSION_MAJOR}.${CONFIG_MAIXCAM2_MSP_VERSION_MINOR}.${CONFIG_MAIXCAM2_MSP_VERSION_PATCH}_${CONFIG_MAIXCAM2_MSP_COMPILED_VERSION}")
set(srcs_path "${DL_EXTRACTED_PATH}/maixcam2_msp_srcs/maixcam2_msp_arm64_glibc_v${maixcam2_msp_version_str}")
################# Add include #################
set(msp_glibc_path "${srcs_path}/out/arm64_glibc")
set(msp_local_path "msp")
list(APPEND ADD_INCLUDE "${msp_local_path}/component/isp_proton/sensor/common/include"
                        "${msp_local_path}/out/arm64_glibc/include"
                        "${msp_local_path}/sample/common"
    )
# list(APPEND ADD_PRIVATE_INCLUDE "${msp_app_path}/component/3rd/inc/appweb")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
# aux_source_directory("inifile2/src" ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
append_srcs_dir(ADD_SRCS "${msp_local_path}/sample/common")     # append source file in src dir to var ADD_SRCS
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

# endif()
###############################################

############ Add dynamic libs ##################
set(libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_sys.so"
    )

if(CONFIG_AX620E_MSP_ENABLE_SENSOR_LIB)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_af.so"
                                        "${msp_glibc_path}/lib/libax_venc.so"
                                        "${msp_glibc_path}/lib/libax_vdec.so"
                                        "${msp_glibc_path}/lib/libax_proton.so"
                                        "${msp_glibc_path}/lib/libax_mipi.so"
                                        "${msp_glibc_path}/lib/libax_ae.so"
                                        "${msp_glibc_path}/lib/libax_awb.so"
                                        "${msp_glibc_path}/lib/libax_ive.so"
                                        "${msp_glibc_path}/lib/libax_ivps.so"
                                        "${msp_glibc_path}/lib/libax_nt_stream.so"
                                        "${msp_glibc_path}/lib/libax_nt_ctrl.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_EXTRA_SENSOR_LIB)
    list(APPEND libax_dynamic_lib_file  # maixcam2 official supported sensors
                                        # use dl_open, so we can not add these libs when compile.
                                        "${msp_glibc_path}/lib/libsns_sc450ai.so"
                                        "${msp_glibc_path}/lib/libsns_os04a10.so"
                                        "${msp_glibc_path}/lib/libsns_gc4653.so"
                                        # others
                                        "${msp_glibc_path}/lib/libsns_sc500ai.so"
                                        "${msp_glibc_path}/lib/libsns_sc231ai.so"
                                        "${msp_glibc_path}/lib/libsns_imx664.so"
                                        "${msp_glibc_path}/lib/libsns_sc430ai.so"
                                        "${msp_glibc_path}/lib/libsns_s5kjn1sq03.so"
                                        "${msp_glibc_path}/lib/libsns_sc850sl.so"
                                        "${msp_glibc_path}/lib/libsns_imx678.so"
                                        "${msp_glibc_path}/lib/libsns_os04d10.so"
                                        "${msp_glibc_path}/lib/libsns_mis2032.so"
                                        "${msp_glibc_path}/lib/libsns_os08c10.so"
                                        "${msp_glibc_path}/lib/libsns_sc200ai.so"
                                        "${msp_glibc_path}/lib/libsns_sc530ai.so"
                                        "${msp_glibc_path}/lib/libsns_sc4210.so"
                                        "${msp_glibc_path}/lib/libsns_gc4663.so"
                                        "${msp_glibc_path}/lib/libsns_sc830ai.so"
                                        "${msp_glibc_path}/lib/libsns_c4395.so"
                                        "${msp_glibc_path}/lib/libsns_dummy.so"
                                        "${msp_glibc_path}/lib/libsns_dummy_bittrue.so"
                                        "${msp_glibc_path}/lib/liblens_t5719.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_NPU_LIB)
    list(APPEND libax_dynamic_lib_file "${msp_glibc_path}/lib/libax_engine.so"
                                    #  "${msp_glibc_path}/lib/libax_engine_tiny.so"
                                       "${msp_glibc_path}/lib/libax_interpreter.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_AUDIO_LIB)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_audio_3a.so"
                                        "${msp_glibc_path}/lib/libax_audio.so"
                                        "${msp_glibc_path}/third-party/lib/libsamplerate.so"
                                        "${msp_glibc_path}/third-party/lib/libtinyalsa.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_IVES_LIB)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_ives.so" # Intelligent Video Extension System
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_SKEL_LIB) # SKELeton Artificial Intelligent Algorithm
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_skel.so"
                                      # "${msp_glibc_path}/lib/libax_skel_tiny.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_OPAL_LIB) # OPerator Abstract Layer(IPC application high level API)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_opal.so")
endif()


if(CONFIG_AX620E_MSP_ENABLE_EFUSE_LIB)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_efuse.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_VO_LIB)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_vo.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_GZIP_LIB)   # hardware acceleration API for gzip
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_gzipd.so"
    )
endif()

if(CONFIG_AX620E_MSP_ENABLE_EXTRA_LIB)
    list(APPEND libax_dynamic_lib_file  "${msp_glibc_path}/lib/libax_utility.so"
                                        "${msp_glibc_path}/lib/libax_avs.so"
                                        "${msp_glibc_path}/lib/libax_avscali.so"
                                        "${msp_glibc_path}/lib/libax_thermal.so"
                                        "${msp_glibc_path}/lib/libax_cipher.so"
                                        "${msp_glibc_path}/lib/libax_syslog.so"
                                        "${msp_glibc_path}/lib/liblens_dciris.so"
                                        # "${msp_glibc_path}/lib/libax_audio.so"
                                        # "${msp_glibc_path}/lib/libax_fdk.so"
                                        # "${msp_glibc_path}/lib/libax_opus.so"
                                        # "${msp_glibc_path}/lib/libax_faac.so"
                                        # "${srcs_path}/third-party/faac/lib/arm64/glibc/libfaac.so.0"
                                        # "${srcs_path}/third-party/fdk-aac/lib/arm64/glibc/libfdk-aac.so.2"
                                        # "${srcs_path}/third-party/opus/lib/arm64/glibc/libopus.so.0"
                                        # "${msp_app_path}/component/resource/lib/arm64/glibc/appweb/libmpr.so"
    )
endif()

set_property(SOURCE ${libax_dynamic_lib_file} PROPERTY GENERATED 1)
list(APPEND ADD_DYNAMIC_LIB ${libax_dynamic_lib_file})
list(APPEND ADD_DIST_LIB_IGNORE ${libax_dynamic_lib_file})

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
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-rpath,${srcs_path}/third-party/faac/lib/arm/glibc/libfaac.so.0" PARENT_SCOPE)
###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()

