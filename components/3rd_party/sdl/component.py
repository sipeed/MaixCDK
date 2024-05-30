
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_SDL_VERSION_MAJOR']}.{confs['CONFIG_SDL_VERSION_MINOR']}.{confs['CONFIG_SDL_VERSION_PATCH']}"
    url = f"https://github.com/libsdl-org/SDL/releases/download/release-{version}/SDL2-{version}.tar.gz"
    if version == "2.28.4":
        sha256sum = "888b8c39f36ae2035d023d1b14ab0191eb1d26403c3cf4d4d5ede30e66a4942c"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/libsdl-org/SDL/releases"]
    filename = f"SDL2-{version}.tar.gz"
    path = "sdl_srcs"
    check_file = f'SDL2-{version}'
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


