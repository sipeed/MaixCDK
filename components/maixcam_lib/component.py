
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_SOPHGO_MIDDLEWARE_VERSION_MAJOR']}.{confs['CONFIG_SOPHGO_MIDDLEWARE_VERSION_MINOR']}.{confs['CONFIG_SOPHGO_MIDDLEWARE_VERSION_PATCH']}"
    if version == "0.0.4":
        url = "https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/sophgo-middleware-0.0.4.tar.xz"
        sha256sum = "e239b4be072c3835962a8f73d24dcc88f9258ae9d90edae94419b39823dc4c14"
    else:
        raise Exception(f"version {version} not support")
    filename = f"sophgo-middleware-${version}.tar.xz"

    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': [],
            'sha256sum': sha256sum,
            'filename': filename,
            'path': 'sophgo-middleware',
            'check_files': [
                f'sophgo-middleware-{version}'
            ]
        }
    ]


