import os
import re
import argparse
import json
import yaml
import time
try:
    from .gen_markdown import module_to_md
except Exception:
    from gen_markdown import module_to_md

def get_headers_recursive(dir):
    headers = []
    for root, dirs, files in os.walk(dir):
        for file in files:
            if file.endswith(".h") or file.endswith(".hpp"):
                headers.append(os.path.join(root, file))
    return headers

def get_code_def(code):
    '''
        find type and definition end of code
        @param code starts with definition, no blank line or char, e.g.
                    'void hello();' not '   void hello();'
    '''
    start_idx = 0
    while code[start_idx] in [" ", "\t", "\n"]:
        start_idx += 1
    code = code[start_idx:]
    def_type = None
    definition = None
    end_idx = 0
    if code.startswith("class") or code.startswith("struct"):
        def_type = "class"
        # definition: class ABC
        # find first { as end of definition
        idx = code.find("{")
        definition = code[:idx]
        end_idx = idx + start_idx
    elif code.startswith("enum"):
        def_type = "enum"
        # definition: enum ABC { ... }
        # find line by line, if line starts with } and not in block comment
        have_block_comment = False
        idx = 0
        for line in code.split("\n"):
            if not have_block_comment:
                if line.strip().startswith("}"):
                    idx += line.find("}") + 1
                    break
                if "/*" in line and "*/" not in line:
                    have_block_comment = True
                    idx += len(line) + 1
                    continue
            else:
                if "*/" in line:
                    have_block_comment = False
            idx += len(line) + 1
        definition = code[:idx]
        end_idx = idx + start_idx
    elif code.startswith("namespace"):
        def_type = "module"
        end_idx = code.find("{") + start_idx
        definition = code[:end_idx]
        # if "::" in definition:
        #     raise Exception("snamespcae definition {} format for API not support yet, please see api example header".format(definition))
    else:
        # find ;(var) and )(function) and = (var)
        # var definition:
        #   std::string hello = "hello"
        # func definition:
        #   std::map<std::string, int> get_dict(std::map<std::string, int> in, int i, const char *j = "10")
        #   Bytes &operator=(const Bytes &other)
        idx = code.find(";")
        if code.find("operator") >= 0:
            idx3 = -1
        else:
            idx3 = code.find("=")
        start_idx = code.find("(")
        sub_count = 1
        idx2 = -1
        for i in range(start_idx + 1, len(code)):
            if code[i] == "(":
                sub_count += 1
            elif code[i] == ")":
                sub_count -= 1
            if sub_count == 0:
                idx2 = i
                break
        if idx < 0 and idx2 < 0:
            raise Exception("get_func_def error: {} not a function or var".format(code))
        if idx2 < 0 or idx < idx2 or (idx3 >=0 and idx3 < start_idx):
            def_type = "var"
            definition = code[:idx]
            end_idx = idx + start_idx
        else:
            def_type = "func"
            definition = code[:idx2 + 1]
            end_idx = idx2 + 1 + start_idx
    return def_type, definition.strip(), (start_idx, end_idx)

def get_enum_values(code):
    '''
        @param code is returned by get_code_def()
               format: 'enum ABC { ... }'
    '''
    values = []
    enum_values = code.rsplit("{", 1)[-1].rsplit("}", 1)[0].split("\n")
    have_block_comment = False
    block_comment_info = ["", ""]
    for enum_value in enum_values:
        enum_value = enum_value.strip()
        if (not enum_value) or enum_value.strip().startswith("//"):
            continue
        comment = ""
        if not have_block_comment:
            enum_value = enum_value.split(",", 1)
            while 1:
                if len(enum_value) > 1 and enum_value[1] != "": # have comment
                    comment = enum_value[1].strip()
                    if comment.startswith("/*"):
                        idx_c_end = comment.find("*/")
                        if idx_c_end < 0:
                            have_block_comment = True
                            block_comment_info[0] = enum_value[0]
                            block_comment_info[1] = comment[2:].lstrip("*").strip()
                        else:
                            comment = comment[:idx_c_end][2:].lstrip("*").rstrip("*").strip()
                    elif comment.startswith("//"):
                        comment = comment.lstrip("/").strip()
                    break
                else:
                    # ['KIND_MAX      /* Max Kind quantity']
                    enum_value = enum_value[0]
                    idx_c = enum_value.find("/*")
                    if idx_c > 0:
                        enum_value = enum_value[:idx_c].strip(), enum_value[idx_c:].strip()
                        continue
                    idx_c = enum_value.find("//")
                    if idx_c > 0:
                        enum_value = enum_value[:idx_c], enum_value[idx_c:]
                        continue
                    enum_value = [enum_value, ""]
                    break

            enum_value = enum_value[0]
            if have_block_comment:
                continue
        else:
            if "*/" in enum_value:
                block_comment_info[1] += enum_value.strip()[:-2].rstrip("*")
                have_block_comment = False
                enum_value = block_comment_info[0]
                comment = block_comment_info[1]
            else:
                block_comment_info[1] += "\n" + enum_value.strip().rstrip("*")
                continue
        if "=" in enum_value:
            enum_value, value = enum_value.split("=", 1)
            value = value.strip()
        else:
            value = ""
        values.append([enum_value.strip(), value, comment])
    return values

