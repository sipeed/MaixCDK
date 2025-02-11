
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "0.22.4"
    url = f"https://github.com/paullouisageneau/libdatachannel/archive/refs/tags/v{version}.tar.gz"
    if version == "0.22.4":
        sha256sum = "43eff11d71382aa1e0249aed9729cd744fa91e5cf2410364efa37e006a620454."
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/paullouisageneau/libdatachannel"]
    filename = f"libdatachannel-{version}.tar.gz"
    path = f"libdatachannel_srcs"
    check_file = 'libdatachannel'
    rename = {
        f'libdatachannel-{version}': 'libdatachannel'
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


