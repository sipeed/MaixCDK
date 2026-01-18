list(APPEND ADD_REQUIREMENTS nn cpu_mnn alsa_lib voice)
list(APPEND ADD_INCLUDE "include" "include/decoder" "include/preprocess")
list(APPEND ADD_DEFINITIONS -DDR_WAV_IMPLEMENTATION )
append_srcs_dir(ADD_SRCS src src/decoder src/preprocess src/device)

register_component(DYNAMIC)
