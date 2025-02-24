
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    return [
        {
            'url': f"https://github.com/AXERA-TECH/ax620e_bsp_sdk/archive/refs/tags/v2.0.0_P7.tar.gz",
            'urls': [],
            'sites': ["https://github.com/AXERA-TECH/ax620e_bsp_sdk"],
            'sha256sum': "adfe5fda41b1aac131a2b2618a823b9c5041c486dfa725ce7fc0e9c1331cca65",
            'filename': f"ax620e_bsp_sdk-2.0.0_P7.tar.gz",
            'path': f"ax620e_bsp_sdk_srcs",
            'check_files': [
                'ax620e_bsp_sdk'
            ],
            'rename': {
                f'ax620e_bsp_sdk-2.0.0_P7': 'ax620e_bsp_sdk'
            }
        },
        {
            'url': f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/ax630c-third-party_2.0.0.tar.gz",
            'urls': [],
            'sites': ["https://github.com/AXERA-TECH/ax620e_bsp_sdk"],
            'sha256sum': "9c9b25aec80e1195fabe35b29f74ca588f6dec0370dfa7f2c6f1c5020ee89547",
            'filename': f"ax630c-third-party_2.0.0.tar.gz",
            'path': f"ax620e_bsp_sdk_srcs/ax620e_bsp_sdk",
            'check_files': [
                'third-party'
            ],
            'rename': {}
        }
    ]


