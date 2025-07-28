
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    url = "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip"
    sha256sum = "eba3f3d414d2f8cba2919c78ec6daab08fc71ba2ba4ae502b7e5d4d99fc02cda"
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


