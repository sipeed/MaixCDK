#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
#

import argparse
import os
import subprocess
import pip

parser = argparse.ArgumentParser(prog="install_env", description="install python packages for project", add_help=False)

############################### Add option here #############################
parser.add_argument("-i", "--index-url", help="Python package index-url, default is https://pypi.org/simple, in China can use https://pypi.tuna.tsinghua.edu.cn/simple", default="")

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
    # install python packages according to sdk_path/requirements.txt
    if vars["project_args"].index_url:
        pip.main(["install", "-i", vars["project_args"].index_url, "-r", os.path.join(vars["sdk_path"], "requirements.txt")])
    else:
        pip.main(["install", "-r", os.path.join(vars["sdk_path"], "requirements.txt")])

# args = parser.parse_args()
if __name__ == '__main__':
    main()
