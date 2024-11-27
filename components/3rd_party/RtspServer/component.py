
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_RTSP_SERVER_VERSION_MAJOR']}.{confs['CONFIG_RTSP_SERVER_VERSION_MINOR']}.{confs['CONFIG_RTSP_SERVER_VERSION_PATCH']}"
    url = f"https://github.com/PHZ76/RtspServer/archive/refs/tags/{version}.tar.gz"
    if version == "1.0.0":
        sha256sum = "7d21dd494380dc24006ca83b01aad05ca97734b052e3075299ebe222370531d3"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"RtspServer-{version}.tar.gz"
    path = f"RtspServer_srcs"
    check_file = 'RtspServer'
    rename = {
        f'RtspServer-{version}': 'RtspServer'
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


