
# def add_file_downloads(confs : dict) -> list:
#     '''
#         @param confs kconfig vars, dict type
#         @return list type, items is dict type
#     '''
#     version = f"{confs['CONFIG_ZBAR_VERSION_MAJOR']}.{confs['CONFIG_ZBAR_VERSION_MINOR']}"
#     url = f"https://github.com/ZBar/ZBar/archive/refs/tags/{version}.tar.gz"
#     if version == "0.10":
#         sha256sum = "de0d19b3edf53e85b3bb8fdf09a800aec960d3811185f59e2fb964bf7534ce01"
#     else:
#         raise Exception(f"version {version} not support")
#     filename = f"ZBar-{version}.tar.gz"
#     path = "zbar_srcs"
#     check_file = f'ZBar-{version}'
#     return [
#         {
#             'url': f'{url}',
#             'urls': [],
#             'sites': [],
#             'sha256sum': sha256sum,
#             'filename': filename,
#             'path': path,
#             'check_files': [
#                 check_file
#             ]
#         }
#     ]


