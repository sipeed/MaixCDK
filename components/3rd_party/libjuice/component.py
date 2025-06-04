
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_LIBJUICE_VERSION_MAJOR']}.{confs['CONFIG_LIBJUICE_VERSION_MINOR']}.{confs['CONFIG_LIBJUICE_VERSION_PATCH']}.{confs['CONFIG_LIBJUICE_COMPILED_VERSION']}"
    url = f"https://github.com/paullouisageneau/libjuice/archive/refs/tags/v{version}.tar.gz"
    if version == "1.5.8":
        sha256sum = "aa81809384c7e2594853304034a60fa2c2a234483b31cb531a4fc19e5877b709"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/paullouisageneau/libjuice"]
    filename = f"libjuice-{version}.tar.gz"
    path = f"libjuice_srcs"
    check_file = f'libjuice-{version}'
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


