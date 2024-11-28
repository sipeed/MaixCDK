'''
    @author neucrack@sipeed.com
    @license MIT
'''
'''
    {
        "Kind": {
            "type": "enum",
            "doc": {
                "brief": "Example enum",
                "maixpy": "maix.example.Kind"
            },
            "values": [
                [
                    "KIND_DOG",
                    "0"
                ],
                [
                    "KIND_CAT",
                    null
                ],
                [
                    "KIND_MAX",
                    null
                ]
            ]
        },
        "var1": {
            "type": "var",
            "doc": {
                "brief": "Example module variable",
                "maixpy": "maix.example.var1"
            },
            "value": "\"Sipeed\"",
            "static": false,
            "readonly": false
        },
        "Example": {
            "type": "class",
            "doc": {
                "brief": "Example class\nthis class will be export to MaixPy as maix.example.Example",
                "maixpy": "maix.example.Example"
            },
            "members": {
                "__init__": {
                    "type": "func",
                    "doc": {
                        "brief": "Example constructor\nthis constructor will be export to MaixPy as maix.example.Example.__init__",
                        "param": [
                            "name name of Example, string type",
                            "age age of Example, int type, default is 18, value range is [0, 100]"
                        ],
                        "maixpy": "maix.example.Example.__init__\n:opt value\n:"
                    },
                    "args": [
                        [
                            "std::string &",
                            "name",
                            null
                        ],
                        [
                            "int",
                            "age",
                            "18"
                        ]
                    ],
                    "static": false
                },
                "get_name": {
                    "type": "func",
                    "doc": {
                        "brief": "get name of Example",
                        "return": "name of Example, string type",
                        "maixpy": "maix.example.Example.get_name"
                    },
                    "args": [],
                    "static": false
                },
                "get_age": {
                    "type": "func",
                    "doc": {
                        "brief": "get age of Example",
                        "return": "age of Example, int type, value range is [0, 100]",
                        "maixpy": "maix.example.Example.get_age"
                    },
                    "args": [],
                    "static": false
                },
                "set_name": {
                    "type": "func",
                    "doc": {
                        "brief": "set name of Example",
                        "param": "name name of Example, string type",
                        "maixpy": "maix.example.Example.set_name"
                    },
                    "args": [
                        [
                            "std::string",
                            "name",
                            null
                        ]
                    ],
                    "static": false
                },
                "set_age": {
                    "type": "func",
                    "doc": {
                        "brief": "set age of Example",
                        "param": "age age of Example, int type, value range is [0, 100]",
                        "maixpy": "maix.example.Example.set_age"
                    },
                    "args": [
                        [
                            "int",
                            "age",
                            null
                        ]
                    ],
                    "static": false
                },
                "hello": {
                    "type": "func",
                    "doc": {
                        "brief": "say hello to someone",
                        "param": "name name of someone, string type",
                        "return": "string type, content is Example::hello_str + name",
                        "maixpy": "maix.example.Example.hello"
                    },
                    "args": [
                        [
                            "std::string",
                            "name",
                            null
                        ]
                    ],
                    "static": true
                },
                "name": {
                    "type": "var",
                    "doc": {
                        "brief": "name member of Example",
                        "maixpy": "maix.example.Example.name"
                    },
                    "value": "std::string name",
                    "static": false,
                    "readonly": false
                },
                "age": {
                    "type": "var",
                    "doc": {
                        "brief": "age member of Example, value range should be [0, 100]",
                        "maixpy": "maix.example.Example.age"
                    },
                    "value": "int age",
                    "static": false,
                    "readonly": false
                },
                "hello_str": {
                    "type": "var",
                    "doc": {
                        "brief": "hello_str member of Example, default value is \"hello \"",
                        "maixpy": "maix.example.Example.hello_str"
                    },
                    "value": "static std::string hello_str",
                    "static": true,
                    "readonly": false
                }
            }
        }
    }
'''

