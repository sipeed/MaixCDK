import argparse
from .version import __version__
import os
from .app_deploy import serve
from .app_release import pack
from .i18n import main as i18n_main

def deploy(port, pkg_path):
    if not pkg_path:
        pkg_path = pack(os.getcwd())
        print("-- release complete, file:{}".format(pkg_path))
    elif not os.path.exists(pkg_path):
        raise Exception(f"pakcage path {pkg_path} not exists")
    else:
        print(f"-- package path: {pkg_path}")
    pkg_path = os.path.abspath(pkg_path)
    serve(pkg_path, port)

def main():
    parser = argparse.ArgumentParser(description='maixtool, tools for maix series development')
    parser.add_argument('-v', '--version', action='version', version=__version__)
    subparsers = parser.add_subparsers(help='command help', dest="cmd")
    # release
    parser_release = subparsers.add_parser("release", help="release APP")
    # deploy
    parser_deploy = subparsers.add_parser("deploy", help="release APP")
    parser_deploy.add_argument('-p', "--port", default=8888, type=int, help="deploy server port")
    parser_deploy.add_argument("--pkg", default="", type=str, help="app package path")
    # i18n
    parser_i18n  = subparsers.add_parser("i18n", help="release APP")
    parser_i18n.add_argument("-k", "--keywords", nargs="+", type=str, default=["_", "tr"], help="translate function keywords to search")
    parser_i18n.add_argument("-r", action="store_true", help="recursive search dir")
    parser_i18n.add_argument("-e", "--exts", nargs="+", type=str, default=[".c", ".cpp", ".h", ".hpp", ".py"], help="file extention to search")
    parser_i18n.add_argument("-l", "--locales", nargs="+", type=str, default=["en", "zh"], help="locals of region, like en zh ja etc. the locale name can be found in [here](https://www.science.co.il/language/Locale-codes.php) or [wikipedia](https://en.wikipedia.org/wiki/Language_localisation), all letters use lower case.")
    parser_i18n.add_argument("-o", "--out", type=str, default="locales", help="translation files output directory")
    parser_i18n.add_argument("-d", "--dir", type=str, help="where to search", required=True)

    args = parser.parse_args()
    if args.cmd == "release":
        pack(os.getcwd())
    elif args.cmd == "deploy":
        deploy(args.port, args.pkg)
    elif args.cmd == "i18n":
        i18n_main(args.dir, args.keywords, args.exts, args.out, args.r, args.locales)

