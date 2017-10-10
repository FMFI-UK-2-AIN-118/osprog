#!/usr/bin/env /bin/bash
PROG=./pipewatch

source ../test/common.sh

waitForFifo() {
	local i=0
	while [[ ! -p "$1" && $i -lt 10 ]] ; do
		let i++
		sleep 0.1
	done
	[[ -p "$1" ]]
}

rm -f a b c out

mkfifo c

$PROG a b c > out & PID=$!

msg "Waiting for fifos"
waitForFifo a || die "FIFO a not created!"
waitForFifo b || die "FIFO b not created!"

msg "hello > a"
echo hello > a

msg "byebye > b"
echo byebye > b

let aLot=4*1024*1024
msg "$aLot zeros > a"
head -c $aLot /dev/zero > a

msg "quit > c"
{ echo -n qu ; sleep 1 ; echo it ; } > c

msg "Waiting for program to finish"
wait $PID || die "Program not finished correctly"

cat >expected <<-EOF
	a: 6 bytes
	b: 7 bytes
	a: $((4*1024*1024)) bytes
	c: 2 bytes
EOF

diff out expected || die "Wrong output"

msg "Note: this script does only very basic testing!"
