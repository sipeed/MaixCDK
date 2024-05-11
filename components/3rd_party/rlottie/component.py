
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    # version = f"{confs['CONFIG_OMV_VERSION_MAJOR']}.{confs['CONFIG_OMV_VERSION_MINOR']}.{confs['CONFIG_OMV_VERSION_PATCH']}"
    version = "0.2"
    url = f"https://github.com/Samsung/rlottie/archive/refs/tags/v{version}.tar.gz"
    if version == "0.2":
        sha256sum = "030ccbc270f144b4f3519fb3b86e20dd79fb48d5d55e57f950f12bab9b65216a"
    else:
        raise Exception(f"version {version} not support")
    sites = []
    filename = f"rlottie-{version}.tar.gz"
    path = f"rlottie"
    check_file = f'rlottie-{version}'
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


