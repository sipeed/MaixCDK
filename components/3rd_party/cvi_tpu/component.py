
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    if (not confs.get("PLATFORM_MAIXCAM", None)) or confs.get("CONFIG_COMPONENTS_COMPILE_FROM_SOURCE", None):
        return []
    if "musl" not in confs["CONFIG_TOOLCHAIN_PATH"] and "glibc" not in confs["CONFIG_TOOLCHAIN_PATH"]:
        return []
    version = f"{confs['CONFIG_BROTLI_VERSION_MAJOR']}.{confs['CONFIG_BROTLI_VERSION_MINOR']}.{confs['CONFIG_BROTLI_VERSION_PATCH']}"
    url = "https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/cvi_tpu_lib_v4.1.0-23_2024.8.7.tar.xz"
    sha256sum = "15d88abdbc368690de0304240e3a859aec56b050283cc218803099d56be5251a"
    filename = "cvi_tpu_lib_v4.1.0-23_2024.8.7.tar.xz"
    path = "cvi_tpu"
    check_file = 'cvi_tpu_lib_v4.1.0-23_2024.8.7'
    sites = [
        'https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0'
    ]

    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': sites,
            'sha256sum': sha256sum,
            'filename': filename,
            'path': path,
            'check_files': [
                check_file
            ]
        }
    ]


