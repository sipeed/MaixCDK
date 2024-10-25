
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_QUIRC_VERSION_MAJOR']}.{confs['CONFIG_QUIRC_VERSION_MINOR']}"
    url = f"https://github.com/dlbeer/quirc/archive/refs/tags/v{version}.tar.gz"
    if version == "1.2":
        sha256sum = "73c12ea33d337ec38fb81218c7674f57dba7ec0570bddd5c7f7a977c0deb64c5"
    else:
        raise Exception(f"version {version} not support")
    filename = f"quirc-{version}.tar.gz"
    path = "quirc_srcs"
    check_file = f'quirc-{version}'
    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': [],
            'sha256sum': sha256sum,
            'filename': filename,
            'path': path,
            'check_files': [
                check_file
            ]
        }
    ]


