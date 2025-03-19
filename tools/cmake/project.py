#
# @file from https://github.com/Neutree/c_cpp_project_framework
# @author neucrack
# @license MIT
# @update 2023.12.20 simplify code by neucrack
#

import argparse
import os, sys, time, re, shutil
import site
import json
import subprocess
import yaml
from multiprocessing import cpu_count

try:
    curr_dir = os.path.abspath(os.path.dirname(__file__))
except Exception:
    print("-- [ERROR] Please upgrade maixtool by pip install -U maixtool")
    sys.exit(1)

sys.path.insert(0, curr_dir)
from build import build, rebuild, clean, distclean, menuconfig
from check_toolchain import main as check_toolchain_main

def get_saved_configs(build_path):
    vars = {}
    config_path = os.path.join(build_path, "config", "project_vars.json")
    if os.path.exists(config_path):
        with open(config_path, encoding="utf-8") as f:
            vars = json.load(f)
    return vars

def save_configs(build_path, vars):
    config_path = os.path.join(build_path, "config", "project_vars.json")
    os.makedirs(os.path.dirname(config_path), exist_ok=True)
    with open(config_path, "w", encoding="utf-8") as f:
        json.dump(vars, f, indent=4)

def update_configs(sdk_path, project_path, project_id, build_path, args, cmd = "menuconfig"):
    '''
        these vars different from saved in .config.mk's, them alway new in every command execute time.
        vars:
            {
                "SDK_PATH": sdk_path,
                ...
            }
    '''
    build_type = None
    cmake_build_type = None
    if hasattr(args, "build_type") and args.build_type:
        cmake_build_type = args.build_type
        build_type = "Debug" if cmake_build_type == "Debug" else "Release"
    elif hasattr(args, "release") and args.release:
        cmake_build_type = "MinSizeRel"
        build_type = "Release"
    elif hasattr(args, "debug") and args.debug:
        cmake_build_type = "Debug"
        build_type = "Debug"
    else:
        cmake_build_type = "MinSizeRel"
        build_type = "Release"
    py_site_pkg_path = site.getsitepackages()[0]
    if not os.path.exists(py_site_pkg_path):
        py_site_pkg_path = ""
    py_usr_site_pkg_path = site.getusersitepackages()
    if not os.path.exists(py_usr_site_pkg_path):
        py_usr_site_pkg_path = ""
    def set_default(vars):
        if cmd == "build":
            if not vars.get("CMAKE_BUILD_TYPE", ""):
                vars["CMAKE_BUILD_TYPE"] = "Debug"
            if not vars.get("BUILD_TYPE", ""):
                vars["BUILD_TYPE"] = "Debug"
            if not vars.get("CMAKE_GENERATOR", ""):
                vars["CMAKE_GENERATOR"] = "Unix Makefiles"
        else:
            if not vars.get("CMAKE_BUILD_TYPE", ""):
                vars["CMAKE_BUILD_TYPE"] = ""
            if not vars.get("BUILD_TYPE", ""):
                vars["BUILD_TYPE"] = ""
            if not vars.get("CMAKE_GENERATOR", ""):
                vars["CMAKE_GENERATOR"] = ""
        if not vars.get("TOOLCHAIN_ID", ""):
            vars["TOOLCHAIN_ID"] = ""
        if not vars.get("DEFAULT_CONFIG_FILE", ""):
            path = "{}/config_defaults.mk".format(project_path)
            if os.path.exists(path):
                vars["DEFAULT_CONFIG_FILE"] = path
            else:
                vars["DEFAULT_CONFIG_FILE"] = ""
        if not vars.get("PLATFORM", ""):
            vars["PLATFORM"] = "linux"
        return vars
    def _update_configs(old, new):
        for k,v in new.items():
            if v:
                old[k] = v
    def update_platform(vars, platform):
        rms = []
        for k,v in vars.items():
            if k.startswith("PLATFORM_"):
                rms.append(k)
        for k in rms:
            vars.pop(k)
        vars["PLATFORM_{}".format(platform.upper())] = 1
        return vars
    # load vars already set by args
    vars = get_saved_configs(build_path)
    vars.update({
        # get every time
        "PROJECT_ID": project_id,
        "SDK_PATH": sdk_path,
        "PROJECT_PATH": project_path,
        "MAIXCDK_EXTRA_COMPONENTS_PATH": os.environ.get("MAIXCDK_EXTRA_COMPONENTS_PATH", ""),
        "PY_PKG_COMPONENTS_PATH": py_site_pkg_path,
        "PY_USR_PKG_COMPONENTS_PATH": py_usr_site_pkg_path,
    })
    new_vars = {
        # if not set in args, get from build/config/project_vars.json
        "CMAKE_BUILD_TYPE": cmake_build_type,
        "BUILD_TYPE": build_type,
        "CMAKE_GENERATOR": args.generator if hasattr(args, "generator") else "",
        "DEFAULT_CONFIG_FILE": args.config_file if (hasattr(args, "config_file") and args.config_file and os.path.exists(args.config_file)) else "",
        "PLATFORM": args.platform if hasattr(args, "platform") else "",
        "TOOLCHAIN_ID": args.toolchain_id if hasattr(args, "toolchain_id") else "",
    }
    if new_vars["DEFAULT_CONFIG_FILE"] and not os.path.isabs(new_vars["DEFAULT_CONFIG_FILE"]):
        new_vars["DEFAULT_CONFIG_FILE"] = os.path.abspath(os.path.join(project_path, new_vars["DEFAULT_CONFIG_FILE"]))
    _update_configs(vars, new_vars)
    # if var not set by args, set default
    vars = set_default(vars)
    vars = update_platform(vars, vars["PLATFORM"])
    os.environ["BUILD_TYPE"] = vars["BUILD_TYPE"]
    return vars

