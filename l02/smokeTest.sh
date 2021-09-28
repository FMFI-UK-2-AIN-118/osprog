#!/usr/bin/env /bin/bash
#
# Note: this not a comprehensive test but only a "smoke test"
# that just test whether it actually copies data over correclty.
# This doesn't detect bad error handling etc.

source ../test/common.sh

trap 'rm -f {random,zeroes}.{bin,in,out}' EXIT

msg "Creating test files"
dd if=/dev/zero of=zeroes.bin bs=$((1024*1024)) count=5 || die "Can't create zeroes.bin"
dd if=/dev/urandom of=random.bin bs=$((1024*1024)) count=1 || die "Can't create random.bin"

echo

testNoArgs()
{
	cp $1.bin $1.in
	msg "Testing './copycat <$1.in >$1.out'"
	./copycat <$1.in >$1.out || die "Failed: copycat <$1.in >$1.out"
	diff $1.bin $1.out || die "Failed: $1.out is different"
	rm -f $1.out
}

testNoArgs random
testNoArgs zeroes

testArgsFiles()
{
	cp $1.bin $1.in
	msg "Testing './copycat $1.in $1.out'"
	./copycat $1.in $1.out || die "Failed: copycat $1.in $1.out"
	diff $1.bin $1.out || die "Failed: $1.out is different"
	rm -f $1.out
}

testArgsFiles random
testArgsFiles zeroes


testStdinFile()
{
	cp $1.bin $1.in
	msg "Testing './copycat - $1.out <$1.in'"
	./copycat - $1.out <$1.in || die "Failed: copycat -  $1.out < $1.in"
	diff $1.bin $1.out || die "Failed: $1.out is different"
	rm -f $1.out
}

testStdinFile random
testStdinFile zeroes

testFileStdout()
{
	cp $1.bin $1.in
	msg "Testing './copycat $1.in - >$1.out'"
	./copycat $1.in - >$1.out || die "Failed: copycat $1.in - > $1.out"
	diff $1.bin $1.out || die "Failed: $1.out is different"
	rm -f $1.out
}

testFileStdout random
testFileStdout zeroes

testStdinStdout()
{
	cp $1.bin $1.in
	msg "Testing './copycat - - <$1.in >$1.out'"
	./copycat - - <$1.in >$1.out || die "Failed: copycat - - <$1.in >$1.out"
	diff $1.bin $1.out || die "Failed: $1.out is different"
	rm -f $1.out
}

testStdinStdout random
testStdinStdout zeroes

msg "Done"
