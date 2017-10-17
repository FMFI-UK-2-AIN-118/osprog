CFLAGS = -Wall -Werror -g
CXXFLAGS = -Wall -Werror -std=c++17 -g -I../test
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects

## Comment these out to disable address sanitizer
CFLAGS += -fsanitize=address
CXXFLAGS += -fsanitize=address
LDFLAGS += -fsanitize=address

TESTS := TestCBuffer TestBuffer
RUN_TEST_DEFAULT := run-TestBuffer
RUN_TESTS := $(foreach t,$(TESTS),run-$t)

.PHONY: default all test tests
default: test
all: tests
test: $(RUN_TEST_DEFAULT)
tests: $(RUN_TESTS)

memcheck: memcheck-TestBuffer

test.o: test.cpp ../test/catch.hpp Makefile

TestCBuffer.o cbuffer.o: cbuffer.h Makefile
TestCBuffer: test.o TestCBuffer.o cbuffer.o

TestBuffer.o Buffer.o: Buffer.h Makefile
TestBuffer: test.o TestBuffer.o Buffer.o

memcheck-%: %
	valgrind --leak-check=yes ./$<

run-%: %
	@echo
	@echo "Running $<"
	./$<
	@echo "You should also run 'make memcheck-$<' to check for memory leaks"

.PHONY: clean distclean test
clean:
	rm -f *.o
distclean: clean
	rm -f $(TESTS)
