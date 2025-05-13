
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "1.1.0"
    url = f"https://github.com/Tencent/rapidjson/archive/refs/tags/v{version}.zip"
    if version == "1.1.0":
        sha256sum = "8e00c38829d6785a2dfb951bb87c6974fa07dfe488aa5b25deec4b8bc0f6a3ab"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/Tencent/rapidjson"]
    filename = f"rapidjson-{version}.zip"
    path = f"rapidjson_srcs"
    check_file = 'rapidjson'
    rename = {
        f"rapidjson-{version}": 'rapidjson'
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


