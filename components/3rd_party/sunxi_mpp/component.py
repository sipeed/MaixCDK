
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "1.0.1"
    url = "https://files.catbox.moe/e35tyl.xz"
    if version == "1.0.1":
        sha256sum = "5c20195c9600092b3cd79e7d96485920208e3ccfa56d1399f330493616b510a8"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"sunxi_mpp-{version}.tar.xz"
    path = "sunxi_mpp"
    check_file = f'sunxi-mpp-{version}'
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


