#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
#

import argparse
import os
import subprocess

parser = argparse.ArgumentParser(prog="run", description="run program", add_help=False)

############################### Add option here #############################
# parser.add_argument("-t", "--terminal", help="open terminal after flash ok", default=False, action="store_true")

#############################################################################

# use project_args created by SDK_PATH/tools/cmake/project.py, e.g. project_args.terminal

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
    exec_path = os.path.join(vars["project_path"], "build", vars["project_id"])
    lib_dir = os.path.join(vars["project_path"], "build", "dl_lib")
    print("exec_path:{}".format(exec_path))
    print("lib_dir:{}".format(lib_dir))
    print("---------------------")
    # execute exec_path file with subprocess
    env = os.environ.copy()
    env["LD_LIBRARY_PATH"] = lib_dir
    cmd = [exec_path]
    exit_code = subprocess.Popen(cmd, env=env).wait()
    print("---------------------")
    if exit_code < 0:
        print("[ERROR] Program exit with code:{}".format(exit_code))

# args = parser.parse_args()
if __name__ == '__main__':
    main()
