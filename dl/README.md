dl
====

Downloaded files should be put in pkgs directory.

Extracted files should be put in extracted directory.

so:
* If you are developing a package, and need data files, cause only code should be add to git repo, you should put the data files on internet, and download them is `dl/pkgs` directory, to use them, extract them to `dl/extracted` directory. You just need edit the `components.py` which next to `CMakeLists.txt` file, add `add_file_downloads` function, this will tell maixcdk to download and extract them before compile, so you can use the extracted files with `${DL_EXTRACTED_PATH}/your_path` in your `CMakeLists.txt`
> e.g. For python packages, will be downloaded to `dl/pkgs/python_srcs`, and extracted to `dl/extracted/python_srcs`, which download commands in CMakelists.txt.

`add_file_downloads` example:
```python

def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = f"{confs['CONFIG_MEDIA_SERVER_VERSION_MAJOR']}.{confs['CONFIG_MEDIA_SERVER_VERSION_MINOR']}.{confs['CONFIG_MEDIA_SERVER_VERSION_PATCH']}"
    if version == "1.0.0":
        url = "https://master.dl.sourceforge.net/projectxxxxxx.zip"
        sha256sum = "dfe405cedbf3969fbf1f252634e7b1c71e8bb3e2d0857db2fe7e70189474ecd1"
    elif version == "1.0.1":
        url = "https://master.dl.sourceforge.net/projectxxxxxx1.0.1.zip"
    else:
        raise Exception(f"version {version} not support")
    filename = f"media_server-{version}.zip"

    return [
        {
            'url': f'{url}',
            'urls': [],
            'sites': [],
            'sha256sum': sha256sum,
            'filename': filename,
            'path': 'media_server',
            'check_files': [
                f'media_server-{version}'
            ]
        }
    ]
```

* If you users to compile projects, you can download files manually to dl/pkgs directory first, when compile `pkgs_info.json` will be write to this directory, all files need to be downloaded will be listed in this file.

