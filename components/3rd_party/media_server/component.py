
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_MEDIA_SERVER_VERSION_MAJOR']}.{confs['CONFIG_MEDIA_SERVER_VERSION_MINOR']}.{confs['CONFIG_MEDIA_SERVER_VERSION_PATCH']}"
    if version == "1.0.0":
        url = "https://files.catbox.moe/c7rklp.zip"
        sha256sum = "dfe405cedbf3969fbf1f252634e7b1c71e8bb3e2d0857db2fe7e70189474ecd1"
    elif version == "1.0.1":
        url = "https://files.catbox.moe/acqhv1.zip"
        sha256sum = "ce06dc3d03b6036165956e60afd5eec5bfd37e7746e4b427f2099732478ecc22"
    elif version == "1.0.2":
        url = "https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/media_server-1.0.2.zip"
        sha256sum = "b9872dbe52fae4d4b60a0db827533877b20186e9457525345407fdad8e187704"
    else:
        raise Exception(f"version {version} not support")
    filename = f"media_server-{version}.zip"

    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': [],
            'sha256sum': sha256sum,
            'filename': filename,
            'path': 'media_server',
            'check_files': [
                f'media_server-{version}'
            ]
        }
    ]


