
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "0.9.5.0"
    url = f"https://github.com/sctplab/usrsctp/archive/refs/tags/{version}.tar.gz"
    if version == "0.9.5.0":
        sha256sum = "260107caf318650a57a8caa593550e39bca6943e93f970c80d6c17e59d62cd92."
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sctplab/usrsctp"]
    filename = f"usrsctp-{version}.tar.gz"
    path = f"usrsctp_srcs"
    check_file = 'usrsctp'
    rename = {
        f'usrsctp-{version}': 'usrsctp'
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


