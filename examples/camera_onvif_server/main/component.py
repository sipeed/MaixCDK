
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version0 = f"{confs['CONFIG_GSOAP_VERSION_MAJOR']}.{confs['CONFIG_GSOAP_VERSION_MINOR']}"
    version = f"{confs['CONFIG_GSOAP_VERSION_MAJOR']}.{confs['CONFIG_GSOAP_VERSION_MINOR']}.{confs['CONFIG_GSOAP_VERSION_PATCH']}"
    url = f"https://github.com/SrcBackup/gsoap/releases/download/v2.8.x/gsoap_{version}.zip"
    sha256sum = "bcc77bc8843ae00091d40bbfaee5071665ca2793ac1e8f05de2e2d54a84a7ddb"
    filename = f"gsoap_{version}.zip"
    path = "gsoap_srcs"
    check_file = f'gsoap-{version0}'
    sites = []

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
            ]
        }
    ]


