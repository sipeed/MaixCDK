
# sophgo-alsa_lib
# set(alsa_lib_version_str "${CONFIG_SOPHGO_alsa_lib_VERSION_MAJOR}.${CONFIG_SOPHGO_alsa_lib_VERSION_MINOR}.${CONFIG_SOPHGO_alsa_lib_VERSION_PATCH}")
# set(alsa_lib_filename "sophgo-alsa_lib-${alsa_lib_version_str}.tar.xz")
# set(alsa_lib_unzip_path "${DL_EXTRACTED_PATH}/sophgo-alsa_lib")
# set(alsa_lib_src_path "${alsa_lib_unzip_path}/sophgo-alsa_lib-${alsa_lib_version_str}")
# if(${alsa_lib_version_str} VERSION_EQUAL "0.0.1")
#     set(alsa_lib_url "https://files.catbox.moe/bq5duu.xz")
#     set(alsa_lib_file_sha256sum "524021c333d9c601bd1e3a962dce02711269cc7bc342679ff317aa038e9679bd")
# else()
#     message(FATAL_ERROR "No sophgo alsa_lib version ${version_str} support, please edit to add sophgo alsa_lib support for this version")
# endif()

# list(APPEND ADD_FILE_DOWNLOADS "{
#     'url': '${alsa_lib_url}',
#     'urls': [],
#     'sites': [],
#     'sha256sum': '${alsa_lib_file_sha256sum}',
#     'filename': '${alsa_lib_filename}',
#     'path': 'sophgo-alsa_lib',
#     'check_files': ['sophgo-alsa_lib-${alsa_lib_version_str}']
#     }"
# )

set(alsa_lib_dir "lib")

# include
set(alsa_lib_include_dir "include")
list(APPEND ADD_INCLUDE ${alsa_lib_include_dir})
set_property(SOURCE ${alsa_lib_include_dir} PROPERTY GENERATED 1)

# lib
set(alsa_lib_dynamic_lib_file   ${alsa_lib_dir}/libasound.so
                                ${alsa_lib_dir}/libatopology.so)
list(APPEND ADD_DYNAMIC_LIB ${alsa_lib_dynamic_lib_file})
set_property(SOURCE ${alsa_lib_dynamic_lib_file} PROPERTY GENERATED 1)

register_component()