def check_project_valid():
    if not os.path.exists("main"):
        print("")
        print("Error: can not find main component, please execute this command in your project root directory")
        print("")
        sys.exit(1)

def get_project_name(project_path):
    project_id = None
    if os.path.exists("app.yaml"):
        with open("app.yaml") as f:
            app_yaml = yaml.safe_load(f)
            project_id = app_yaml["id"]
    if not project_id:
        project_id = os.path.basename(project_path)
    return project_id

def get_extra_cmds(sdk_path) -> tuple:
    '''
        return extra_tools
        {
            "run":
            {
                "obj": tool_object,
                "cmd": cmd,
                "parser": arg_parser
            }
        }
    '''
    # find extra tools
    tools_dir = os.path.join(sdk_path, "tools", "cmds")
    sys.path.insert(1, tools_dir)
    # find all .py files in tools dir
    extra_tools_names = []
    extra_tools = {}
    for name in os.listdir(tools_dir):
        if name.endswith(".py"):
            extra_tools_names.append(name[:-3])
    # import all tools
    for name in extra_tools_names:
        # import from tool file
        tool = __import__(name)
        if hasattr(tool, "parser"):
            extra_tools[tool.__name__] = {
                "obj": tool,
                "cmd": tool.parser.prog,
                "parser": tool.parser
            }
    return extra_tools

def get_platforms(sdk_path) -> list:
    # find all devices from ../../platforms/*.yaml
    boards_dir = os.path.join(sdk_path, "platforms")
    platforms = []
    for name in sorted(os.listdir(boards_dir)):
        if name.endswith(".yaml"):
            platforms.append(name[:-5])
    return platforms

def parse_args(sdk_path, project_path, extra_tools):
    parser = argparse.ArgumentParser(description='build tool, e.g. `python project.py build`', prog="project.py")
    subparsers = parser.add_subparsers(help='command help', dest="cmd")
    for k,v in extra_tools.items():
        sub_parser = v["parser"]
        subparsers.add_parser(sub_parser.prog, parents=[sub_parser], help=sub_parser.description)

    # cmd build
    parser_build = subparsers.add_parser("build", help="start compile project, temp files in `build` dir, dist files in `dist` dir, build command by default always execute cmake to regenerate, to disable file change scan (cmake execute), use build2 command or add --no-gen option to only compile changed files, warning build2 will not detect file additions and deletions")
    parser_build2 = subparsers.add_parser("build2", help="same as `maixcdk build --no-gen`, compile project, not scan files additions and deletions, only compile changed files, so build faster but be attention you should use build again if you add new file or delete files")
    parser_build.add_argument("--no-gen", action="store_true", default=False, help="same as command build2, by default the build command do the same action as rebuild to avoid user can't understand rebuild, but if you don't want to re-execute cmake command, and only compile, use this option")
    def add_build_args(parser_):
        parser_.add_argument('--config-file',
                                help='config file path, e.g. config_defaults.mk',
                                metavar='PATH',
                                default="{}/config_defaults.mk".format(project_path))
        parser_.add_argument('--verbose',
                                help='for build command, execute `cmake -build . --verbose` to compile',
                                action="store_true",
                                default=False)
        parser_.add_argument('-G', '--generator', default="", help="project type to generate, supported type on your platform see `cmake --help`")
        parser_.add_argument('--release', action="store_true", default=False, help="release mode, default is release mode")
        parser_.add_argument('--debug', action="store_true", default=False, help="debug mode, default is release mode")
        parser_.add_argument('--build-type', default=None, help="build type, [Debug, Release, MinRelSize, RelWithDebInfo], you can also set build type by CMAKE_BUILD_TYPE environment variable")
        parser_.add_argument('-p', "--platform", default="", help="device name, e.g. linux, maixcam, m2dock", choices=get_platforms(sdk_path))
        parser_.add_argument('--toolchain-id', default="", help="toolchain id, if platform has multiple toolchains, use this option to select one")
    add_build_args(parser_build)
    add_build_args(parser_build2)

    # cmd menuconfig
    parser_menuconfig = subparsers.add_parser("menuconfig", help="open menuconfig panel, a visual config panel")
    parser_menuconfig.add_argument('--config-file',
                            help='config file path, e.g. config_defaults.mk',
                            metavar='PATH',
                            default="{}/config_defaults.mk".format(project_path))
    parser_menuconfig.add_argument('-p', "--platform", default="", help="device name, e.g. linux, maixcam, m2dock", choices=get_platforms(sdk_path))

    # cmd clean
    subparsers.add_parser("clean", help="clean build files, won't clean configuration")

    # cmd distclean
    subparsers.add_parser("distclean", help="clean all generated files, including build files and configuration")

    args = parser.parse_args()
    return args, parser

