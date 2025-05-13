
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "1.1.9"
    url = f"https://github.com/BYVoid/OpenCC/archive/refs/tags/ver.{version}.zip"
    if version == "1.1.9":
        sha256sum = "f8aac3eda054edaf0313aa2103baa965f63f80f4a496b35ac7b632dfd1a33953"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/BYVoid/OpenCC"]
    filename = f"OpenCC-ver.{version}.zip"
    path = f"opencc_srcs"
    check_file = 'OpenCC'
    rename = {
        f"OpenCC-ver.{version}": 'OpenCC'
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


