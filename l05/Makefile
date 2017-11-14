CXXFLAGS = -Wall -Werror -std=c++17 -g -I../test
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects

test: testMain
	./testMain

testMain: testMain.o TestRingBuffer.o RingBuffer.o

clean:
	rm -f *.o
distclean: clean
	rm -f testMain

.PHONY: test clean distclean
