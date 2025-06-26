
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"v{confs['CONFIG_MAIXCAM2_MSP_VERSION_MAJOR']}.{confs['CONFIG_MAIXCAM2_MSP_VERSION_MINOR']}.{confs['CONFIG_MAIXCAM2_MSP_VERSION_PATCH']}_{confs['CONFIG_MAIXCAM2_MSP_COMPILED_VERSION']}"
    url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/maixcam2_msp_arm64_glibc_{version}.tar.xz"
    if version == "v3.0.0_20241120230136":
        sha256sum = "3315bde03839e8451a864f79e0f1b1252703af821dd5d2b0418b1535a5396c84"
    elif version == "v3.0.0_20250319114413":
        sha256sum = "20ab34bded456d8328b825bb2cc842937651596ea43f7075de95be7ced9a953d"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"maixcam2_msp_arm64_glibc_{version}.tar.xz"
    path = f"maixcam2_msp_srcs"
    check_file = f'maixcam2_msp_arm64_glibc_{version}'
    rename = {
            f'maixcam2_msp': check_file
        }

    return [
        {
            'url': url,
            'urls': [],
            'sites': sites,
            'sha256sum': sha256sum,
            'filename': filename,
            'path': path,
            'check_files': [
                check_file
            ],
            'rename': rename
        },
    ]


