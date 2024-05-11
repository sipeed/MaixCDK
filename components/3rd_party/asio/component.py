
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    url = "https://phoenixnap.dl.sourceforge.net/project/asio/asio/1.28.0%20%28Stable%29/asio-1.28.0.tar.gz"
    sha256sum = "e854a53cc6fe599bdf830e3c607f1d22fe74eee892f64c81d3ca997a80ddca97"
    filename = "asio-1.28.0.tar.gz"

    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': ['https://sourceforge.net/projects/asio/files/asio/1.28.0%20%28Stable%29/'],
            'sha256sum': sha256sum,
            'filename': filename,
            'path': 'asio',
            'check_files': [
                'asio-1.28.0'
            ]
        }
    ]


