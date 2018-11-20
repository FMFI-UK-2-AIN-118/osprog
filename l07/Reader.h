#ifndef HAVE_READER_H
#define HAVE_READER_H 1

#include <functional>
#include <vector>
#include <cstddef>

class Reader {
public:
	typedef std::function<void(const char *, size_t)> CallbackFunc;
	/**
	 * Creates a new reader.
	 *
	 * @param fd the socket to read from.
	 * @param callback a function to call when data is read
	 */
	Reader(int fd, CallbackFunc &&callback, size_t bufSize = 1024*1024);

	/**
	 * Reads data from #fd and adds it to ringbuffers of all clients.
	 */
	void run();

	/**
	 * Terminate the run method.
	 * This will be called from another thread.
	 * After it is called, the run method should finish as soon
	 * as possible and should not access any class resources anymore
	 * (i.e. the ringbuffer).
	 */
	void quit();

private:
	CallbackFunc callback;
	/* TODO; implement me! */
};

#endif//HAVE_READER_H
