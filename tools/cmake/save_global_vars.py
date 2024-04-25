import argparse
import json

if __name__ == "__main__":
    import sys
    parser = argparse.ArgumentParser(description='Save cmake global variables')
    parser.add_argument('-o', '--output', type=str, default="", help="Output file, json format")
    parser.add_argument('--var', default=[], nargs='+', action="append", help="Variables to save")

    args = parser.parse_args()
    vars = {}
    for var in args.var:
        key = var[0]
        if len(key) == 1:
            value = []
        else:
            value = var[1:]
        vars[key] = value
    with(open(args.output, "w", encoding="utf-8")) as f:
        json.dump(vars, f, indent=4)
