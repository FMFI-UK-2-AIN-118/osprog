CFLAGS = -Wall -Werror -g -pthread
CXXFLAGS = -Wall -Werror -std=c++17 -g -I../test -pthread
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects

# Modify these if you add more source files
hello_objects := hello.o
echo_objects := echo.o

.PHONY: default all
default: echo
all: hello echo
test: echo
	@echo "No tests apart compilation for now, sorry!"

$(hello_objects) : Makefile
hello: $(hello_objects)

$(echo_objects) : Makefile
echo: $(echo_objects)


.PHONY: clean distclean
clean:
	rm -f *.o
distclean: clean
	rm -f hello echo
