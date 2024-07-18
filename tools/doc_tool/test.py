from gen_api import parse_api_from_header
import os


curr_dir = os.path.abspath(os.path.dirname(__file__))

header_path = os.path.join(curr_dir, "..", "..", "components", "basic", "include", "maix_api_example.hpp")
res = parse_api_from_header(header_path, api_tree = {}, for_sdk=["maixpy", "maixcdk"])
import json, os

with open("test.json", "w") as f:
    f.write(json.dumps(res, indent=4))


