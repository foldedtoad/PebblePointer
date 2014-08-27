#!/bin/bash

# clean, build, install and monitor script.

if [ -z "$VAR" ]; then
    echo "Please set the environment variable PEBBLE_IP to teh IP address of your device with a Pebble paired with it."
    exit 1
fi

pebble clean
pebble build
pebble install --phone $PEBBLE_IP
pebble logs    --phone $PEBBLE_IP
