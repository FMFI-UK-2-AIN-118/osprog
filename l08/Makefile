CXXFLAGS = -Wall -Werror -std=c++17 -g -pthread -I../test
LDFLAGS = -pthread
LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH) # we're linking c++ objects

## Comment these out to disable address sanitizer
CXXFLAGS += -fsanitize=address
LDFLAGS += -fsanitize=address

# Modify this if you add more source files
chat_objects := chat.o Server.o Client.o Reader.o Writer.o RingBuffer.o

.PHONY: default
default: chat

chat: $(chat_objects)

.PHONY: test test-ringbuffer test-chat test-bonus
test: test-ringbuffer test-chat

test-ringbuffer: TestRingBuffer
	./TestRingBuffer

test-chat: chat TestChat
	./TestChat

# Tests for the bonus assignments (not part of `make test`)
test-bonus: chat TestChat
	./TestChat "[bonus]"

TestRingBuffer: testMain.o TestRingBuffer.o RingBuffer.o
TestChat: testMain.o TestChat.o
TestChat.o: ../test/TestHelpers.h ../test/TestNet.h


clean:
	rm -f *.o
distclean: clean
	rm -f chat TestChat TestRingBuffer

.PHONY: clean distclean
