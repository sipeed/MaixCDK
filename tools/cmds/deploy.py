#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
#

import argparse
import os
import sys
import subprocess
import yaml
from maixtool.app_deploy import serve

parser = argparse.ArgumentParser(prog="deploy", description="deploy program to device", add_help=False)

############################### Add option here #############################
parser.add_argument("-p", "--port", default=8888, type=int, help="deploy server port")
parser.add_argument("--skip-build", action="store_true", help="skip build only serve")

#############################################################################

# use project_args created by SDK_PATH/tools/cmake/project.py, e.g. project_args.terminal

def get_app_path():
    '''
        current path is project path
    '''
    files = os.listdir("dist")
    # find release_info_vx.x.x.yaml, and sort desc, get first one
    release_info_files = [f for f in files if f.startswith("release_info_") and f.endswith(".yaml")]
    if len(release_info_files) == 0:
        raise ValueError("release_info_vx.x.x.yaml not found")
    release_info_files.sort(reverse=True)
    release_info_path = os.path.join("dist", release_info_files[0])
    if not os.path.exists(release_info_path):
        raise ValueError("release_info.yaml not found")
    with open(release_info_path, "r") as f:
        info = yaml.load(f, Loader=yaml.FullLoader)
        return os.path.abspath(info["path"])

def main(vars):
    '''
        @vars: dict,
            "project_path": project_path,
            "project_id": project_id,
            "sdk_path": sdk_path,
            "build_type": build_type,
            "project_parser": project_parser,
            "project_args": project_args,
            "configs": configs,
    '''
    # release program first
    if not vars["project_args"].skip_build:
        cmd = ["maixcdk", "release"]
        exit_code = subprocess.Popen(cmd).wait()
        if exit_code != 0:
            print("[ERROR] release exit with code:{}".format(exit_code))
            sys.exit(1)
    # serve deployment program
    release_path = get_app_path()
    # show qr code of url
    local_port = int(vars["project_args"].port) if type(vars["project_args"].port) == int else 8888
    serve(release_path, local_port)


# args = parser.parse_args()
if __name__ == '__main__':
    main()
