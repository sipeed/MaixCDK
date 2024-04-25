
import subprocess
import os
from multiprocessing import cpu_count
import shutil
import sys
import re

thread_num = cpu_count()

def rebuild(build_path, configs, verbose):
    os.makedirs(build_path, exist_ok=True)
    os.chdir(build_path)
    if not os.path.exists(os.path.join(build_path, "config", "global_config.mk")):
        menuconfig(configs["SDK_PATH"], build_path, configs, False)
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
    build(build_path, configs, verbose)

def build(build_path, configs, verbose):
    if not os.path.exists(build_path):
        rebuild(build_path, configs, verbose)
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

