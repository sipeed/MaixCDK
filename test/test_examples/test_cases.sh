#!/bin/bash

# set -e
set -x

platform=$1
need_run=$2
if [ "${need_run}x" == "1x" ]; then
    run_cmd="maixcdk run"
else
    run_cmd=""
fi

function test_script()
{
    set +x
    echo ""
    echo "--------------------------------"
    echo "-- Test [$1] Start --"
    echo "--------------------------------"
    set -x
    cd $1
    maixcdk distclean
    maixcdk build -p $platform
    $run_cmd
    cd ..
    set +x
    echo "--------------------------------"
    echo "-- Test [$1] End --"
    echo "--------------------------------"
    echo ""
    set -x
}

function test_start()
{
    test_script $1
    if [ $? -ne 0 ]; then
        echo "Error: $1 failed to execute."
        exit 1
    fi
}


cd ../../examples

for dir in */; do
  if [ -d "$dir" ]; then
    test_start "${dir%/}"
  fi
done




