CFLAGS = -Wall -Werror
CXXFLAGS = -Wall -Werror -std=c++17
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects

SOURCES := $(wildcard *.c) $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))

default: pipewatch

$(OBJECTS): Makefile
pipewatch: $(OBJECTS)

.PHONY: clean distclean test
clean:
	rm -f $(OBJECTS)
distclean: clean
	rm -f pipewatch
test: pipewatch
	./pipeTest.sh
