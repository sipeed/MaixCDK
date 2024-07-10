
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_FFMPEG_VERSION_MAJOR']}.{confs['CONFIG_FFMPEG_VERSION_MINOR']}.{confs['CONFIG_FFMPEG_VERSION_PATCH']}.{confs['CONFIG_FFMPEG_COMPILED_VERSION']}"
    url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/ffmpeg_libs_n{version}.tar.xz"
    if version == "4.4.4.1":
        sha256sum = "d4b8eb70a7864c855e8e286fef39b9b90faf72c1b77491148c451da252cad6c0"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"ffmpeg_libs_n{version}.tar.xz"
    path = f"ffmpeg_srcs"
    check_file = 'ffmpeg'
    rename = {
        f'ffmpeg_libs_n{version}': 'ffmpeg'
    }

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
            ],
            'rename': rename
        }
    ]


