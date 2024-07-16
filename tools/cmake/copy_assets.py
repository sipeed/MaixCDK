import os
import sys
import shutil
import yaml

def copy_assets(project_path, dist_path):
    app_yaml_path = os.path.join(project_path, 'app.yaml')

    # 检查 app.yaml 是否存在
    if not os.path.exists(app_yaml_path):
        print(f"{app_yaml_path} not a app, skip copy files")
        return

    # 读取 app.yaml 文件
    with open(app_yaml_path, 'r') as file:
        config = yaml.safe_load(file)

    # 检查 assets 键是否存在
    if 'files' not in config:
        print("No 'files' key found in app.yaml, skip copy files")
        return

    assets = config.get('files', [])

    if isinstance(assets, list):
        for item in assets:
            src = os.path.join(project_path, item)
            dest = os.path.join(dist_path, item)
            if os.path.exists(src):
                shutil.copytree(src, dest, dirs_exist_ok=True)
                print(f"Copied {src} to {dest}")
            else:
                raise Exception(f"{src} does not exist.")
    elif isinstance(assets, dict):
        for src_key, dest_value in assets.items():
            src = os.path.join(project_path, src_key)
            dest = os.path.join(dist_path, dest_value)
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            if os.path.exists(src):
                if os.path.isdir(src):
                    shutil.copytree(src, dest, dirs_exist_ok=True)
                else:
                    shutil.copyfile(src, dest)
                print(f"Copied {src} to {dest}")
            else:
                raise Exception(f"{src} does not exist.")
    else:
        print("'files' key must be a list or a dictionary.")

if __name__ == "__main__":
    project_path = sys.argv[1]
    dist_path = sys.argv[2]
    copy_assets(project_path, dist_path)
