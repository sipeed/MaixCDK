
def add_file_downloads(confs : dict) -> list:
    '''
        @param confs kconfig vars, dict type
        @return list type, items is dict type
    '''
    version = "0.8.2"
    url = "https://master.dl.sourceforge.net/project/websocket.mirror/0.8.2/WebSocket%2B%2B_0.8.2.tar.gz?viasf=1"
    if version == "0.8.2":
        sha256sum = "8b1773ea2832751071ac19d2708314d68316dd3916434c7dc0dd58cef14d51cd"
    else:
        raise Exception(f"version {version} not support")
    sites = ["https://github.com/sipeed/MaixCDK/releases/tag/v0.0.0"]
    filename = "WebSocketpp_0.8.2.tar.gz"
    path = "websocketpp"
    check_file = 'websocketpp-0.8.2'
    rename = {'zaphoyd-websocketpp-0e42417': 'websocketpp-0.8.2'}

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