def check_project_cmakelists(project_path):
    project_cmake = '''
# !!! This file is auto generated by MaixCDK, don't modify it manually !!!

cmake_minimum_required(VERSION 3.13)

# cmake used vars defined in MaixCDK/tools/cmake/project.py's get_configs function

include(${SDK_PATH}/tools/cmake/compile.cmake)
project(${PROJECT_ID})

'''
    cmakelists_path = os.path.join(project_path, "CMakeLists.txt")
    if not os.path.exists(cmakelists_path):
        with open(cmakelists_path, "w") as f:
            f.write(project_cmake)

def choose_platform(sdk_path):
    plats = get_platforms(sdk_path)
    default_idx = plats.index("linux")
    print("\nPlatform not set, use `-p xxx` arg to assign, or `maixcdk build -h` to see help")
    print("Select platform: ")
    for i, plat in enumerate(plats):
        print("\33[32m{}. {}\33[33m{}\33[0m".format(i+1, plat, " (default)" if i == default_idx else ""))
    print("Input number to select, Ctrl+C to cancel: ", end="")
    try:
        select_item = input()
        if select_item == "":
            idx = default_idx + 1
        else:
            idx = int(select_item)
            if idx <= 0 or idx > len(plats):
                print("Value error, canceled!")
                sys.exit(1)
    except Exception:
        print("Value error, canceled!")
        sys.exit(1)
    new_plat = plats[idx-1]
    return new_plat, plats

