#!/bin/bash

SCRIPT_NAME=$(basename "${0}")
if [ "$(whoami)" != "root" ]; then
        echo "${SCRIPT_NAME} - ERROR: Run this script as a root user"
        exit 1
fi

sh -c 'echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb'
