set(cpu_mnn_lib_dir "lib")

# include
list(APPEND ADD_INCLUDE "include" "include/MNN" "include/imageHelper")

# lib
if(PLATFORM_MAIXCAM)
    set(cpu_mnn_lib_file ${cpu_mnn_lib_dir}/maixcam2/libMNN_rv64musl.a)
    list(APPEND ADD_STATIC_LIB ${cpu_mnn_lib_file})
elseif(PLATFORM_MAIXCAM2)
    set(cpu_mnn_lib_file ${cpu_mnn_lib_dir}/maixcam2/libMNN_aarch64.a)
    list(APPEND ADD_STATIC_LIB ${cpu_mnn_lib_file})
endif()

register_component()
