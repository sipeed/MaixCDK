
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    url = "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip"
    sha256sum = "1ccaabbfe870f60af3d6a519c53e09f3dcf630207321dffa553564a8e75c4fc8"
    filename = "eigen-3.4.0.zip"

    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': ['https://eigen.tuxfamily.org/index.php'],
            'sha256sum': sha256sum,
            'filename': filename,
            'path': 'eigen',
            'check_files': [
                'eigen-3.4.0'
            ]
        }
    ]


