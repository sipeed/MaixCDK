import argparse
from .version import __version__
import os
from .app_deploy import serve
from .app_release import pack

def deploy(port):
    path = pack(os.getcwd())
    print("-- release complete, file:{}".format(path))
    serve(path, port)

def main():
    cmds = ["release", "deploy"]
    parser = argparse.ArgumentParser(description='maixtool, tools for maix series development')
    parser.add_argument('-v', '--version', action='version', version=__version__)
    parser.add_argument('-p', "--port", default=8888, type=int, help="deploy server port")
    parser.add_argument("command", help="command to run", choices=cmds)
    args = parser.parse_args()
    if args.command == "release":
        pack(os.getcwd())
    elif args.command == "deploy":
        deploy(args.port)

