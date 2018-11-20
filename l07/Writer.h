#ifndef HAVE_WRITER_H
#define HAVE_WRITER_H 1

#include "RingBuffer.h"
#include <cstddef>
#include <pthread.h>

/**
 * A class that accepts data into a RingBuffer
 * (from other threads) and writes them into a fd.
 * Blocks on a wait condition if no data are in
 * the buffer.
 */
class Writer {
public:
	/**
	 * Creates a new writer for the given #fd.
	 * Can be given an optional buffer size.
	 */
	Writer(int fd, size_t bufSize = 1024*1024);

	/**
	 * Releases any allocated resources.
	 * TODO maybe not needed, depending on implementation.
	 */
	~Writer();

	/**
	 * Adds data to the ringbuffer for this writer.
	 * This method can be called from any thread.
	 * Adding data should wake any thread waiting
	 * inside the #run method for new data.
	 *
	 * @param data pointer to the data to add
	 * @param size number of bytes to add
	 * @returns number of bytes that were actually added
	 * (can be lower if there wasn't enough space
	 * in the buffer).
	 */
	size_t add(const char *data, size_t size);

	/**
	 * Write data (blocking) that is added to the ringbuffer
	 * into the fd. If there is no data in the ringbuffer,
	 * then wait until notified by the #add method.
	 *
	 * Finishes when an error happens.
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
	/* TODO; implement me! */
};

#endif//HAVE_WRITER_H
