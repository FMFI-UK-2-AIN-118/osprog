CFLAGS += -Wall -g -pthread
CXXFLAGS += -Wall -g -std=c++17 -pthread
LDFLAGS += -pthread

SOURCES := $(wildcard *.c)
PROGRAMS := $(patsubst %.c,%,$(SOURCES))

all: $(PROGRAMS)

memcheck: $(patsubst %,memcheck-%,$(PROGRAMS))

memcheck-%: %
	valgrind --leak-check=yes --error-exitcode=47 ./$*

helgrind:
	valgrind --tool=helgrind --error-exitcode=47 ./2_pthreads_bad

clean:
	rm -f $(PROGRAMS)
