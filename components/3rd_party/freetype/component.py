
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_FREETYPE_VERSION_MAJOR']}.{confs['CONFIG_FREETYPE_VERSION_MINOR']}.{confs['CONFIG_FREETYPE_VERSION_PATCH']}"
    url = f"https://phoenixnap.dl.sourceforge.net/project/freetype/freetype2/{version}/freetype-{version}.tar.xz"
    if version == "2.13.2":
        sha256sum = "12991c4e55c506dd7f9b765933e62fd2be2e06d421505d7950a132e4f1bb484d"
    else:
        raise Exception(f"version {version} not support")
    filename = f"freetype-{version}.tar.xz"
    path = "freetype_srcs"
    check_file = f'freetype-{version}'

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


