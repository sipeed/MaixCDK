
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    if confs.get("CONFIG_COMPONENTS_COMPILE_FROM_SOURCE", None) or confs.get("CONFIG_PYTHON_COMPILE_FROM_SOURCE", None):
        version = f"{confs['CONFIG_PYTHON_VERSION_MAJOR']}.{confs['CONFIG_PYTHON_VERSION_MINOR']}.{confs['CONFIG_PYTHON_VERSION_PATCH']}"
        if confs.get("PLATFORM_MAIXCAM", None):
            version = "3.11.6"
        elif confs.get("PLATFORM_MAIXCAM2", None):
            version = "3.13.2"
        if version == "0.0.0":
            return []
        url = f"https://www.python.org/ftp/python/{version}/Python-{version}.tar.xz"
        if version == "3.11.6":
            sha256sum = "92e14b22e708612c6e280931cc247b4266da9a6bac8459edf25bfb4cebcbac66"
        elif version == "3.13.2":
            sha256sum = "d984bcc57cd67caab26f7def42e523b1c015bbc5dc07836cf4f0b63fa159eb56"
        else:
            raise Exception(f"version {version} not support, please sha256sum of {url} add in 3rd_party/python3/component.py")
        sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
        filename = f"Python-{version}.tar.xz"
        path = f"python_srcs"
        check_file = f'Python-{version}'
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
            },
            {
                'url': 'https://zlib.net/zlib-1.3.1.tar.xz',
                'urls': [],
                'sites': ['https://zlib.net/'],
                'sha256sum': '38ef96b8dfe510d42707d9c781877914792541133e1870841463bfa73f883e32',
                'filename': 'zlib-1.3.1.tar.xz',
                'path': 'zlib'
            },
            {
                'url': 'https://www.openssl.org/source/openssl-3.0.12.tar.gz',
                'urls': [],
                'sites': ['https://www.openssl.org/source/'],
                'sha256sum': 'f93c9e8edde5e9166119de31755fc87b4aa34863662f67ddfcba14d0b6b69b61',
                'filename': 'openssl-3.0.12.tar.gz',
                'path': 'openssl'
            }
        ]
    elif confs.get("PLATFORM_MAIXCAM", None):
        if "musl" not in confs["CONFIG_TOOLCHAIN_PATH"]:
            return []
        # version = f"{confs['CONFIG_OMV_VERSION_MAJOR']}.{confs['CONFIG_OMV_VERSION_MINOR']}.{confs['CONFIG_OMV_VERSION_PATCH']}"
        version = "3.11.6"
        url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/python3_lib_maixcam_musl_3.11.6.tar.xz"
        if version == "3.11.6":
            sha256sum = "92e14b22e708612c6e280931cc247b4266da9a6bac8459edf25bfb4cebcbac66"
        else:
            raise Exception(f"version {version} not support")
        sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
        filename = f"python3_lib_maixcam_musl_3.11.6.tar.xz"
        path = f"python3"
        check_file = f'python3_lib_maixcam_musl_3.11.6'
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
    elif confs.get("PLATFORM_MAIXCAM2", None):
        filename = "python3.13.2_maixcam2_gcc11.4.0.tar.xz"
        sha256 = "a37165fddf7401b3932b94a10e9cbb166d9ee825e34e23a27de2dadcacce411a"
        check_file = "bin"
        return [{
                'url': f'https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/{filename}',
                'urls': [],
                'sites': ['https://github.com/sipeed/MaixCDK/releases/download/v0.0.0'],
                'sha256sum': sha256,
                'filename': filename,
                'path': 'python3/python3.13_maixcam2',
                'check_files': [
                    check_file
                ],
            }]
    return []
