def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_ONNXRUNTIME_VERSION_MAJOR']}.{confs['CONFIG_ONNXRUNTIME_VERSION_MINOR']}.{confs['CONFIG_ONNXRUNTIME_VERSION_PATCH']}"
    if confs.get('PLATFORM_MAIXCAM', None):
        url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/sg2002_onnxruntime_v{version}.tar.xz"
        if version == "1.20.1":
            sha256sum = "03f78241122953d214bbf4696880c8d41f8d1e12785b0d49c749beb9644ab144"
        else:
            raise Exception(f"version {version} not support")
        sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
        filename = f"sg2002_onnxruntime_v{version}.tar.xz"
        path = f"onnxruntime_srcs"
        check_file = f'maixcam_onnxruntime'
        rename = {
            f'sg2002_onnxruntime_v{version}': check_file
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
        url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/maixcam2_onnxruntime_v{version}.tar.xz"
        if version == "1.22.0":
            sha256sum = "e1a7fc0754105d9e9651302bd1d7e44d3efab4c0541409f6f9995e91f845b739"
        else:
            raise Exception(f"version {version} not support")
        sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
        filename = f"maixcam2_onnxruntime_v{version}.tar.xz"
        path = f"onnxruntime_srcs"
        check_file = f'maixcam2_onnxruntime_v{version}'
        rename = {}

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

