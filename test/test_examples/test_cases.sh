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
    fi
}


cd ../../examples

test_start hello_world
test_start camera_display
test_start comm_protocol
test_start comm_uart
test_start gui_lvgl
test_start gui_simple
test_start i18n
test_start image_method
test_start image_show
test_start key_demo
test_start multi_threads
test_start nn_classifier
test_start nn_runner
test_start nn_yolov5
test_start opencv_demo
test_start peripheral_gpio
test_start peripheral_i2c_eeprom
test_start peripheral_pwm_lcd_backlight
test_start rtsp_demo
test_start video_record_mp4
test_start websocket_client
test_start websocket_server
test_start wifi_demo




