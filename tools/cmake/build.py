import subprocess

import os
from multiprocessing import cpu_count
import shutil
import sys
import re
import traceback
import json

try:
    curr_dir = os.path.abspath(os.path.dirname(__file__))
except Exception:
    print("-- [ERROR] Please upgrade maixtool by pip install -U maixtool")
    sys.exit(1)

if curr_dir not in sys.path:
    sys.path.insert(0, curr_dir)

from file_downloader import download_extract_files

thread_num = cpu_count()

def execute_component_py_func(py_path : str, func : str, *args, **kwargs):
    g = {}
    with open(py_path, "r", encoding="utf-8") as f:
        exec(f.read(), g)
        if func not in g:
            return False, []
    return True, g[func](*args, **kwargs)

def parse_kconfigs(config_path, toolchain_config_path):
    configs = {}
    with open(config_path, "r") as f:
        lines = f.readlines()
    for line in lines:
        line = line.strip()
        if line.startswith("#") or line == "":
            continue
        k,v = line.split("=", 1)
        if v == "y":
            v = True
        configs[k] = v
    with open(toolchain_config_path, "r") as f:
        lines = f.readlines()
    for line in lines:
        line = line.strip()
        if line.startswith("#") or line == "":
            continue
        if line.startswith("set("):
            k, v = line.split("(", 1)[1].rsplit(")", 1)[0].split(" ", 1)
            configs[k] = v
    return configs

def get_components_dirs(find_dirs):
    components = {}
    for dir in find_dirs:
        if (not dir) or not os.path.exists(dir):
            continue
        for name in os.listdir(dir):
            path = os.path.join(dir, name, "CMakeLists.txt")
            if os.path.exists(path):
                components[name] = os.path.dirname(path)
    return components

def get_components_files(components, valid_components, kconfigs):
    files = {}
    for name in valid_components:
        py_path = os.path.join(components[name], "component.py")
        if not os.path.exists(py_path):
            continue
        print(f"-- get files for component {name} (component.py)")
        found, files[name] = execute_component_py_func(py_path, "add_file_downloads", kconfigs)
        if not found:
            files[name] = []
    return files

def get_component_requirements(component_name, component_dir, find_dirs):
    requires = []
    component_py = os.path.join(component_dir, "component.py")
    if os.path.exists(component_py):
        found, requires = execute_component_py_func(component_py, "add_requirements", find_dirs)
        if not found:
            requires = []
    return requires

def find_valid_components(components, find_dirs):
    '''
        get project depends(not accurately, just exclude some obvious not depend components)
        return valid components name
    '''
    # get components quire
    depends = {}
    for name, dir in components.items():
        component_py = os.path.join(dir, "component.py")
        found = False
        if os.path.exists(component_py):
            found, depends_component = execute_component_py_func(component_py, "add_requirements", find_dirs)
        if not found:
            cmakelist = os.path.join(dir, "CMakeLists.txt")
            with open(cmakelist, "r", encoding="utf-8") as f:
                content = f.read()
                match = re.findall(r'list\(APPEND ADD_REQUIREMENTS(.*?)\)', content, re.DOTALL|re.MULTILINE)
                depends_component = list(set(" ".join(match).split()))
        depends[name] = []
        for r in depends_component:
            if r in components:
                if name == r:
                    continue
                depends[name].append(r)
    # find main depends
    def get_depend_recursive(name, seen={}):
        if name in seen:
            return seen[name]
        d = depends[name].copy()
        for r in depends[name]:
            d.extend(get_depend_recursive(r, seen))
        d = list(set(d))
        seen[name] = d
        return d
    valid = ["main"]
    valid.extend(get_depend_recursive("main"))
    return valid

def get_components_find_dirs(configs):
    final = []
    find_dirs = [ # same as compile.cmake find_components in project macro
        configs.get("MAIXCDK_EXTRA_COMPONENTS_PATH", ""),
        configs.get("PY_PKG_COMPONENTS_PATH", ""),
        configs.get("PY_USR_PKG_COMPONENTS_PATH", ""),
        os.path.join(configs["SDK_PATH"], "components"),
        os.path.join(configs["SDK_PATH"], "components", "3rd_party"),
        os.path.join(configs["SDK_PATH"], "components", "ext_devs"),
        configs["PROJECT_PATH"],
        os.path.join(configs["PROJECT_PATH"], "components"),
        os.path.join(configs["PROJECT_PATH"], "..", "components"),
    ]
    for dir in find_dirs:
        if (not dir) or not os.path.exists(dir):
            continue
        final.append(dir)
    return final

