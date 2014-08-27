#!/bin/bash

# clean, build, install and monitor script.

if [ -z "$PEBBLE_IP" ]; then
    echo "Please set the environment variable PEBBLE_IP to the IP address of your device with a Pebble paired with it."
    exit 1
fi

pebble clean
pebble build
pebble install --phone $PEBBLE_IP
pebble logs    --phone $PEBBLE_IP
