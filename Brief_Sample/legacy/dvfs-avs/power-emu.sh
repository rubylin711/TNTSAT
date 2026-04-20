#!/bin/sh

# how many cores, you should run how many app
# more app not any helpful, contrarily, schedule will lead power dissipation down
power-emu 5000000 > /dev/null &
power-emu 5000000 > /dev/null
