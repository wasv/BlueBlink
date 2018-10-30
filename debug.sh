#!/bin/sh
d=$(dirname $0)
arm-none-eabi-gdb -iex "target remote | openocd -c \"gdb_port pipe; log_output openocd.log\" -f $d/debug/bluepill-debug.cfg" $1
