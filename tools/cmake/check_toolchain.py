import sys, re, os
import yaml
import json


# current dir path
cur_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(cur_dir)
sys.path.append(os.path.join(cur_dir, ".."))
dl_dir = os.path.abspath(os.path.join(cur_dir, "..", "..", "dl"))

from file_downloader import Downloader, check_sha256sum, unzip_file, download_extract_files

def check_toolchain_info(info, board_name):
    keys = ["url", "sha256sum", "path", "bin_path", "prefix"]
    for key in keys:
        if key not in info:
            print("Error: %s not found in toolchain info of board %s.yaml" % (key, board_name))
            return

def select_multi_toolchain(board_name, toolchain_info, select_id):
    if type(toolchain_info) is not list:
        return toolchain_info
    if len(toolchain_info) == 1:
        return toolchain_info[0]
    for info in toolchain_info:
        if not "id" in info:
            print("Error: id not found in toolchain info of board %s.yaml" % board_name)
            sys.exit(1)
        if select_id and info['id'] == select_id:
            return info
    print("This platform has multiple toolchains, please select one\n")
    default_idx = None
    for i, info in enumerate(toolchain_info):
        is_default = info.get("default", False)
        idx_str = str(i + 1)
        print("\33[32m{}: {}\33[33m{}\33[0m\n{}{}\n".format(idx_str, info['id'], " (default)" if is_default else "", " " * (len(idx_str) + 2), info['name']))
        if is_default:
            default_idx = i + 1
    while True:
        select_idx = input("\nInput number to select toolchain, or Ctrl+C to cancel: ").strip()
        if default_idx is not None and select_idx == "":
            select_idx = default_idx
            break
        if not select_idx.isdigit():
            print("Error: please input number")
            continue
        select_idx = int(select_idx)
        if select_idx < 1 or select_idx > len(toolchain_info):
            print("Error: please input number in range [1, {}]".format(len(toolchain_info)))
            continue
        break
    return toolchain_info[select_idx-1]

def save_pkgs_info(sdk_path, files_info):
    info_path = os.path.join(sdk_path, "dl", "pkgs_info.json")
    count = 0
    for name, files in files_info.items():
        if len(files) > 0:
            count += len(files)
            for i, item in enumerate(files):
                item["pkg_path"] = os.path.join(sdk_path, "dl", "pkgs", item["path"], item["filename"])
    print("\n-------------------------------------------------------------------")
    print("-- All {} files info need to be downloaded saved to\n   {}".format(count, info_path))
    print("-------------------------------------------------------------------\n")
    os.makedirs(os.path.dirname(info_path), exist_ok=True)
    with open(info_path, "w") as f:
        json.dump(files_info, f, indent=4)

def main(board_name, boards_dir, out_cmake, toolchain_id = None):

    # parse mk file, find PLATFORM_.*?=y to get board name
    if not board_name:
        print("Error: Platform name not set")
        sys.exit(1)

    # parse boards_dir/{board_name}.yaml to get toolchain info
    toolchain_info = None
    board_yaml = os.path.join(boards_dir, board_name + '.yaml')
    with open(board_yaml, 'r') as f:
        board_info = yaml.safe_load(f)
        toolchain_info = board_info['toolchain']
    toolchain_info = select_multi_toolchain(board_name, toolchain_info, toolchain_id)

    if not toolchain_info:
        print("Error: toolchain info not found in %s, please check yaml file of board" % board_yaml)
        sys.exit(1)

    check_toolchain_info(toolchain_info, board_name)

    toolchain_bin_path = os.path.join(dl_dir, "extracted", toolchain_info['bin_path']) if toolchain_info['bin_path'] else None

    # generate cmake file, set CONFIG_TOOLCHAIN_PATH and CONFIG_TOOLCHAIN_PREFIX
    with open(out_cmake, 'w') as f:
        f.write('set(CONFIG_TOOLCHAIN_PATH "{}")\n'.format(toolchain_bin_path if toolchain_bin_path else ""))
        f.write('set(CONFIG_TOOLCHAIN_PREFIX "{}")\n'.format(toolchain_info['prefix'] if toolchain_info['prefix'] else ""))

    print("-- Toolchain for platform %s is ready" % board_name)
    return toolchain_info
