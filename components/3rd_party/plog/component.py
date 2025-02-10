
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "1.1.10"
    url = f"https://github.com/SergiusTheBest/plog/archive/refs/tags/{version}.tar.gz"
    if version == "1.1.10":
        sha256sum = "55a090fc2b46ab44d0dde562a91fe5fc15445a3caedfaedda89fe3925da4705a.."
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/SergiusTheBest/plog"]
    filename = f"plog-{version}.tar.gz"
    path = f"plog_srcs"
    check_file = 'plog'
    rename = {
        f'plog-{version}': 'plog'
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


