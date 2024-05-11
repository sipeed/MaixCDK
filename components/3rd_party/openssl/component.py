
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    # version = f"{confs['CONFIG_OMV_VERSION_MAJOR']}.{confs['CONFIG_OMV_VERSION_MINOR']}.{confs['CONFIG_OMV_VERSION_PATCH']}"
    version = "3.0.12"
    url = f"https://www.openssl.org/source/openssl-3.0.12.tar.gz"
    if version == "3.0.12":
        sha256sum = "f93c9e8edde5e9166119de31755fc87b4aa34863662f67ddfcba14d0b6b69b61"
    else:
        raise Exception(f"version {version} not support")
    sites = ['https://www.openssl.org/source/']
    filename = f"openssl-3.0.12.tar.gz"
    path = f"openssl"
    check_file = f'openssl-{version}'
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


