CFLAGS = -Wall -Werror -g -pthread
CXXFLAGS = -Wall -Werror -std=c++17 -g -I../test -pthread
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects

## Comment these out to disable address sanitizer
CFLAGS += -fsanitize=address
CXXFLAGS += -fsanitize=address
LDFLAGS += -fsanitize=address

# Modify these if you add more source files
hello_objects := hello.o
echo_objects := echo.o

.PHONY: default all
default: echo
all: hello echo

# Only echo is tested for submissions (hello is just an example)
test: test-echo

.PHONY: test test-echo test-hello
test-echo: echo TestEcho
	./TestEcho

test-hello: hello TestHello
	./TestHello

$(hello_objects) : Makefile
hello: $(hello_objects)

$(echo_objects) : Makefile
echo: $(echo_objects)


# Tests start ./hello and ./echo and talk to them over TCP
TestHello.o TestEcho.o: ../test/TestHelpers.h ../test/TestNet.h Makefile
TestHello: testMain.o TestHello.o
TestEcho: testMain.o TestEcho.o

.PHONY: clean distclean
clean:
	rm -f *.o
distclean: clean
	rm -f hello echo TestHello TestEcho
