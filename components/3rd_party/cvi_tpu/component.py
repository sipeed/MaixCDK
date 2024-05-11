
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
    url = "https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/cvi_tpu_lib_v4.1.0-23-gb920beb.tar.xz"
    sha256sum = "8177ce35a05565a4fdc6cf1ac6bef0dc18809a31fff332321e17d01d2b85421a"
    filename = "cvi_tpu_lib_v4.1.0-23-gb920beb.tar.xz"
    path = "cvi_tpu"
    check_file = 'cvi_tpu_lib_v4.1.0-23-gb920beb'
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


