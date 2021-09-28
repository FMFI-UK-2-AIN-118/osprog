#!/usr/bin/env /bin/bash

clNorm=$'\e[0m'
clRed=$'\e[031;1m'
clYellow=$'\e[033;1m'
clGreen=$'\e[032;1m'
die()  { echo "${clRed}ERROR${clNorm}: $*" >&2 ; exit 1 ; }
msg()  { echo  "$clGreen *$clNorm $*" ; }
err()  { echo    "$clRed *$clNorm $*" ; }
warn() { echo "$clYellow *$clNorm $*" ; }

# vim: set sw=4 sts=4 ts=4 noet :
