set(srcs_path "${DL_EXTRACTED_PATH}/ax620e_bsp_sdk_srcs/ax620e_bsp_sdk")
################# Add include #################
set(bsp_glibc_path "${srcs_path}/msp/out/arm64_glibc")
set(bsp_sample_path "${srcs_path}/msp/sample")
set(bsp_app_path "${srcs_path}/app")
list(APPEND ADD_INCLUDE "${bsp_glibc_path}/include"
    "${bsp_sample_path}/common"
    )
list(APPEND ADD_PRIVATE_INCLUDE "${bsp_app_path}/component/3rd/inc/appweb")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
# aux_source_directory("inifile2/src" ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
append_srcs_dir(ADD_SRCS "${bsp_sample_path}/common")     # append source file in src dir to var ADD_SRCS
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
set(libax_dynamic_lib_file "${bsp_glibc_path}/lib/libax_skel.so"
"${bsp_glibc_path}/lib/libax_skel.so"
"${bsp_glibc_path}/lib/libax_avs.so"
"${bsp_glibc_path}/lib/libax_utility.so"
"${bsp_glibc_path}/lib/libax_af.so"
"${bsp_glibc_path}/lib/libax_venc.so"
# "${bsp_glibc_path}/lib/libax_faac.so"
"${bsp_glibc_path}/lib/libax_proton.so"
"${bsp_glibc_path}/lib/libsns_sc500ai.so"
"${bsp_glibc_path}/lib/libax_nt_stream.so"
"${bsp_glibc_path}/lib/libax_audio_3a.so"
"${bsp_glibc_path}/lib/libax_interpreter.so"
"${bsp_glibc_path}/lib/libsns_sc231ai.so"
"${bsp_glibc_path}/lib/libax_ivps.so"
"${bsp_glibc_path}/lib/libax_ives.so"
# "${bsp_glibc_path}/lib/libax_audio.so"
"${bsp_glibc_path}/lib/libax_ae.so"
"${bsp_glibc_path}/lib/libax_skel_tiny.so"
"${bsp_glibc_path}/lib/libsns_imx664.so"
"${bsp_glibc_path}/lib/libax_efuse.so"
"${bsp_glibc_path}/lib/libax_vo.so"
"${bsp_glibc_path}/lib/libsns_sc430ai.so"
"${bsp_glibc_path}/lib/libsns_s5kjn1sq03.so"
# "${bsp_glibc_path}/lib/libax_fdk.so"
"${bsp_glibc_path}/lib/libsns_sc450ai.so"
"${bsp_glibc_path}/lib/libax_sys.so"
"${bsp_glibc_path}/lib/libax_vdec.so"
"${bsp_glibc_path}/lib/libax_awb.so"
"${bsp_glibc_path}/lib/libax_mipi.so"
"${bsp_glibc_path}/lib/liblens_t5719.so"
"${bsp_glibc_path}/lib/libsns_imx678.so"
"${bsp_glibc_path}/lib/libax_ive.so"
"${bsp_glibc_path}/lib/libax_engine.so"
"${bsp_glibc_path}/lib/libsns_os04d10.so"
"${bsp_glibc_path}/lib/libsns_mis2032.so"
"${bsp_glibc_path}/lib/libax_nt_ctrl.so"
"${bsp_glibc_path}/lib/libsns_os08c10.so"
"${bsp_glibc_path}/lib/libax_avscali.so"
# "${bsp_glibc_path}/lib/libax_opus.so"
"${bsp_glibc_path}/lib/libsns_sc200ai.so"
"${bsp_glibc_path}/lib/libsns_dummy.so"
"${bsp_glibc_path}/lib/libsns_os04a10.so"
"${bsp_glibc_path}/lib/libsns_sc530ai.so"
"${bsp_glibc_path}/lib/libax_engine_tiny.so"
"${bsp_glibc_path}/lib/libsns_gc4653.so"
"${bsp_glibc_path}/lib/libax_gzipd.so"
"${bsp_glibc_path}/lib/libax_thermal.so"
"${bsp_glibc_path}/lib/libsns_sc4210.so"
"${bsp_glibc_path}/lib/libsns_gc4663.so"
"${bsp_glibc_path}/lib/libsns_sc830ai.so"
"${bsp_glibc_path}/lib/libsns_c4395.so"
"${bsp_glibc_path}/lib/libax_cipher.so"
# "${bsp_glibc_path}/lib/libax_opal.so"
"${bsp_glibc_path}/lib/libsns_sc850sl.so"
"${bsp_glibc_path}/lib/libax_syslog.so"
"${bsp_glibc_path}/lib/libsns_dummy_bittrue.so"
"${bsp_glibc_path}/lib/liblens_dciris.so"
# "${srcs_path}/third-party/faac/lib/arm64/glibc/libfaac.so.0"
"${srcs_path}/third-party/fdk-aac/lib/arm64/glibc/libfdk-aac.so.2"
"${srcs_path}/third-party/opus/lib/arm64/glibc/libopus.so.0"
"${bsp_app_path}/component/resource/lib/arm64/glibc/appweb/libmpr.so")

set_property(SOURCE ${libax_dynamic_lib_file} PROPERTY GENERATED 1)
list(APPEND ADD_DYNAMIC_LIB ${libax_dynamic_lib_file})
# list(APPEND ADD_DIST_LIB_IGNORE ${libax_dynamic_lib_file})

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

