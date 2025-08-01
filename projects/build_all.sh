#!/bin/bash

set -e

platform="maixcam"

if [ -n "$1" ]; then
    platform="$1"
fi

function build_start()
{
    # test_script $1
    # if [ $? -ne 0 ]; then
    #     echo "Error: $1 failed to execute."
    #     exit 1
    # fi
    cd $1
    if [[ -f main.py ]]; then
        rm -rf dist
        maixtool release
    else
        maixcdk distclean
        maixcdk release -p "$platform" --toolchain-id default
    fi
    mkdir -p ../apps
    cp -r dist/pack/* ../apps
    cd ..
}

rm -rf apps/

for dir in */; do
  if [ -d "$dir" ]; then
    if [[ $dir == app* && $dir != apps* ]]; then
      echo "----- build ${dir} -----"
      build_start "${dir%/}"
      echo "----- build ${dir} done -----"
    fi
  fi
done

