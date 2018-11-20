CXXFLAGS = -Wall -Werror -std=c++17 -g -I../test
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're (maybe) linking c++ objects
LDFLAGS = -pthread

test: testMain
	./testMain

Writer.o: Writer.h
Reader.o: Reader.h
TestWriter.o TestReader.o: ../test/TestHelpers.h TestWorkerThread.h
testMain: testMain.o TestReader.o Reader.o TestWriter.o Writer.o RingBuffer.o

clean:
	rm -f *.o
distclean: clean
	rm testMain

.PHONY: test clean distclean
