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

msg "Testing invalid argument counts"
./copycat arg1 2>/dev/null && die "Failed: copycat should fail with 1 argument"
./copycat arg1 arg2 arg3 2>/dev/null && die "Failed: copycat should fail with more than 2 arguments"

msg "Testing non-existent input file"
./copycat non_existent_file_xyz.txt out.txt 2>/dev/null && die "Failed: should fail on missing input"

msg "Done"