'''
---
title: prefix + module_name
date: 2023-09-14
---

module doc

## Enum

### Kind

* brief: Example enum
* maixpy: maix.example.Kind
* values:
  * KIND_DOG
  * KIND_CAT
  * KIND_MAX
* **C++ defination code**: `enum Kind {KIND_DOG, KIND_CAT, KIND_MAX};`

## Variable

### var1

* brief: Example module variable
* maixpy: maix.example.var1
* value: "Sipeed"
* readonly: false
* **C++ defination code**: `std::string var1 = "Sipeed";`

## Function

### hello

* brief: say hello to someone
* maixpy: maix.example.Example.hello
* param:
  * name name of someone, string type
* return: string type, content is name
* **C++ defination code**: `std::string hello(std::string name);`


## Class

### Example

* brief: Example class
* maixpy: maix.example.Example
* **C++ defination code**: `class Example;`

#### __init__

* type: function
* brief: Example constructor
* maixpy: maix.example.Example.__init__
* param:
    * name name of Example, string type
    * age age of Example, int type, default is 18, value range is [0, 100]
* return: None
* static: false
* **C++ defination code**: `Example(std::string &name, int age = 18);`

#### name

* type: variable
* brief: name member of Example
* maixpy: maix.example.Example.name
* value: std::string name
* static: false
* readonly: false
* **C++ defination code**: `std::string name;`


'''

def multilines_add_prefix(content, prefix):
    lines = content.split("\n")
    new_lines = []
    for line in lines:
        new_lines.append(prefix + line)
    return "\n".join(new_lines)

