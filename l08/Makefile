CXXFLAGS = -Wall -Werror -std=c++17 -g -pthread -I../test
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're linking c++ objects

chat: chat.o Server.o Client.o Reader.o Writer.o RingBuffer.o

test: chat
	@echo ; echo "No automated tests apart from building, sorry." ; echo

clean:
	rm -f *.o
distclean: clean
	rm -f chat

.PHONY: clean distclean test
