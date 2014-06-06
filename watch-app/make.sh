# clean, build, install and monitor script.
# REM: Change IP address to that of your android phone's.
#
pebble clean
pebble build
pebble install --phone 192.168.1.2
pebble logs    --phone 192.168.1.2
