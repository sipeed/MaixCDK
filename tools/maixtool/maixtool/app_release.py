import os
import shutil
import yaml
import zipfile
from datetime import datetime
import hashlib

def check_app_info(app_info, project_path):
    keys = ["id", "name", "version", "author", "desc"]
    for key in keys:
        if key not in app_info:
            raise ValueError("app.yaml must contain {} keyword".format(key))
    # check version format, should be major.minor.patch
    version = app_info["version"]
    version = version.split(".")
    try:
        version = [int(x) for x in version]
    except Exception:
        raise ValueError("app.yaml version must be major.minor.patch, e.g. 1.0.0")
    # check icon
    if (not "icon" in app_info) or (not app_info["icon"]):
        app_info["icon"] = ""


def get_app_info(project_path):
    app_info_path = os.path.join(project_path, "app.yaml")
    if not os.path.exists(app_info_path):
        app_info_path = os.path.join(project_path, "app.yml")
    if os.path.exists(app_info_path):
        # load app info from yaml file
        with open(app_info_path, "r") as f:
            app_info = yaml.load(f, Loader=yaml.FullLoader)
            check_app_info(app_info, project_path)
            return app_info
    return None

def copy_files(files, src_dir, dst_dir):
    for src, dst in files.items():
        if not os.path.exists(src):
            raise FileNotFoundError("file {} not found".format(src))
        if not os.path.isabs(src):
            src = os.path.join(src_dir, src)
        # check path, do not allow write to dangerous path
        dst_abs = os.path.abspath(os.path.join(dst_dir, dst))
        if not dst_abs.startswith(dst_dir):
            raise ValueError("file {} copy to {} is not allowed".format(src, dst_abs))
        os.makedirs(os.path.dirname(dst_abs), exist_ok=True)
        if os.path.isdir(src):
            print("-- copy dir: {} -> {}".format(src, dst))
            shutil.copytree(src, dst_abs)
        else:
            print("-- copy file: {} -> {}".format(src, dst))
            shutil.copyfile(src, dst_abs)

def get_files(project_path, exclude = [], black_list = []):
    '''
        get all files in project_path, except exclude items
    '''
    files = {}
    for root, dirs, fs in os.walk(project_path):
        for f in fs:
            path = os.path.relpath(os.path.join(root, f), project_path).replace("\\", "/")
            # ignore __pycache__ and .git
            if "__pycache__" in path or ".git" in path:
                continue
            valid = True
            for e in exclude:
                if e == path or path.startswith(e + "/"):
                    valid = False
                    break
            if not valid:
                continue
            files[path] = path
    return files

def check_icon_file(files, icon):
    if not icon:
        return
    found = False
    for src, dst in files.items():
        if os.path.isfile(dst):
            print(dst, icon)
            if dst == icon:
                found = True
                break
        else:
            # walk dir to find icon
            for root, dirs, fs in os.walk(dst):
                for f in fs:
                    if os.path.join(root, f) == icon:
                        found = True
                        break
                if found:
                    break
        if found:
            break
    if not found:
        raise FileNotFoundError("icon file {} not found in files".format(icon))

def pack(project_path, bin_path="main.py", extra_files = {}):
    '''
        Find main.py and app.yaml in current directory, pack to dist/app_id_vx.x.x.zip,
        with a app_id directory in it.
    '''
    old_path = os.getcwd()
    os.chdir(project_path)
    app_yaml = "app.yaml"
    app_info = get_app_info(project_path)
    if not app_info:
        raise FileNotFoundError("app.yaml not found in current directory")
    if not os.path.exists(bin_path):
        raise FileNotFoundError("{} not found".format(bin_path))
    app_id = app_info["id"]
    temp_dir = os.path.join(project_path, "dist", "pack", app_id)
    shutil.rmtree(temp_dir, ignore_errors=True)
    os.makedirs(temp_dir)
    # get files need to copy
    if "files" not in app_info or not app_info["files"]:
        # get all files in project_path except app_info["exclude"] items
        app_info["files"] = get_files(project_path, app_info.get("exclude", []))
        # append app_info["extra_include"] items
        app_info["files"].update(app_info.get("extra_include", {}))
    if type(app_info["files"]) == list:
        li = app_info["files"]
        app_info["files"] = {}
        for k in li:
            app_info["files"][k] = k

    # copy main.py
    if bin_path.endswith(".py"):
        bin_name = "main.py"
        # find all py files in project dir add to app_info["files"]
        for name in os.listdir(project_path):
            if name.endswith(".py"):
                app_info["files"][name] = name
    elif bin_path.endswith(".sh"):
        bin_name = "main.sh"
    else:
        bin_name = app_id
    app_info["files"][bin_path] = bin_name
    app_info["files"][app_yaml] = app_yaml
    app_info["files"].update(extra_files)
    check_icon_file(app_info["files"], app_info.get("icon", ""))
    copy_files(app_info["files"], project_path, temp_dir)
    # zip
    version_str = "_v" + app_info["version"]
    zip_path = os.path.join(project_path, "dist", app_id + version_str +".zip")
    os.chdir(os.path.dirname(temp_dir))
    if os.path.exists(zip_path):
        os.remove(zip_path)
    os.makedirs(os.path.dirname(zip_path), exist_ok=True)
    with zipfile.ZipFile(zip_path,'w', zipfile.ZIP_DEFLATED) as target:
        for i in os.walk(os.path.basename(temp_dir)):
            for n in i[2]:
                target.write(os.path.join(i[0],n))
    os.chdir(project_path)

    release_info_path = os.path.join(project_path, "dist", "release_info_v{}.yaml".format(app_info["version"]))
    release_info = {
        "app_info": app_info,
        "path": zip_path,
        "date": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "sha256sum": hashlib.sha256(open(zip_path, "rb").read()).hexdigest(),
        "filename": os.path.basename(zip_path),
        "filesize": os.path.getsize(zip_path)
    }

    print("-- write release info to dist/release_info.yaml")
    with open(release_info_path, "w") as f:
        yaml.dump(release_info, f, encoding="utf-8", allow_unicode=True)
    os.chdir(old_path)
    return zip_path

if __name__ == "__main__":
    import sys
    zip_file = pack(sys.argv[1], sys.argv[2])
    print("-- release complete, file:{}".format(zip_file))
