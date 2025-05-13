
# def add_file_downloads(confs : dict) -> list:
#     '''
#         @param confs kconfig vars, dict type
#         @return list type, items is dict type
#     '''
#     version = "1.1.0"
#     url = f"https://github.com/Tencent/rapidjson/archive/refs/tags/v{version}.zip"
#     if version == "1.1.0":
#         sha256sum = "d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d."
#     else:
#         raise Exception(f"version {version} not support")
#     sites = ["https://github.com/Tencent/rapidjson"]
#     filename = f"rapidjson-{version}.zip"
#     path = f"rapidjson_srcs"
#     check_file = 'rapidjson'
#     rename = {
#         f"rapidjson-{version}.zip": 'rapidjson'
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


