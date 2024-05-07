#!/bin/bash

set -e
set -x

python -m pip install -r ../../requirements.txt

chmod +x test_cases.sh

echo "
-----------------------------------
---                             ---
---    test for board linux     ---
---                             ---
-----------------------------------
"
./test_cases.sh linux 1
echo "-- test for board linux end\n\n"

# echo "
# -----------------------------------
# ---                             ---
# ---    test for board m2dock    ---
# ---                             ---
# -----------------------------------
# "
# ./test_cases.sh m2dock 0
# echo "-- test for board m2dock end\n\n"

