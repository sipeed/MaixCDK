Simple websocket TCP proxy example.
========


A simple example of converting a WebSocket connection to a TCP connection.

**WebSocket Client** <---WebSocket---> **Proxy** <---TCP---> **TCP Server**

## Usage

`./websocket_tcp_proxy <tcp_server_ip> <tcp_server_port> <websocket_listening_port>`

e.g. `./websocket_tcp_proxy 127.0.0.1 5555 9000` will listen websocket connections on port 9000 and forward them to a TCP server at port 5555 on localhost.