def get_var_name_value(definition):
    if "=" in definition:
        name, value = definition.rsplit("=", 1)
        value = value.strip()
    elif "{" in definition:
        name, value = definition.rsplit("{", 1)
        value = "{" + value.strip()
    elif ";" in definition:
        name = definition.rsplit(";", 1)[0]
        value = None
    else:
        value = None
        name = definition.strip()
    words = name.split()
    new_words = []
    for word in words:
        # remove word startswith const, static, volatile, mutable, __attribute__, __restrict, extern
        rms = ["const", "static", "volatile", "mutable", "__attribute__", "__restrict", "extern"]
        valid = True
        for rm in rms:
            if word.startswith(rm):
                valid = False
                break
        if valid:
            new_words.append(word)
    name = new_words[-1].strip()
    if name.startswith("&"):
        name = name.rsplit("&", 1)[1]
    elif name.startswith("*"):
        name = name.rsplit("*", 1)[1]
    return name, value


def get_func_def_info(code):
    '''
        std::map<std::string, int> get_dict(std::map<std::string, int> in,
                    std::function<int(std::vector<int>, int)> cb,
                    int i, const char *j = "10",
                    std::vector<int> v = {1, 2, 3},
                    std::vector<int> &v2 = std::vector<int>()),
                    ...
        {}
    '''
    def get_param_values(param_str):
        arg = param_str.strip()
        if "=" in arg:
            arg, default_value = arg.split("=", 1)
            default_value = default_value.strip()
            arg = arg.strip()
        else:
            default_value = None
        if arg.strip() == "...":
            arg_name = "..."
            arg = "..."
            arg_type = "..."
        else:
            arg_type, arg = arg.rsplit(" ", 1)
            if "*" in arg:
                arg_name = arg.rsplit("*", 1)
                arg = arg_name[1].strip()
                arg_type += " {}*".format(arg_name[0])
            elif "&" in arg:
                arg_name = arg.rsplit("&", 1)
                arg = arg_name[1].strip()
                arg_type += " {}&".format(arg_name[0])
        return [arg_type, arg, default_value]

    def find_parentheses_pair(code):
        '''
            Find first parentheses pair in code, there maybe multiple sub parentheses pair, need skip them
            @return ['std::map<std::string, int> get_dict', 'std::map<std::string, int> in,
                    int i, const char *j = "10",
                    std::vector<int> v = {1, 2, 3},
                    std::vector<int> &v2 = std::vector<int>()']
        '''
        start_idx = code.find("(")
        sub_count = 1
        # find (, sub_count + 1, find ), sub_count - 1, until sub_count == 0
        for i in range(start_idx + 1, len(code)):
            if code[i] == "(":
                sub_count += 1
            elif code[i] == ")":
                sub_count -= 1
            if sub_count == 0:
                end_idx = i
                break
        return code[:start_idx].strip(), code[start_idx + 1:end_idx].strip()

    args = []
    # ret_code, params_code = find_parentheses_pair(code)
    idx = code.find("(")
    ret_code = code[:idx].strip()
    params_code = code[idx + 1:-1].strip()
    ret_type_name = ret_code.rsplit(" ", 1)
    if len(ret_type_name) == 1:
        func_name = ret_type_name[0]
        return_type = None
    else:
        return_type, func_name = ret_type_name
        func_name = re.findall(r"([\*&]*)([\S]+)", func_name)[0]
        return_type = (return_type + func_name[0]).strip()
        func_name = func_name[1]
        if return_type.startswith("static") or return_type.startswith("extern"):
            return_type = return_type.split(" ", 1)[1].strip()
    func_name = func_name.strip()
    except_pair = {
        "<": ">",
        "{": "}"
    }
    args_str = params_code.replace("\n", " ")
    if args_str:
        except_start = []
        param_str = ""
        # e.g. std::function<int(std::vector<int>, int)> cb
        for i in range(len(args_str)):
            if except_start:
                if args_str[i] in ["<", "{"]: # more <
                    except_start.append(args_str[i])
                if args_str[i] == except_pair[except_start[-1]]:
                    except_start.pop()
                param_str += args_str[i]
                continue
            if args_str[i] in ["<", "{"]:
                except_start = [args_str[i]]
                param_str += args_str[i]
                continue
            if args_str[i] == ",":
                args.append(get_param_values(param_str))
                param_str = ""
                continue
            param_str += args_str[i]
        if param_str:
            args.append(get_param_values(param_str))
    return func_name, args, return_type

