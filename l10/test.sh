#!/usr/bin/env /bin/bash

source ../test/common.sh

[[ -r exploit.png ]] || die "No exploit.png present!"

echo "Running ./pnginfo exploit.png"
trap 'rm -f output' EXIT

./pnginfo exploit.png >output 2>&1

echo -e "Output:\n-----"
cat output
echo "-----"

if grep -q "Well done" output ; then
	msg "OK"
	exit 0
else
	err "Fail"
	exit 1
fi

# vim: set sw=4 sts=4 ts=4 noet :
