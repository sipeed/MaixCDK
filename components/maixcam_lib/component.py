
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_SOPHGO_MIDDLEWARE_VERSION_MAJOR']}.{confs['CONFIG_SOPHGO_MIDDLEWARE_VERSION_MINOR']}.{confs['CONFIG_SOPHGO_MIDDLEWARE_VERSION_PATCH']}"
    url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/sophgo-middleware-{version}.tar.xz"
    if version == "0.0.4":
        sha256sum = "e239b4be072c3835962a8f73d24dcc88f9258ae9d90edae94419b39823dc4c14"
    elif version == "0.0.5":
        sha256sum = "484dd9199f7d8d5d2af34bf76fd2b9e5feb3d47691308e491c68270844adffb9"
    elif version == "0.0.6":
        sha256sum = "e7a9f4fb6e3eeb791c8ebf0a19bbf660151deeffa977e99e1835fe1cbeb2aa0b"
    else:
        raise Exception(f"version {version} not support")
    filename = f"sophgo-middleware-{version}.tar.xz"

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


