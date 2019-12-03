CXXFLAGS = -Wall -Werror -std=c++17 -g -pthread -I../test
CFLAGS = -Wall -pthread
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're linking c++ objects

shell: shell.o

test: shell
	@echo ; echo "No automated tests apart from building, sorry." ; echo

clean:
	rm -f *.o
distclean: clean
	rm -f shell

.PHONY: clean distclean test
