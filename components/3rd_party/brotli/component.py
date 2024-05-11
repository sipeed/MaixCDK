
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_BROTLI_VERSION_MAJOR']}.{confs['CONFIG_BROTLI_VERSION_MINOR']}.{confs['CONFIG_BROTLI_VERSION_PATCH']}"
    url = f"https://master.dl.sourceforge.net/project/brotli.mirror/v{version}/v{version}.tar.gz?viasf=1"
    if version == "1.1.0":
        sha256sum = "10973f4b4199eafa1d5735ef661ddb2ec2f97319ee9fd1824d4aabe08cff5265"
    else:
        raise Exception(f"version {version} not support")
    filename = f"brotli-{version}.tar.gz"
    path = "brotli_srcs"
    check_file = f'google-brotli-{version}'

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


