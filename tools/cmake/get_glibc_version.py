import os, sys


if __name__ == "__main__":
    gcc_path = sys.argv[1]
    save_path = sys.argv[2]
    if not os.path.exists(gcc_path):
        print("gcc path not exists: {}".format(gcc_path))
        sys.exit(1)
    cmd = "strings {} | grep -o 'GLIBC_[0-9.].*'".format(gcc_path)
    ret = os.popen(cmd).read()
    if ret:
        version_strs = ret.strip().split("\n")
        max_version = [0]
        for v in version_strs:
            v = v.split("_")[1]
            v = v.split(".")
            v = [int(x) for x in v]
            for i, x in enumerate(v):
                if len(max_version) <= i:
                    max_version = v
                    break
                if x > max_version[i]:
                    max_version = v
                    break
                elif x < max_version[i]:
                    break

        version = ".".join([str(x) for x in max_version])
        with open(save_path, "w", encoding="utf-8") as f:
            f.write(version)
