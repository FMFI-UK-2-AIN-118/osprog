CXXFLAGS = -Wall -Werror -std=c++17 -g -pthread -I../test
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're linking c++ objects

procs: procs.o

test: procs
	@echo ; echo "No real automated tests apart from building and running, sorry." ; echo
	./procs -p data/proc

clean:
	rm -f *.o
distclean: clean
	rm -f procs

.PHONY: test clean distclean
