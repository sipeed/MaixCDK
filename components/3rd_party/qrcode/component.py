
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    # version = f"{confs['CONFIG_OMV_VERSION_MAJOR']}.{confs['CONFIG_OMV_VERSION_MINOR']}.{confs['CONFIG_OMV_VERSION_PATCH']}"
    version = "2.3.0"
    url = f"https://github.com/zxing-cpp/zxing-cpp/archive/refs/tags/v{version}.tar.gz"
    if version == "2.3.0":
        sha256sum = "64e4139103fdbc57752698ee15b5f0b0f7af9a0331ecbdc492047e0772c417ba"
    else:
        raise Exception(f"version {version} not support")
    sites = ['https://www.openssl.org/source/']
    filename = f"zxing-cpp-{version}.tar.gz"
    path = f"zxing-cpp"
    check_file = f'zxing-cpp-{version}'
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


