import sys, json
'''
{
    "main": ["maix"],
    "python": []
}

'''

def get_depends(component, data):
    depends = data[component] if component in data else []
    sub_depends = []
    for d in depends:
        if d == component:
            continue
        sub_depends += get_depends(d, data)
    return depends + sub_depends


if __name__ == "__main__":
    cmd = sys.argv[1]
    out_file = sys.argv[2]

    if cmd == "clear":
        with open(out_file, "w") as f:
            f.write("{}")
    elif cmd == "append":
        component_name = sys.argv[3]
        depends = sys.argv[4:]
        with open(out_file, "r", encoding="utf-8") as f:
            data = json.load(f)
        data[component_name] = depends
        with open(out_file, "w", encoding="utf-8") as f:
            f.write(json.dumps(data, indent=4))
    elif cmd == "get":
        component_name = sys.argv[3]
        with open(out_file, "r", encoding="utf-8") as f:
            data = json.load(f)
        if component_name not in data:
            print("{} not found in {}".format(component_name, out_file))
            sys.exit(1)
        depends = get_depends(component_name, data)
        print(";".join(depends), end="")
    else:
        raise Exception("components_depends.py unknown cmd: {}".format(cmd))
