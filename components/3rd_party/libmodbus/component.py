
# def add_file_downloads(confs : dict) -> list:
#     '''
#         @param confs kconfig vars, dict type
#         @return list type, items is dict type
#     '''
#     libname = 'libmodbus'
#     libversion = '3.1.10'
#     libfullname = f'{libname}-{libversion}'
#     url = f"http://127.0.0.1:8000/{libfullname}.tar.gz"
#     sha256sum = "083bce8a63120d659524ac8ede1cd88f9f883ea9774ad31f9f761953857664eb"
#     sites = ["http://127.0.0.1:8000/"]
#     filename = f"{libfullname}.tar.gz"
#     path = f"libmodbus_srcs"
#     check_file = libname
#     rename = {
#         libfullname : 'libmodbus'
#     }

#     return [
#         {
#             'url': f'{url}',
#             'urls': [],
#             'sites': sites,
#             'sha256sum': sha256sum,
#             'filename': filename,
#             'path': path,
#             'check_files': [
#                 check_file
#             ],
#             'rename': rename
#         }
#     ]

def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    libname = 'libmodbus'
    libversion = '3.1.10'
    libfullname = f'{libname}-{libversion}'
    url = f"https://github.com/stephane/libmodbus/archive/refs/tags/v{libversion}.tar.gz"
    sha256sum = "e93503749cd89fda4c8cf1ee6371a3a9cc1f0a921c165afbbc4fd96d4813fa1a"
    sites = ["https://github.com/stephane/libmodbus/archive/refs/tags/"]
    filename = f"v{libversion}.tar.gz"
    path = f"libmodbus_srcs"
    check_file = libname
    rename = {
        libfullname : 'libmodbus'
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

