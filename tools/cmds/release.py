#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
#

import argparse
import os
import subprocess
import sys
from maixtool.app_release import pack

parser = argparse.ArgumentParser(prog="release", description="release program as package file", add_help=False)

############################### Add option here #############################
parser.add_argument("-p", "--platform", help="Select platforms, use maixcdk build --help to see supported platforms", default="")

#############################################################################

# use project_args created by SDK_PATH/tools/cmake/project.py, e.g. project_args.terminal

def get_main_sh_content(app_id):
    content = '''#!/bin/bash
export LD_LIBRARY_PATH=./dl_lib:$LD_LIBRARY_PATH
chmod +x ./{app_id}
./{app_id} $@

'''
    return content.format(app_id=app_id)

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

    print("\n--------- rebuild start ------------")
    # rebuild program
    cmd = ["maixcdk", "build", "--release"]
    if vars["project_args"].platform:
        cmd.append("--platform")
        cmd.append(vars["project_args"].platform)
    exit_code = subprocess.Popen(cmd).wait()
    print("--------- rebuild complete ------------\n")
    if exit_code != 0:
        print("[ERROR] rebuild exit with code:{}".format(exit_code))
        sys.exit(1)
    exec_path = os.path.join(vars["project_path"], "build", vars["project_id"])
    dl_lib_dir = os.path.join(vars["project_path"], "build", "dl_lib")
    files = {}
    if os.path.exists(dl_lib_dir):
        files[dl_lib_dir] = "dl_lib"
    zip_path = pack(vars["project_path"], exec_path, extra_files = files)
    print("-- release complete, file:{}".format(zip_path))


# args = parser.parse_args()
if __name__ == '__main__':
    main()
