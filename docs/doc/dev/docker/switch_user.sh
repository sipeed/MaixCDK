#!/bin/bash

set -e


echo "###########################################################"
echo "##"
echo "##  ███    ███  █████  ██ ██   ██  ██████ ██████  ██   ██"
echo "##  ████  ████ ██   ██ ██  ██ ██  ██      ██   ██ ██  ██"
echo "##  ██ ████ ██ ███████ ██   ███   ██      ██   ██ █████"
echo "##  ██  ██  ██ ██   ██ ██  ██ ██  ██      ██   ██ ██  ██"
echo "##  ██      ██ ██   ██ ██ ██   ██  ██████ ██████  ██   ██"
echo "##"
echo "##  ${USER}! Welcome to MaixCDK Docker Container!"
echo "###########################################################"
echo "##"

# if $MAIXCDK_PATH exists and is a directory and is not empty
if [ -n "$MAIXCDK_PATH" ] && [ -d "$MAIXCDK_PATH" ] ; then
    echo "## MAIXCDK_PATH is set to '$MAIXCDK_PATH'"
else
    echo "## MAIXCDK_PATH is not set or ${MAIXCDK_PATH} not exists"
    echo "## Please set MAIXCDK_PATH to MaixCDK path in container"
    echo "## e.g. add args \`--env MAIXCDK_PATH=/path/to/MaixCDK -v /path/to/MaixCDK:/path/to/MaixCDK\` to docker run command"
fi
# echo "## The default password of user ${USER} is 'maixcdk'"
echo "###########################################################"
echo ""

bash 
