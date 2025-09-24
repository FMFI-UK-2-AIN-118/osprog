#!/usr/bin/env /bin/bash

source ../test/common.sh

hit() { msg "touch $*" ; touch "$@" ; }

echo
warn " Note: this a very fragile test that doesn't test all the cases and can break easily"
warn " (for example it doesn't work when run from the network drive at school"
warn " because of timestamps.)"
echo

rm -f program.o number.o program libnumber.so

[[ -r Makefile ]] || die "No Makefile present"

msg "make: the default target should build everything"
make || die "Running 'make' failed"

[[ -x program ]] || die "program was not created (or is not executable)"
[[ -x libnumber.so ]] || die "libnumber.so was not created (or is not executable)"

hit libnumber.so
sleep 1
hit program.c
sleep 1

msg "make:  program.c and program should be re-built, no libnumber.so / number.o"
make || die "Re-running make failed"

[[ program.o -nt program.c ]] || die "program.o was not rebuilt!"
[[ program -nt program.c ]] || die "program was not rebuilt!"
[[ number.o -nt program.c ]] && die "number.o was rebuild when it should not"
[[ libnumber.so -nt program.c ]] && die "libnumber.so was rebuilt when it should not"

sleep 1
hit number.c
sleep 1

msg "make: number.o, libnumber.so and program should be rebuilt, no program.o"
make || die "Re-running make failed"

[[ number.o -nt number.c ]] || die "number.o was not rebuild!"
[[ libnumber.so -nt number.c ]] || die "libnumber.so was not rebuilt!"
[[ program -nt number.c ]] || die "program was not rebuilt!"
[[ program.o -nt number.c ]] && die "libnumber.so was rebuilt when it should not"

sleep 1
hit number.h
sleep 1

msg "make: everything should be reuilt"
make || die "Re-running make failed"

[[ number.o -nt number.h ]] || die "number.o was not rebuild!"
[[ libnumber.so -nt number.h ]] || die "libnumber.so was not rebuilt!"
[[ program.o -nt number.h ]] || die "program.o was not rebuilt!"
[[ program -nt number.h ]] || die "program was not rebuilt!"


msg "make clean: object files should be removed"
make clean || die "Running 'make clean' failed"

# TODO: this is easily circumvented as we don't really know what the
# aux files might have been named ;)
# checking generally for everything before  + targets is a bit more complicated
[[ -x program.o ]] && die "program.o was not removed by clean"
[[ -x number.o ]] && die "number.o was not removed by clean"
[[ -x program ]] || die "program was removed by clean"
[[ -x libnumber.so ]] || die "libnumber.so was removed by clean"

msg "make clean: once again ;-)"
make clean || die "Running 'make clean' twice in a row should not fail!"

msg "make: to get them back"
make || die "Running 'make' failed"

msg "make distclean: program and libnumber.so along with objects should be removed"
make distclean || die "Running 'make distclean' failed"

[[ -x program.o ]] && die "program.o was not removed by distclean"
[[ -x number.o ]] && die "number.o was not removed by distclean"
[[ -e program ]] && die "program was not removed by distclean"
[[ -e libnumber.so ]] && die "libnumber.so was not removed by distclean"

msg "make distclean: once again once again"
make distclean || die "Running 'make distclean' twice in a row should not fail!"

msg "make: building again for test"
make || die "Running 'make' failed"

msg "make test: should print 47 and finish correctly"
output=$(make test)

[[ $? == 0 ]] || die "Running 'make test' failed"

warn "Got: >>>${output}<<<"
[[ "$output" == "47" ]] || die "Wrong 'make test'  output: >>>${output}<<<"

msg "Passed"

# vim: set sw=4 sts=4 ts=4 noet :
