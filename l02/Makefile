CFLAGS = -Wall -Werror
UNAME := $(shell uname -s)
ifeq ($(UNAME), Linux)
	LDLIBS = -ldl
endif

default: copycat

copy.o: copy.c copy.h Makefile
copycat.o: copycat.c copy.h Makefile

copycat: copy.o copycat.o

mock.o: mock.c mock.h
testCopy.o: testCopy.c mock.h
testCopy: testCopy.o mock.o copy.o

test: copycat testCopy
	./testCopy
	bash smokeTest.sh
