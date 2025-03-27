
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''

    return [
        {
            'url': f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/maixcam2_msp_arm64_glibc_v3.0.0_20241120230136.tar.xz",
            'urls': [],
            'sites': ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"],
            'sha256sum': "4fb40c4ee1e75cbf673dc4f3a8101a79e287515acb151fdf3b889207eaf4d526",
            'filename': f"maixcam2_msp_arm64_glibc_v3.0.0_20241120230136.tar.xz",
            'path': f"maixcam2_msp_srcs",
            'check_files': [
                'maixcam2_msp'
            ],
            'rename': {
                f'maixcam2_msp_arm64_glibc_v3.0.0_20241120230136': 'maixcam2_msp'
            }
        },
    ]


