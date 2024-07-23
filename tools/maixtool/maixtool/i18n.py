import sys, os
import yaml
from collections import OrderedDict


def get_files(scan_dir, recursive, exts=[".c", ".cpp", ".h", ".hpp"]):
    files = []
    if recursive:
        for root, dirs, fs in os.walk(scan_dir):
            for f in fs:
                if os.path.splitext(f)[-1] in exts:
                    files.append(os.path.join(root, f))
    else:
        for name in os.listdir(scan_dir):
            if os.path.splitext(name)[1] in exts:
                files.append(os.path.join(scan_dir, name))
    return files


def get_i18n_strs(file, keywords):
    '''
        read content and find '_("xxx")', get all xxx and return
    '''
    strs = []
    with open(file, "r") as f:
        content = f.read()
        for key in keywords:
            idx = 0
            while True:
                flag = False
                idx = content.find(f'{key}("', idx)
                if idx == -1:
                    idx = content.find(f"{key}('", idx)
                    if idx == -1:
                        break
                    flag = True
                if not (idx == 0 or content[idx - 1] in [".", ",", " ", ">", "(", "[", "{"]): # maybe like str etc.
                    idx += 1
                    continue
                idx += len(key) + 2
                if flag:
                    end = content.find("')", idx)
                else:
                    end = content.find('")', idx)
                if end == -1:
                    break
                strs.append(content[idx:end])
                idx = end
    return strs

def main(search_dir, keywords, exts, out, recursive, locales):
    print("search dir:", dir)
    print("keywords:  ", keywords)
    print("extensions:", exts)
    print("out dir:   ", out)
    print("")
    files = get_files(search_dir, recursive, exts)
    print(f"{len(files)} files found")
    keys_list = []
    for f in files:
        strs = get_i18n_strs(f, keywords)
        keys_list.extend(strs)
    keys_list = list(set(keys_list))
    keys_list.sort()
    keys_dict = {}
    for i in keys_list:
        keys_dict[i] = i
    os.makedirs(out, exist_ok=True)
    for locale in locales:
        print(f"gen locale [ {locale} ]")
        path = os.path.join(out, f"{locale}.yaml")
        old = {}
        if os.path.exists(path):
            with open(path, "r", encoding="utf-8") as f:
                old = yaml.safe_load(f)
        if old is None:
            old = {}
        # update by old
        for k in keys_dict:
            if k in old:
                keys_dict[k] = old[k]
        with open(path, "w", encoding="utf-8") as f:
            yaml.dump(keys_dict, f, allow_unicode=True)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-k", "--keywords", nargs="+", type=str, default=["_", "tr"], help="translate function keywords to search")
    parser.add_argument("-r", action="store_true", help="recursive search dir")
    parser.add_argument("-e", "--exts", nargs="+", type=str, default=[".c", ".cpp", ".h", ".hpp", ".py"], help="file extension to search")
    parser.add_argument("-l", "--locales", nargs="+", type=str, default=["en", "zh"], help="locals of region, like en zh ja etc. the locale name can be found in [here](https://www.science.co.il/language/Locale-codes.php) or [wikipedia](https://en.wikipedia.org/wiki/Language_localisation), all letters use lower case.")
    parser.add_argument("-o", "--out", type=str, default="locales", help="translation files output directory")
    parser.add_argument("-d", "--dir", type=str, help="where to search", required=True)
    args = parser.parse_args()
    main(args.dir, args.keywords, args.exts, args.out, args.r, args.locales)





