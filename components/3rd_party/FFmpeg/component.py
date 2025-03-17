
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_FFMPEG_VERSION_MAJOR']}.{confs['CONFIG_FFMPEG_VERSION_MINOR']}.{confs['CONFIG_FFMPEG_VERSION_PATCH']}.{confs['CONFIG_FFMPEG_COMPILED_VERSION']}"
    if confs.get('PLATFORM_MAIXCAM', None):
        url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/ffmpeg_libs_n{version}.tar.xz"
        if version == "4.4.4.1":
            sha256sum = "d4b8eb70a7864c855e8e286fef39b9b90faf72c1b77491148c451da252cad6c0"
        elif version == "4.4.4.2":
            sha256sum = "aa95faf66e92f9b244e5d48006ceee1a6e3cfb1122fe78f9fbe3a0cf6db7a7f2"
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
    elif confs.get('PLATFORM_MAIXCAM2', None):
        version = "6.1.1.1"
        url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/ffmpeg_ax630c_libs_n{version}.tar.xz"
        if version == "4.4.4.1":
            sha256sum = "3fcc394fad917127dd9503396dd84ce29fe47f33e76ebfd86dce6a73031e3a4b"
        elif version == "6.1.1.1":
            sha256sum = "fd2f693f94545e59e3dd51a211b818023c60c77c7f611e38e51003d0a321e2a7"
        else:
            raise Exception(f"version {version} not support")
        sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
        filename = f"ffmpeg_ax630c_libs_n{version}.tar.xz"
        path = f"ffmpeg_srcs"
        check_file = 'ffmpeg_ax630c_libs'
        rename = {
            f'ffmpeg_ax630c_libs_n{version}': 'ffmpeg_ax630c_libs'
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

