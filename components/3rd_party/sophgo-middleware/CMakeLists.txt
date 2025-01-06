set(middleware_src_path "${CMAKE_CURRENT_SOURCE_DIR}/sophgo-middleware")

# middleware
set(middleware_include_dir .
    ${middleware_src_path}/v2/component/panel/sg200x
    ${middleware_src_path}/v2/include
    ${middleware_src_path}/v2/include/isp/sg200x
    ${middleware_src_path}/v2/sample/common
    ${middleware_src_path}/v2/uapi
    ${middleware_src_path}/v2/3rdparty/inih
    ${middleware_src_path}/v2/modules/ive/include/
)
list(APPEND ADD_INCLUDE ${middleware_include_dir})
set_property(SOURCE ${middleware_include_dir} PROPERTY GENERATED 1)

append_srcs_dir(middleware_src_dir  ${middleware_src_path}/v2/sample/common
                                    ${middleware_src_path}/v2/component/isp/sensor/cv181x/gcore_gc4653
                                    ${middleware_src_path}/v2/component/isp/sensor/sg200x/sms_sc035gs
                                    ${middleware_src_path}/v2/component/isp/sensor/sg200x/ov_ov2685
                                    ${middleware_src_path}/v2/component/isp/sensor/sg200x/lontium_lt6911
                                    ${middleware_src_path}/v2/component/isp/sensor/sg200x/ov_os04a10
                                    ${middleware_src_path}/v2/component/isp/sensor/sg200x/gcore_gc02m1
                                    )
list(APPEND ADD_SRCS        ${middleware_src_dir})
set_property(SOURCE ${middleware_src_dir} PROPERTY GENERATED 1)

set(mmf_lib_dir ${middleware_src_path}/v2/lib)
set(middleware_dynamic_lib_file     ${mmf_lib_dir}/libcvi_ive.so
                                    ${mmf_lib_dir}/libcvi_bin.so
                                    ${mmf_lib_dir}/libaaccomm2.so
                                    ${mmf_lib_dir}/libaacdec2.so
                                    ${mmf_lib_dir}/libaacenc2.so
                                    ${mmf_lib_dir}/libaacsbrdec2.so
                                    ${mmf_lib_dir}/libaacsbrenc2.so
                                    ${mmf_lib_dir}/libae.so
                                    ${mmf_lib_dir}/libaf.so
                                    ${mmf_lib_dir}/libawb.so
                                    ${mmf_lib_dir}/libcvi_audio.so
                                    ${mmf_lib_dir}/libcvi_bin_isp.so
                                    ${mmf_lib_dir}/libcvi_bin.so
                                    ${mmf_lib_dir}/libcvi_ispd2.so
                                    ${mmf_lib_dir}/libcvi_RES1.so
                                    ${mmf_lib_dir}/libcvi_ssp.so
                                    ${mmf_lib_dir}/libcvi_VoiceEngine.so
                                    ${mmf_lib_dir}/libcvi_vqe.so
                                    ${mmf_lib_dir}/libdnvqe.so
                                    ${mmf_lib_dir}/libisp_algo.so
                                    ${mmf_lib_dir}/libisp.so
                                    ${mmf_lib_dir}/libmipi_tx.so
                                    ${mmf_lib_dir}/libmisc.so
                                    ${mmf_lib_dir}/libosdc.so
                                    ${mmf_lib_dir}/libraw_dump.so
                                    ${mmf_lib_dir}/libsys.so
                                    ${mmf_lib_dir}/libvdec.so
                                    ${mmf_lib_dir}/libvenc.so
                                    ${mmf_lib_dir}/libvpu.so
                                    ${mmf_lib_dir}/libjson-c.so.5
                                    ${mmf_lib_dir}/libtinyalsa.so
                                    ${mmf_lib_dir}/3rd/libcli.so
                                    ${mmf_lib_dir}/3rd/libini.so)
list(APPEND ADD_DYNAMIC_LIB ${middleware_dynamic_lib_file})
set_property(SOURCE ${middleware_dynamic_lib_file} PROPERTY GENERATED 1)

list(APPEND ADD_DEFINITIONS_PRIVATE -DSENSOR_GCORE_GC4653
                                -DSENSOR0_TYPE=SONY_IMX327_MIPI_2M_30FPS_12BIT
                                -DSENSOR1_TYPE=SONY_IMX327_MIPI_2M_30FPS_12BIT)

list(APPEND ADD_DEFINITIONS_PRIVATE -DSENSOR_SMS_SC035GS
                                -DSENSOR2_TYPE=SMS_SC035GS_MIPI_480P_120FPS_12BIT)

list(APPEND ADD_DEFINITIONS_PRIVATE -DSENSOR_OV_OV2685)
list(APPEND ADD_DEFINITIONS_PRIVATE -DSENSOR_LONTIUM_LT6911)
list(APPEND ADD_DEFINITIONS_PRIVATE -DSENSOR_OV_OS04A10)
list(APPEND ADD_DEFINITIONS_PRIVATE -DSENSOR_GCORE_GC02M1)
register_component(DYNAMIC)