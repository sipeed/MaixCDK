
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "1.20.1"
    url = "https://github.com/csukuangfj/kaldi-native-fbank/archive/refs/tags/v1.20.1.tar.gz"
    if version == "1.20.1":
        sha256sum = "170cb8c6b8b96891ebb2e3ddaa427cd5d81e40efc48501ac0bd9b24301c09856"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = f"kaldi-native-fbank-{version}.tar.gz"
    path = "kaldi-native-fbank_srcs"
    check_file = f'kaldi-native-fbank'
    rename = {f'kaldi-native-fbank-{version}': 'kaldi-native-fbank'}

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