def get_all_components_dl_info(find_dirs, kconfigs):
    components = get_components_dirs(find_dirs)
    valid_components = find_valid_components(components, find_dirs)
    files_info = get_components_files(components, valid_components, kconfigs)
    return files_info

def save_pkgs_info(sdk_path, files_info):
    info_path = os.path.join(sdk_path, "dl", "pkgs_info.json")
    count = 0
    for name, files in files_info.items():
        if files is not None and len(files) > 0:
            count += len(files)
            for i, item in enumerate(files):
                item["pkg_path"] = os.path.join(sdk_path, "dl", "pkgs", item["path"], item["filename"])
    print("\n-------------------------------------------------------------------")
    print("-- All {} files info need to be downloaded saved to\n   {}".format(count, info_path))
    print("-------------------------------------------------------------------\n")
    os.makedirs(os.path.dirname(info_path), exist_ok=True)
    with open(info_path, "w") as f:
        json.dump(files_info, f, indent=4)

def rebuild(build_path, configs, toolchain_info, verbose):
    os.makedirs(build_path, exist_ok=True)
    os.chdir(build_path)
    config_path = os.path.join(build_path, "config", "global_config.mk")
    toolchain_config_path = os.path.join(build_path, "config", "toolchain_config.cmake")
    if not os.path.exists(config_path):
        menuconfig(configs["SDK_PATH"], build_path, configs, False)

    # find and download all files, save to sdk_path/dl/pkgs_info.json
    all_configs = configs.copy()
    kconfigs = parse_kconfigs(config_path, toolchain_config_path)
    all_configs.update(kconfigs)
    find_dirs = get_components_find_dirs(configs)
    files_info = get_all_components_dl_info(find_dirs, all_configs)
    # add toolchain dl info
    dl_info = []
    if toolchain_info["url"]:
        dl_info = [
            {
                "url": toolchain_info['url'],
                "sha256sum": toolchain_info['sha256sum'],
                "filename": toolchain_info['filename'],
                "path": toolchain_info['path'],
            }
        ]
        download_extract_files(dl_info)
    files_info["toolchain"] = dl_info

    save_pkgs_info(configs["SDK_PATH"], files_info)
    for name, files in files_info.items():
        if files is not None and len(files) > 0:
            print(f"\n-- Download files for component {name}")
            try:
                download_extract_files(files)
            except Exception as e:
                print("\n\n---- ERROR -----")
                traceback.print_exc()
                print("----------------")
                print(f"[ERROR] download for component [{name}] failed, please check its component.py")
                sys.exit(1)

    # cmake
    cmd = ["cmake", "-G", configs["CMAKE_GENERATOR"]]
    for k, v in configs.items():
        cmd.append("-D{}={}".format(k, v))
    cmd.append("..")
    print("\n-- CMake CMD: ", " ".join(cmd))
    print("")
    res = subprocess.call(cmd)
    if res != 0:
        exit(1)
    # write global_config_platform.h
    path = os.path.join(build_path, "config", "global_config_platform.h")
    with open(path, "w", encoding="utf-8") as f:
        code = "#ifndef __GLOBAL_CONFIG_PLATFORM_H__\n#define __GLOBAL_CONFIG_PLATFORM_H__\n"
        code += "#define PLATFORM_{} 1\n".format(configs["PLATFORM"].upper())
        code += "#define PLATFORM \"{}\"\n".format(configs["PLATFORM"])
        code += "#define PROJECT_ID \"{}\"\n".format(configs["PROJECT_ID"])
        code += "#endif\n"
        f.write(code)

    # build
    build(build_path, configs, toolchain_info, verbose)

