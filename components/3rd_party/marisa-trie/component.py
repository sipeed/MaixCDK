
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_MARISA_TRIE_VERSION_MAJOR']}.{confs['CONFIG_MARISA_TRIE_VERSION_MINOR']}.{confs['CONFIG_MARISA_TRIE_VERSION_PATCH']}"
    url = f"https://github.com/s-yata/marisa-trie/archive/refs/tags/v{version}.zip"
    if version == "0.2.6":
        sha256sum = "8dc0b79ff9948be80fd09df6d2cc70134367339ec7d6496857bc47cf421df1af"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/s-yata/marisa-trie"]
    filename = f"marisa-trie-{version}.zip"
    path = f"marisa_trie_srcs"
    check_file = f"marisa-trie-{version}"
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


