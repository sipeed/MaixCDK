# def add_file_downloads(confs : dict) -> list:
#     '''
#         @param confs kconfig vars, dict type
#         @return list type, items is dict type
#     '''
#     if confs.get('PLATFORM_MAIXCAM', None):
#         version = f"1.20.1"
#         url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/sg2002_onnxruntime_v{version}.tar.xz"
#         if version == "1.20.1":
#             sha256sum = "03f78241122953d214bbf4696880c8d41f8d1e12785b0d49c749beb9644ab144"
#         else:
#             raise Exception(f"version {version} not support")
#         sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
#         filename = f"sg2002_onnxruntime_v{version}.tar.xz"
#         path = f"onnxruntime_srcs"
#         check_file = f'maixcam_onnxruntime'
#         rename = {
#             f'sg2002_onnxruntime_v{version}': check_file
#         }

#         return [
#             {
#                 'url': f'{url}',
#                 'urls': [],
#                 'sites': sites,
#                 'sha256sum': sha256sum,
#                 'filename': filename,
#                 'path': path,
#                 'check_files': [
#                     check_file
#                 ],
#                 'rename': rename
#             }
#         ]
#     elif confs.get('PLATFORM_MAIXCAM', None):
#         version = f"1.22.0"
#         url = f"https://github.com/sipeed/MaixCDK/releases/download/v0.0.0/maixcam2_onnxruntime_v{version}.tar.xz"
#         if version == "1.22.0":
#             sha256sum = "03f78241122953d214bbf4696880c8d41f8d1e12785b0d49c749beb9644ab144"
#         else:
#             raise Exception(f"version {version} not support")
#         sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
#         filename = f"maixcam2_onnxruntime_v{version}.tar.xz"
#         path = f"onnxruntime_srcs"
#         check_file = f'maixcam2_onnxruntime'
#         rename = {
#             f'maixcam2_onnxruntime_v{version}': check_file
#         }

#         return [
#             {
#                 'url': f'{url}',
#                 'urls': [],
#                 'sites': sites,
#                 'sha256sum': sha256sum,
#                 'filename': filename,
#                 'path': path,
#                 'check_files': [
#                     check_file
#                 ],
#                 'rename': rename
#             }
#         ]

