import argparse
from .version import __version__
import os
from .app_deploy import serve
from .app_release import pack

def deploy(port, pkg_path):
    if not pkg_path:
        pkg_path = pack(os.getcwd())
        print("-- release complete, file:{}".format(pkg_path))
    elif not os.path.exists(pkg_path):
        raise Exception(f"pakcage path {pkg_path} not exists")
    else:
        print(f"-- package path: {pkg_path}")
    serve(pkg_path, port)

def main():
    cmds = ["release", "deploy"]
    parser = argparse.ArgumentParser(description='maixtool, tools for maix series development')
    parser.add_argument('-v', '--version', action='version', version=__version__)
    parser.add_argument('-p', "--port", default=8888, type=int, help="deploy server port")
    parser.add_argument("--pkg", default="", type=str, help="app package path")
    parser.add_argument("command", help="command to run", choices=cmds)
    args = parser.parse_args()
    if args.command == "release":
        pack(os.getcwd())
    elif args.command == "deploy":
        deploy(args.port, args.pkg)

