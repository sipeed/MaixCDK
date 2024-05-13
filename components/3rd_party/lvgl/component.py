
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_LVGL_VERSION_MAJOR']}.{confs['CONFIG_LVGL_VERSION_MINOR']}.{confs['CONFIG_LVGL_VERSION_PATCH']}"
    url = f"https://github.com/lvgl/lvgl/archive/refs/tags/v{version}.tar.gz"
    if version == "9.1.0":
        sha256sum = "6930f1605d305fcd43f31d5f470ecf4a013c4ce0980e78ee4c33b96a589bf433"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"lvgl-{version}.tar.gz"
    path = f"lvgl_srcs/lvgl-{version}"
    check_file = 'lvgl'
    rename = {
        f'lvgl-{version}': 'lvgl'
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