def find_comments(code, add_py_doc = True):
     # get all /** ... */ comments
    comments = re.findall(r"/\*\*[\s\S]*?\*/", code, re.MULTILINE)

    # parse all keywords of comments, first key brief can have no key
    comments_parsed = []
    for comment in comments:
        item = {
            # "raw_comment": comment
        }
        # get comment start index of code
        idx = code.find(comment)
        item["start_idx"] = idx
        comment = comment[3:-2]
        comment = re.sub(r"\n\s*\*\s{1,3}", "\n", comment) # replace * at the start of each line with \n
        comment = re.sub(r"\n\s*\*", "\n", comment).strip()
        if not comment.startswith("@"):
            idx = comment.find("@")
            if idx == -1: # only have brief
                comments_parsed.append({"brief": comment})
                continue
            item["brief"] = comment[:idx].strip()
            comment = comment[idx:]
        # get all @key value, value can be multi line
        curr_key = None
        curr_value = []

        def add_kv(item, key, value, comment):
            # check docxygen format keys
            value_key = None
            if key.startswith("param"):
                key = key.split("[")
                key, dir = key[0], key[1][:-1] if len(key) > 1 else None
                value0 = value.split("\n", 1)
                value = value.split(" ", 1)
                if len(value0[0]) < len(value[0]):
                    value = value0
                if len(value) <= 1:
                    raise Exception("param must have detail description, DO NOT lazy!!")
                value_key, value = value
                value = "{}{}".format("direction [{}], ".format(dir) if dir else "", value.strip())
            elif key.startswith("retval"):
                value = value.split(" ", 1)
                if len(value) <= 1:
                    raise Exception("retval must have detail description, DO NOT lazy!!")
                value_key, value = value

            if key in item:
                if value_key:
                    if type(item[key]) != dict:
                        raise Exception("-- Comment error, check comment format,\nkey: {},\ncomment: {}".format(key, comment))
                    item[key][value_key] = value
                else:
                    if type(item[key]) != list:
                        item[key] = [item[key]]
                    item[key].append(value)
            else:
                if value_key:
                    item[key] = {value_key: value}
                else:
                    item[key] = value

        for line in comment.split("\n"):
            line = line.strip()
            if line.startswith("@"):
                if curr_key:
                    add_kv(item, curr_key, "\n".join(curr_value), comment)
                    curr_value = []
                curr_key = re.findall(r"^@(\S+)", line)[0]
                line = line[len(curr_key) + 1:]
            curr_value.append(line.strip())
        if curr_key:
            add_kv(item, curr_key, "\n".join(curr_value), comment)
        if "brief" not in item and ("maixpy" in item or "maixcdk" in item):
            raise Exception("-- Comment error, no brief keyword,\ncomment: {}".format(comment))
        if add_py_doc:
            item["py_doc"] = item.get("brief", "")
            params = item.get("param", {})
            if len(params) > 0:
                item["py_doc"] += "\n\nArgs:\n"
                for k,v in params.items():
                    if k in ["brief", "maixpy", "maixcdk"]:
                        continue
                    item["py_doc"] += "  - {}: {}\n".format(k, v)
            return_info = item.get("return", "")
            retvals =  item.get("retval", {})
            if return_info or len(retvals) > 0:
                item["py_doc"] += "\n\nReturns: {}\n".format(return_info)
                for k,v in retvals.items():
                    item["py_doc"] += "  - {}: {}\n".format(k, v)
        comments_parsed.append(item)
    return comments_parsed

