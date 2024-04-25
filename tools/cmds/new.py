#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
#

import argparse
import os
import sys
import shutil

parser = argparse.ArgumentParser(prog="new", description="create new project", add_help=False)

############################### Add option here #############################
parser.add_argument("--id", help="project id", default=None)
parser.add_argument("--name", help="project name", default=None)
parser.add_argument("--icon", help="project icon", default=None)
parser.add_argument("--author", help="project author", default=None)
parser.add_argument("--desc", help="project description", default=None)

#############################################################################

# use project_args created by SDK_PATH/tools/cmake/project.py, e.g. project_args.terminal

def create_project(sdk_path, id, name, icon = None, author = None, desc = None):
    print("-- create new project --")
    if not id:
        id = input("Please input project unique id, e.g. hello_world: ")
    if os.path.exists(id):
        print("-- [ERROR] Already have {} in this folder".format(id))
        sys.exit(1)
    if not name:
        name = input("Input project name, e.g. Hello World: ")
    if icon is None:
        icon = input("Input icon path, e.g. logo.png: (can be empty) ")
    if author is None:
        author = input("Input author name: (can be empty) ")
    if desc is None:
        desc = input("Input Project description: (can be empty) ")
    app_yaml = '''id: {}
name: {}
name[zh]:
version: 1.0.0
#icon: assets/hello.png
author: {}
desc: {}
desc[zh]:
files:
  # assets: assets
'''.format(id, name, author, desc)
    os.makedirs(id)
    with open(os.path.join(id, "app.yaml"), "w", encoding="utf-8") as f:
        f.write(app_yaml)
    readme = '''{} Project based on MaixCDK
====

{}

{}

This is a project based on MaixCDK, build method please visit [MaixCDK](https://github.com/sipeed/MaixCDK)

'''.format(name, "![logo]({})".format(icon) if icon else "", desc)
    with open(os.path.join(id, "README.md"), "w", encoding="utf-8") as f:
        f.write(readme)
    # copy sdk_path/tools/project_template/* to id/
    template_path = os.path.join(sdk_path, "tools", "project_template")
    for file in os.listdir(template_path):
        s = os.path.join(template_path, file)
        d = os.path.join(id, file)
        if os.path.isdir(s):
            shutil.copytree(s, d)
        else:
            shutil.copy2(s, d)

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
    args = vars["project_args"]
    create_project(vars["sdk_path"], args.id, args.name, args.icon, args.author, args.desc)

# args = parser.parse_args()
if __name__ == '__main__':
    main()
