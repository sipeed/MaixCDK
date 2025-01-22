
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "e43025bbf"
    url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/tinyalsa-master-{version}.zip"
    if version == "e43025bbf":
        sha256sum = "71b6d0b64a5875b0e5f6d2af7dbe5ff2d5db75d5ec180a157e14fd32d7faf82e"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"tinyalsa-master-{version}.zip"
    path = "tinyalsa_srcs"
    check_file = f'tinyalsa'
    rename = {f'tinyalsa-master': 'tinyalsa'}

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