def parse_api_item_info(item):
    info = {}
    item = item.split("\n")
    api = item[0].strip()
    info["kv"] = {}
    for kv_str in item[1:]:
        if kv_str.startswith(":") and len(kv_str) > 1:
            kv_str = kv_str[1:].strip()
            kv = kv_str.split(" ", 1)
            if len(kv) == 1:
                info["kv"][kv[0]] = True
            elif len(kv) == 2:
                info["kv"][kv[0]] = kv[1]
            else:
                raise Exception("parse_api_info key {} value error: {} => {}".format(api, item, kv_str))
    return api, info

def parse_api(code, apis, sdks = ["maixpy", "maixcdk"], header_name = None, module_name = "maix", header_path = None):

    # if not header_name.endswith("api_example.hpp"):
        # return apis, "", False, []

    def parse_item_code_def(item):
        def_code = code[item["start_idx"]:]
        idx = def_code.find(api)
        idx = def_code.find("*/", idx) + 2
        idx = def_code.find("\n", idx) + 1
        item["type"], definition, (idx, idx2) = get_code_def(def_code[idx:])
        # parse args values flags etc.
        if item["type"] == "class":
            words = definition.split("\n")[0].split()
            if "class" in words:
                item["name"] = words[words.index("class") + 1].split(":")[0].split("{")[0]
            else:
                item["name"] = words[words.index("struct") + 1].split(":")[0].split("{")[0]
        elif item["type"] == "enum":
            item["values"] = get_enum_values(definition)
            words = definition.split("\n")[0].split()
            if "class" in words:
                item["name"] = words[words.index("class") + 1].split(":")[0].split("{")[0]
            else:
                item["name"] = words[words.index("enum") + 1].split(":")[0].split("{")[0]
        elif item["type"] == "func":
            item["type"] = "func"
            item["name"], item["args"], item["ret_type"] = get_func_def_info(definition)
        elif item["type"] == "var":
            item["name"], item["value"] = get_var_name_value(definition)
            if definition.startswith("const"):
                item["kv"]["readonly"] = True
        elif item["type"] == "module":
            item["name"] = definition.split()[1].split(":")[-1]
        else:
            raise Exception("item type no valid: {}, {}".format(item["type"], definition))
        if definition.startswith("static"):
            item["kv"]["static"] = True
        item["def_idx"] = [idx, idx2]
        item["def"] = definition

    comments_parsed = find_comments(code)

    # find all @maixpy items
    # items = {} # {'maix.example.Example.__init__': {'kv': {'hello': True}}}
    items = {}
    for comment in comments_parsed:
        sdk_name = None
        for sdk in sdks:
            if sdk in comment:
                sdk_name = sdk
        if not sdk_name:
            continue
        api, info = parse_api_item_info(comment[sdk_name])
        info["doc"] = comment
        comment["brief"] = comment["brief"].replace('"', '\\"').replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t")
        info["start_idx"] = comment["start_idx"]
        comment.pop("start_idx")
        if api in items:
            if ["maixpy"] == sdks: # only for maixpy, not allow overload
                try:
                    parse_item_code_def(info)
                    parse_item_code_def(items[api])
                    def1 = info["def"]
                    def2 = items[api]["def"]
                except Exception:
                    def1 = ""
                    def2 = ""
                return None, "parse_api error:\n\nAPI \033[1;31m {} \033[0m multiple defined !!!\n\n\033[1;31mOne\033[0m:\n  \033[1;33m{}\033[0m\n    brief: {}\n    info: {},\n\n\033[1;31mAnother\033[0m:\n  \033[1;33m{}\033[0m\n    brief: {}\n    info: {}\n".format(
                    api, def1, info["doc"]["brief"], info, def2, items[api]["doc"]["brief"], items[api]), False, []
            print("-- API {} is overloaded".format(api))
            if not "overload" in items[api]:
                items[api]["overload"] = []
            items[api]["overload"].append(info)
        else:
            items[api] = info

    # parse the api definition after @maixpy comment block
    # final items:
    '''
    {
        'maix.example.Example': {
            'type': 'var',
            'value': '"hello"',
            'kv': {
                'readonly': False,
                'static': False
            },
            'doc': {'brief': '****'},
            'def': 'class Example',
            'def_idx': [1444, 1467]
        },
        'maix.example.Example.__init__': {
            'type': 'func',
            'args': [
                ['const char *', 'i', None], # [type, key name, default value(string type)]
            ],
            'kv': {
                'static': False
            },
            'doc': {'brief': '****'},
            'def': 'class Example',
            'def_idx': [1444, 1467]
        },
        'maix.example.Kind': {
            'type': 'enum',
            'values': {
                "KIND_DOG": '0',
            },
            'kv': {
                'readonly': False,
                'static': False
            },
            'doc': {'brief': '****'},
            'def': 'class Example',
            'def_idx': [1444, 1467]
        },
    }
    '''

    for api, item in items.items():
        parse_item_code_def(item)
        py_name = api.split(".")[-1]
        if not py_name.startswith("__") and item["name"] != py_name:
            return None, "API name \033[1;31m{} \033[0m not match with code definition name \033[1;31m{}\033[0m, def: {}".format(
                api, item["name"], item["def"]), False, []
        for overload in item.get("overload", []):
            parse_item_code_def(overload)

    # generate module tree
    apis_default = {
        "type": "top_module",
        "members": {
            module_name: {
                "type": "module",
                "doc": {"breif": "MaixPy C/C++ API from MaixCDK"},
                "members":{
                # "var1": {
                #     "type": "var",
                #     "value": "",
                #     "static": False,
                #     "readonly": False,
                # },
                # "func1": {
                #     "type": "func",
                #     "args": [
                #         ['const char *', "i", None],  # [type, key name, default value]
                #         ['const char *', "j", 10]
                #     ],
                #     "doc": {}, # comment except @maixpy section
                #     "static": False
                # },
                # "class1": {
                #     "type": "class",
                #     "members": {}
                # },
                # "enum1": {
                #     "type": "enum",
                #     "values": [
                #         ("KIND_DOG", 0)
                #     ]
                # },
                },
                # "module1": {
                #     "type": "module",
                #     "doc": {"brief": "MaixPy C/C++ API from MaixCDK"},
                #     "members":{},
                # }
            }
        }
    }
    if len(apis.keys()) == 0:
        # copy
        for k in apis_default:
            apis[k] = apis_default[k]
    # find type and generate tree
    # sort items' keys by key's name length(split by .)
    keys = sorted(items.keys(), key=lambda x: len(x.split(".")))
    final_keys = []
    # add missing module
    for api in keys:
        item = items[api]
        # parse api name
        names = api.split(".")
        # to limit only have maix module
        if names[0] != module_name:
            return None, "API name must start with {}, api: {} ".format(module_name, api), False, []
        for i in range(len(names)):
            key = ".".join(names[:i+1])
            if key not in items:
                if key != module_name:
                    print("-- Auto add module {}, < {} >".format(key, header_name))
                items[key] = {"type": "module", "doc": {"brief": "{} module".format(key)}, "members": {}, "auto_add": True}
                final_keys.append(key)
        final_keys.append(api)

    # generate tree
    def get_parent_node(root, api):
        parents = api.split(".")
        api_name = parents.pop()
        parent = root
        for name in parents:
            parent = parent["members"][name]
        if api_name in parent["members"]:
            msg = "parse_api error: \033[1;31m{}\033[0m multiple define in different headers".format(api)
            if "header_path" in parent["members"][api_name]:
                msg += f": \033[1;31m{parent['members'][api_name]['header_path']}\033[0m and \033[1;31m{header_path}\033[0m"
            raise Exception(msg)
        return parent, api_name

    for key in final_keys:
        item = items[key]
        # !! assume all parent already exists first
        if item["type"] == "module":
            names = key.split(".")
            if not "name" in item:
                item["name"] = key.replace(".", "::")
            define_names = item["name"].split("::")
            module = apis
            if not "members" in item:
                item["members"] = {}
            curr_names = []
            for name in names:
                curr_names.append(name)
                item_doc = items[".".join(curr_names)]["doc"]
                item_members = items[".".join(curr_names)]["members"]
                item_auto_add = items[".".join(curr_names)]["auto_add"] if "auto_add" in items[".".join(curr_names)] else False
                if name not in module["members"]:
                    module["members"][name] = {"type": "module", "doc": item_doc, "members": item_members, "auto_add": item_auto_add}
                elif "auto_add" in module["members"][name] and module["members"][name]["auto_add"]:
                    if not item_auto_add:
                        new_members = {}
                        new_members.update(module["members"][name]["members"])
                        new_members.update(item_members)
                        module["members"][name]["doc"] = item_doc
                        module["members"][name]["members"] = new_members
                module = module["members"][name]
        elif item["type"] == "var":
            parent, name = get_parent_node(apis, key)
            parent["members"][name] = {
                        "type": "var",
                        "name": item["name"],
                        "doc": item["doc"],
                        "value": item["value"],
                        "static": item["kv"].get("static", False),
                        "readonly": item["kv"].get("readonly", False),
                        "def": item["def"],
                        "header_path": header_path
                    }
        elif item["type"] == "class":
            parent, name = get_parent_node(apis, key)
            parent["members"][name] = {
                        "type": "class",
                        "name": item["name"],
                        "doc": item["doc"],
                        "members": {},
                        "def": item["def"],
                        "header_path": header_path
                    }
        elif item["type"] == "func":
            parent, name = get_parent_node(apis, key)
            parent["members"][name] = {
                        "type": "func",
                        "name": item["name"],
                        "doc": item["doc"],
                        "args": item["args"], # [["const char *", "i", None], ["int", "j", "10"]]
                        "ret_type": item["ret_type"],
                        "static": item["kv"].get("static", False),
                        "def": item["def"],
                        "header_path": header_path
                    }
            # overload method
            if "overload" in item:
                parent["members"][name]["overload"] = []
                for overload in item["overload"]:
                    parent["members"][name]["overload"].append({
                            "type": "func",
                            "name": overload["name"],
                            "doc": overload["doc"],
                            "args": overload["args"], # [["const char *", "i", None], ["int", "j", "10"]]
                            "ret_type": overload["ret_type"],
                            "static": overload["kv"].get("static", False),
                            "def": overload["def"],
                            "header_path": header_path
                        })
        elif item["type"] == "enum":
            parent, name = get_parent_node(apis, key)
            parent["members"][name] = {
                        "type": "enum",
                        "name": item["name"],
                        "doc": item["doc"],
                        "values": item["values"], # [("KIND_DOG", "0"), ("KIND_CAT", None)]
                        "def": item["def"],
                        "header_path": header_path
                    }
        else:
            return None, "parse_api error: {} not support when generate tree".format(item["type"]), False, []

    return apis, "", len(final_keys) > 0, keys