def main(sdk_path, project_path):
    sdk_path = os.path.abspath(sdk_path)
    project_path = os.path.abspath(project_path)

    # 3. get extra cmds
    extra_tools = get_extra_cmds(sdk_path)
    # 4. parse args
    args, parser = parse_args(sdk_path, project_path, extra_tools)
    cmd = args.cmd

    # 2. get basic vars
    project_id = get_project_name(project_path)
    build_path = os.path.join(project_path, "build")
    dist_path = os.path.join(project_path, "dist")
    thread_num = cpu_count()

    if cmd not in ["clean", "distclean"]:
        configs_old = get_saved_configs(build_path)
        is_first_compile = not os.path.exists(os.path.join(build_path, "config", "project_vars.json"))
        configs = update_configs(sdk_path, project_path, project_id, build_path, args, cmd)
        platform_old = configs_old.get("PLATFORM", "")
        build_type_old = configs_old.get("CMAKE_BUILD_TYPE", "")
        if platform_old and platform_old != configs["PLATFORM"]:
            print("\nPlatform changed from {} to {}".format(platform_old, configs["PLATFORM"]))
            print("Please execute `maixcdk distclean` to clean old build files first, maybe you need to execute `maixcdk menuconfig` after distclean")
            sys.exit(1)
        elif build_type_old and build_type_old != configs["CMAKE_BUILD_TYPE"]:
            print("\nBuild type changed from {} to {}".format(build_type_old, configs["CMAKE_BUILD_TYPE"]))
            print("Please execute `maixcdk distclean` to clean old build files first, maybe you need to execute `maixcdk menuconfig` after distclean")
            sys.exit(1)
        if is_first_compile and ((hasattr(args, "platform") and not args.platform) or not hasattr(args, "platform")):
            new_plat, plats = choose_platform(sdk_path)
            args.platform = new_plat
            configs = update_configs(sdk_path, project_path, project_id, build_path, args, cmd)
        save_configs(build_path, configs)

    # dispatch cmd
    if cmd == "clean":
        print("-- Clean now")
        clean(build_path)
        print("-- Clean done")
        sys.exit(0)
    elif cmd == "distclean":
        print("-- Distclean now")
        distclean(project_path, build_path, dist_path)
        print("-- Distclean done")
        sys.exit(0)
    elif cmd in ["build", "build2"]:
        check_project_valid()
        platform = configs["PLATFORM"]
        print("-- Project ID: {}".format(project_id))
        print("-- Project path: {}".format(project_path))
        print("-- Platform: {}".format(platform))
        print("-- Build path: {}".format(build_path))
        print("-- CPU count: {}".format(thread_num))
        print("-- Build type: {} ({})".format(configs["BUILD_TYPE"], configs["CMAKE_BUILD_TYPE"]))
        print("-- Build generator: {}".format(configs["CMAKE_GENERATOR"]))
        print("-- MAIXCDK_EXTRA_COMPONENTS_PATH: {}".format(configs["MAIXCDK_EXTRA_COMPONENTS_PATH"]))
        print("-- PY_PKG_COMPONENTS_PATH: {}".format(configs["PY_PKG_COMPONENTS_PATH"]))
        print("-- PY_USR_PKG_COMPONENTS_PATH: {}".format(configs["PY_USR_PKG_COMPONENTS_PATH"]))
        print("")

        # 6. check project CMakeLists.txt
        check_project_cmakelists(project_path)

        # check toolchain
        info = check_toolchain_main(platform, os.path.join(sdk_path, "platforms"), os.path.join(build_path, "config", "toolchain_config.cmake"), configs["TOOLCHAIN_ID"])
        configs["TOOLCHAIN_ID"] = info.get("id", "")
        save_configs(build_path, configs)
        configs["CMAKE_C_FLAGS"] = info.get("c_flags", "")
        configs["CMAKE_CXX_FLAGS"] = info.get("cxx_flags", "")
        configs["CMAKE_EXE_LINKER_FLAGS"] = info.get("ld_flags", "")
        configs["CMAKE_C_LINK_FLAGS"] = info.get("c_link_flags", "")
        configs["CMAKE_CXX_LINK_FLAGS"] = info.get("cxx_link_flags", "")
        if "cpp_flags" in info or "cpp_link_flags" in info:
            raise Exception("Please use cxx_flags or cxx_link_flags instead of cpp_flags or cpp_link_flags in platform yaml file")

        # 7. check staging dir for some toolchains
        if not os.environ.get("STAGING_DIR"):
            os.environ["STAGING_DIR"] = build_path+"/staging_dir"
            print("-- STAGING_DIR: {}".format(os.environ["STAGING_DIR"]))

        print("-- Build now")
        t = time.time()
        if cmd == "build2" or args.no_gen:
            build(build_path, configs, info, args.verbose)
        else:
            rebuild(build_path, configs, info, args.verbose)
        t = time.time() - t
        print("\n==================================")
        print("build time: {:.2f}s ({})".format(t, time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())))
        print("platform  : {}".format(platform))
        print("build type: {}({})".format(configs["BUILD_TYPE"], configs["CMAKE_BUILD_TYPE"]))
        if configs["TOOLCHAIN_ID"]:
            print("toolchain : {}".format(configs["TOOLCHAIN_ID"]))
        print("==================================\n")
    elif cmd == "menuconfig":
        check_project_valid()
        print("-- Menuconfig now")
        menuconfig(sdk_path, build_path, configs)
        print("-- Menuconfig done")
    # extra tools
    else:
        for k, v in extra_tools.items():
            if cmd == v["cmd"]:
                tool = v["obj"]
                vars = {
                    "project_path": project_path,
                    "project_id": project_id,
                    "sdk_path": sdk_path,
                    "build_type": configs["CMAKE_BUILD_TYPE"],
                    "project_parser": parser,
                    "project_args": args,
                    "configs": configs,
                }
                print("\n-------- {} start ---------".format(args.cmd))
                ret = tool.main(vars)
                print("-------- {} end ---------\n".format(args.cmd))
                exit(ret)
        print("Error: Unknown command")
        exit(1)


