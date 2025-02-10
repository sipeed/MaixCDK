
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "3.11.3"
    url = f"https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz"
    if version == "3.11.3":
        sha256sum = "d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d."
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/nlohmann/json"]
    filename = f"json.tar.xz"
    path = f"json_srcs"
    check_file = 'json'
    rename = {
        f'json': 'json'
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


