
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''

    return [
        {
            'url': f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/ax620e_msp_arm64_glibc_v3.0.0_20241120230136.tar.xz",
            'urls': [],
            'sites': ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"],
            'sha256sum': "ce07ad0bd4301bb9659a6d0353120b1c9772db832c5ceb5c0a31aab4943b8627",
            'filename': f"ax620e_msp_arm64_glibc_v3.0.0_20241120230136.tar.xz",
            'path': f"ax620e_msp_srcs",
            'check_files': [
                'ax620e_msp'
            ],
            'rename': {
                f'ax620e_msp_arm64_glibc_v3.0.0_20241120230136': 'ax620e_msp'
            }
        },
    ]