def parse_api_from_header(header_path, api_tree = {}, sdks = ["maixpy"], module_name = "maix"):
    with open(header_path, "r", encoding="utf-8") as f:
        code = f.read()
    try:
        api_tree, msg, updated, keys = parse_api(code, api_tree, sdks, os.path.basename(header_path), module_name, header_path=header_path)
    except Exception as e:
        import traceback
        traceback.print_exc()
        raise Exception("\n\nparse_api_from_header \033[1;31m {} \033[0m error:\n{}".format(header_path, e))
    if api_tree is None:
        raise Exception("\n\nparse_api_from_header \033[1;31m {} \033[0m error:\n{}".format(header_path, msg))
    return api_tree, updated, keys


if __name__ == "__main__":
    print("-- Generate MaixPy C/C++ API")
    parser = argparse.ArgumentParser(description='Generate MaixPy C/C++ API')
    parser.add_argument('--vars', type=str, default="", help="CMake global variables file, a json file in build/config dir")
    parser.add_argument('--doc', type=str, default="", help="API documentation output file")
    parser.add_argument("--sdk_path", type=str, default="", help="MaixCDK SDK path")
    parser.add_argument("--module_name", type=str, default="maix", help="module name")
    args = parser.parse_args()

    t = time.time()

    # get header files
    headers = []
    if args.vars:
        with open(args.vars, "r", encoding="utf-8") as f:
            vars = json.load(f)
        for include_dir in vars["includes"]:
            headers += get_headers_recursive(include_dir)
    else: # add sdk_path/components all .h and .hpp header files, except 3rd_party components
        except_dirs = ["components/3rd_party"]
        for root, dirs, files in os.walk(os.path.join(args.sdk_path, "components")):
            ignored = False
            for except_dir in except_dirs:
                if os.path.join(args.sdk_path, except_dir) in root:
                    ignored = True
                    break
            if ignored:
                continue
            for name in files:
                path = os.path.join(root, name)
                if path.endswith(".h") or path.endswith(".hpp"):
                    headers.append(path)

    # check each header file to find API
    api_tree = {}
    rm = []
    all_keys = {}
    for header in headers:
        api_tree, updated, keys = parse_api_from_header(header, api_tree, sdks = ["maixpy", "maixcdk"], module_name=args.module_name)
        if not updated:
            rm.append(header)
        for h, ks in all_keys.items():
            for k in ks:
                if k in keys:
                    raise Exception("API {} multiple defined in {} and {}".format(k, h, header))
        all_keys[header] = keys

    for r in rm:
        headers.remove(r)

    # generate API documentation according to api_tree
    print("-- Generating MaixCDK API documentation")
    doc_out_dir = args.doc
    if not os.path.exists(doc_out_dir):
        os.makedirs(doc_out_dir)
    api_json_path = os.path.join(doc_out_dir, "api.json")
    side_bar_path = os.path.join(doc_out_dir, "sidebar.yaml")
    readme_path = os.path.join(doc_out_dir, "README.md")
    sidebar = {
        "items": [
            {
                "label": "Brief",
                "file": "README.md"
            }
        ]
    }
    with open(api_json_path, "w", encoding="utf-8") as f:
        json.dump(api_tree, f, indent=4)

    doc_maix_sidebar = {
        "label": args.module_name,
        "collapsed": False,
        "items": [
            # {
            #     "label": "example",
            #     "file": "maix/example.md"
            # }
        ]
    }
    sidebar["items"].append(doc_maix_sidebar)
    start_comment_template = '''
> This is `{}` module of [MaixCDK](https://github.com/sipeed/MaixCDK).
> All of these elements are in namespace `{}`.
>
> For MaixCDK developer: DO NOT edit this doc file manually, this doc is auto generated!
\n'''
    top_api_keys = [args.module_name]
    module_members = api_tree["members"][args.module_name]["members"]
    def gen_modules_doc(module_members, parents):
        sidebar_items = []
        for m, v in module_members.items():
            if v["type"] == "module":
                item = {
                        "label": m,
                        "collapsed": False,
                        "file": "{}.md".format("/".join(parents) + "/" + m)
                    }
                sidebar_items.append(item)
                api_file = os.path.join(doc_out_dir, item["file"])
                os.makedirs(os.path.dirname(api_file), exist_ok = True)
                module_full_name = "::".join(parents + [m])
                start_comment = start_comment_template.format(module_full_name, module_full_name)
                content = module_to_md(parents, m, v, start_comment, module_join_char = "::")
                with open(api_file, "w", encoding="utf-8") as f:
                    f.write(content)
                # find submodule
                for _k, _v in v["members"].items():
                    if _v["type"] == "module":
                        item["items"] = gen_modules_doc(v["members"], parents + [m])
        return sidebar_items

    sidebar_items = gen_modules_doc(module_members, top_api_keys)
    doc_maix_sidebar["items"] += sidebar_items
    with open(side_bar_path, "w", encoding="utf-8") as f:
        yaml.dump(sidebar, f, indent=4)

    readme = '''
---
title: MaixCDK API -- Maix AI machine vision platform C/C++ API
---

> For MaixCDK developer: This API documentation is generated from the source code, DO NOT edit this file manually!


MaixCDK API documentation, modules:

'''
    readme += "| module | brief |\n"
    readme += "| --- | --- |\n"
    for m, v in module_members.items():
        # add link to module api doc
        readme += "|[{}::{}](./{}/{}.md) | {} |\n".format(args.module_name, m, m, args.module_name, v["doc"]["brief"].replace("\n", "<br>")
                    )
    with open(readme_path, "w", encoding="utf-8") as f:
        f.write(readme)

    print("-- Generate MaixCDK API doc complete ({:.2f}s)".format(time.time() - t))