def build(build_path, configs, toolchain_info, verbose):
    if not os.path.exists(build_path):
        rebuild(build_path, configs, toolchain_info, verbose)
        return
    os.chdir(build_path)
    if verbose:
        if configs["CMAKE_GENERATOR"] == "Unix Makefiles":
            res = subprocess.call(["cmake", "--build", ".", "--target", "all", "--", "VERBOSE=1"])
        elif configs["CMAKE_GENERATOR"] == "Ninja":
            res = subprocess.call(["cmake", "--build", ".", "--target", "all", "--", "-v"])
        else:
            res = subprocess.call(["cmake", "--build", ".", "--target", "all"])
    else:
        if configs["CMAKE_GENERATOR"] in ["Unix Makefiles", "Ninja"]:
            res = subprocess.call(["cmake", "--build", ".", "--target", "all", "--", "-j{}".format(thread_num)])
        else:
            res = subprocess.call(["cmake", "--build", ".", "--target", "all"])
    if res != 0:
        exit(1)


def clean(build_path):
    if os.path.exists(build_path):
        os.chdir(build_path)
        p =subprocess.Popen(["cmake", "--build", ".", "--target", "clean"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, err = p.communicate("")
        res = p.returncode
        if res == 0:
            print(output.decode(encoding="gbk" if os.name == "nt" else "utf-8"))

def distclean(project_path, build_path, dist_path):
    if os.path.exists(build_path):
        os.chdir(build_path)
        p =subprocess.Popen(["cmake", "--build", ".", "--target", "clean"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, err = p.communicate("")
        res = p.returncode
        if res == 0:
            print(output.decode(encoding="gbk" if os.name == "nt" else "utf-8"))
        os.chdir("..")
        shutil.rmtree("build")
        project_cmakelist = os.path.join(project_path, "CMakeLists.txt")
        if os.path.exists(project_cmakelist):
            os.remove(project_cmakelist)
    if os.path.exists(dist_path):
        shutil.rmtree(dist_path)

def menuconfig(sdk_path, build_path, configs, gui = True):
    config_out_path = os.path.join(build_path, "config")
    os.makedirs(config_out_path, exist_ok=True)
    os.chdir(build_path)
    tool_path = os.path.join(sdk_path, "tools/kconfig/genconfig.py")
    if not os.path.exists(tool_path):
        print("-- [ERROR] kconfig tool not found:", tool_path)
        exit(1)
    cmd = [sys.executable, tool_path, "--kconfig", os.path.join(sdk_path, "Kconfig"), "--menuconfig", "True" if gui else "False"]
    if configs["DEFAULT_CONFIG_FILE"] and os.path.exists(configs["DEFAULT_CONFIG_FILE"]):
        cmd += ["--defaults", configs["DEFAULT_CONFIG_FILE"]]
    for k, v in configs.items():
        cmd += ["--env", "{}={}".format(k, v)]
    cmd += ["--output", "makefile", os.path.join(config_out_path, "global_config.mk")]
    cmd += ["--output", "cmake", os.path.join(config_out_path, "global_config.cmake")]
    cmd += ["--output", "header", os.path.join(config_out_path, "global_config.h")]
    res = subprocess.call(cmd)
    if res != 0:
        exit(1)

    # if board changed(find PLATFORM_.*?=y match not the same), need to clean build dir except config dir
    if os.path.exists(os.path.join(config_out_path, "global_config.mk.old")):
        with open(os.path.join(config_out_path, "global_config.mk"), "r") as f:
            config_mk = f.read()
        with open(os.path.join(config_out_path, "global_config.mk.old"), "r") as f:
            config_mk_old = f.read()
        m = re.findall(r'PLATFORM_(.*?)=y', config_mk)
        m_old = re.findall(r'PLATFORM_(.*?)=y', config_mk_old)
        if m and m_old and m[0] != m_old[0]:
            print("-- board changed, auto clean build dir")
            fs = os.listdir(".")
            for name in fs:
                if name == "config":
                    continue
                if os.path.isdir(name):
                    shutil.rmtree(name)
                else:
                    os.remove(name)
            print("-- clean build dir complete")

if __name__ == "__main__":
    cmd = sys.argv[1]
    if cmd == "get_valid_components":
        find_dirs = sys.argv[2:]
        for dir in find_dirs:
            if " " in dir:
                raise Exception("Path can not contain space or special characters")
        components = get_components_dirs(find_dirs)
        valid_components = find_valid_components(components, find_dirs)
        print(";".join(valid_components), end="")
    elif cmd == "get_requirements":
        find_dirs = sys.argv[4:]
        for dir in find_dirs:
            if " " in dir:
                raise Exception("Path can not contain space or special characters")
        requires = get_component_requirements(sys.argv[2], sys.argv[3], find_dirs)
        print(";".join(requires), end="")
