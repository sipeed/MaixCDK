
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "2.6.0"
    url = f"https://github.com/cisco/libsrtp/archive/refs/tags/v{version}.tar.gz"
    if version == "2.6.0":
        sha256sum = "bf641aa654861be10570bfc137d1441283822418e9757dc71ebb69a6cf84ea6b"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/cisco/libsrtp"]
    filename = f"libsrtp-{version}.tar.gz"
    path = f"libsrtp_srcs"
    check_file = 'libsrtp'
    rename = {
        f'libsrtp-{version}': 'libsrtp'
    }

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