def module_to_md(pre_modules, module_name, module, start_comment, module_join_char = "."):
    pre_modules_str = module_join_char.join(pre_modules)
    m_doc = module["doc"] if type(module["doc"]) == str else module["doc"]["brief"]
    content = '---\ntitle: {}{}{}\n---\n\n{}\n\n'.format(
        pre_modules_str, module_join_char, module_name, m_doc)
    content += start_comment
    skip_keys = ["maixpy", "maixcdk", "py_doc", "brief"]
    def have_doc_kv(doc_comment):
        keys = list(doc_comment.keys())
        valid_keys = []
        for k in keys:
            if k in skip_keys:
                continue
            valid_keys.append(k)
        return len(valid_keys) > 0

    def gen_md_doc_from_comment(doc_comment):
        content = ''
        for doc_k, v in doc_comment.items():
            if doc_k in skip_keys:
                continue
            _content = ''
            if type(v) == list:
                _content += '| **{}** | '.format(doc_k)
                for i, _v in enumerate(v):
                    _content += '**{}**. {}<br>'.format(i + 1, _v.replace('\n', '<br>'))
                _content += '|\n'
            elif type(v) == dict:
                _content += '| **{}** | '.format(doc_k)
                for _k, _v in v.items():
                    _content += '**{}**: {}<br>'.format(_k, _v.replace('\n', '<br>'))
                _content += "|\n"
            if not _content:
                _content += '| **{}** | {} |\n'.format(doc_k, v.replace('\n', '<br>'))
            content += _content
        return content

    ids = {
        # "module": {
        #     "id": "Module",
        #     "count": 1
        # },
    }
    def update_id(h_str, id) -> str:
        l_id = id.lower()
        if l_id not in ids:
            ids[l_id] = {
                "id": id,
                "count": 1
            }
            return f'{h_str} {id} {{#{id}}}'
        ids[l_id]["count"] += 1
        return f'{h_str} {id} {{#{id}-{ids[l_id]["count"]}}}'

    # Module
    content += update_id("##", "Module") + "\n\n"
    _content = ""
    for key, item in module["members"].items():
        if item["type"] != "module":
            continue
        _content += '| [{}](./{}/{}.md) | {} |\n'.format(key.replace("_", "\_"), module_name, key, item["doc"]["brief"].replace('\n', '<br>'))
    if _content:
        content += '| module | brief |\n'
        content += '| --- | --- |\n'
        content += _content
    else:
        content += "No module\n"
    content += "\n\n"

    # Enum
    content += update_id("##", "Enum") + "\n\n"
    for key, item in module["members"].items():
        if item["type"] != "enum":
            continue
        content += update_id("###", key.replace("_", "\_")) + "\n\n"
        content += item["doc"].get("brief", "") + "\n\n"
        content += '| item | describe |\n'
        content += '| --- | --- |\n'
        content += gen_md_doc_from_comment(item["doc"])
        content += '| **values** | '
        for value in item["values"]:
            content += '**{}**: {}<br>'.format(value[0], value[2].replace('\n', '<br>'))
        content += '\n> C++ defination code:\n> ```cpp\n{}\n> ```\n'.format(multilines_add_prefix(item["def"], "> "))
    content += "\n\n"

    # Variable
    content += update_id("##", "Variable") + "\n\n"
    for key, item in module["members"].items():
        if item["type"] != "var":
            continue
        content += update_id("###", key.replace("_", "\_")) + "\n\n"
        content += item["doc"].get("brief", "") + "\n\n"
        content += '| item | description |\n'
        content += '| --- | --- |\n'
        content += gen_md_doc_from_comment(item["doc"])
        content += '| **value** | **{}** |\n'.format(item["value"].replace('\n', '<br>')) if item["value"] else ""
        content += '| **readonly**| {} |\n'.format(item["readonly"])
        content += '\n> C++ defination code:\n> ```cpp\n{}\n> ```\n'.format(multilines_add_prefix(item["def"], "> "))
    content += "\n\n"

    # Function
    content += update_id("##", "Function") + "\n\n"
    def gen_func_info(key, item, overload_count = -1):
        if overload_count >= 0:
            key = "{} (overload {})".format(key, overload_count + 1)
        content = update_id('###', key.replace("_", "\_")) + "\n\n"
        if "py_def" in item:
            content += f'```python\n{item["py_def"]}\n```\n'
        content += item["doc"].get("brief", "") + "\n\n"
        if have_doc_kv(item["doc"]):
            content += '| item | description |\n'
            content += '| --- | --- |\n'
            content += gen_md_doc_from_comment(item["doc"])
        content += '\n> C++ defination code:\n> ```cpp\n{}\n> ```\n'.format(multilines_add_prefix(item["def"], "> "))
        return content
    for key, item in module["members"].items():
        if item["type"] != "func":
            continue
        content += gen_func_info(key, item)
        # overload methods
        if "overload" in item:
            for i, overload in enumerate(item["overload"]):
                content += gen_func_info(key, overload, i)
    content += "\n\n"

    # Class
    content += update_id("##", "Class") + "\n\n"
    for key, item in module["members"].items():
        if item["type"] != "class":
            continue
        content += update_id('###', key.replace("_", "\_")) + "\n\n"
        content += item["doc"].get("brief", "") + "\n\n"
        if have_doc_kv(item["doc"]):
            content += '| item | description |\n'
            content += '| --- | --- |\n'
            content += gen_md_doc_from_comment(item["doc"])
        content += '\n> C++ defination code:\n> ```cpp\n{}\n> ```\n'.format(multilines_add_prefix(item["def"], "> "))
        content += '\n'
        def gen_class_func_info(key, item, overload_count = -1):
            supported_types = ["func", "var"]
            if item["type"] not in supported_types:
                raise Exception("class member only support {} now, but got {}".format(supported_types, item["type"]))
            if overload_count >= 0:
                key = "{} (overload {})".format(key, overload_count + 1)
            content = update_id('####', key.replace("_", "\_")) + "\n\n"
            if "py_def" in item:
                content += f'```python\n{item["py_def"]}\n```\n'
            content += item["doc"].get("brief", "") + "\n\n"
            content += '| item | description |\n'
            content += '| --- | --- |\n'
            content += '| **type** | {} |\n'.format(item["type"])
            content += gen_md_doc_from_comment(item["doc"])
            content += '| **static** | {} |\n'.format(item["static"])
            if item["type"] == "var":
                content += '| **readonly** | {} |\n'.format(item["readonly"])
            content += '\n> C++ defination code:\n> ```cpp\n{}\n> ```\n'.format(multilines_add_prefix(item["def"], "> "))
            return content

        for key, item in item["members"].items():
            content += gen_class_func_info(key, item)
            # overload methods
            if "overload" in item:
                for i, overload in enumerate(item["overload"]):
                    content += gen_class_func_info(key, overload, i)
    return content
