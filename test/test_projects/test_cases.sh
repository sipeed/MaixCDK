#!/bin/bash

set -e
set -x

platform=$1
need_run=$2

cd ../../examples

cd hello_world
maixcdk distclean
maixcdk build -p $platform
ls -al build/hello_world | awk '{print "\nDebug program file size: " $5 "B\n"}'
if [ "${need_run}x" == "1x" ]; then
    maixcdk run
fi
maixcdk clean
maixcdk distclean
maixcdk build --release -p $platform
ls -al build/hello_world | awk '{print "\nRelease program file size: " $5 "B\n"}'
maixcdk distclean
cd ..


cd multi_threads
maixcdk distclean
maixcdk build -p $platform
ls -al build/multi_threads | awk '{print "\nDebug program file size: " $5 "B\n"}'
if [ "${need_run}x" == "1x" ]; then
    maixcdk run
fi
maixcdk clean
maixcdk distclean
maixcdk build --release -p $platform
ls -al build/multi_threads | awk '{print "\nRelease program file size: " $5 "B\n"}'
maixcdk distclean
cd ..


# opencv_demo
cd opencv_demo
maixcdk distclean
maixcdk build -p $platform
ls -al build/opencv_demo | awk '{print "\nDebug program file size: " $5 "B\n"}'
if [ "${need_run}x" == "1x" ]; then
    maixcdk run
fi
maixcdk clean
maixcdk distclean
maixcdk build --release -p $platform
ls -al build/opencv_demo | awk '{print "\nRelease program file size: " $5 "B\n"}'
maixcdk distclean
cd ..


