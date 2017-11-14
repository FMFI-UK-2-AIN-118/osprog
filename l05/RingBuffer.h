#ifndef HAVE_RINGBUFFER_H
#define HAVE_RINGBUFFER_H 1

#include <cstddef>

class RingBuffer {
private:
	/* TODO implement me! */

public:
	/**
	 * Creates a new ring buffer of size #capacity.
	 */
	RingBuffer(size_t capacity);

	/**
	 * Releases any resources used by the buffer.
	 * TODO maybe not needed, depending on implementation.
	 */
	~RingBuffer();

	/**
	 * Adds up to #size bytes of #data to the end of the buffer.
	 *
	 * @returns the actual number of bytes that were added (i.e. when
	 * available space was less than #size).
	 */
	size_t add(const char *data, size_t size);

	/**
	 * Takes up to #size bytes from the beginning of the buffer
	 * and copies them to #data.
	 *
	 * @returns the actual number of bytes copied
	 * (should be min(size, buf.size())).
	 */
	size_t take(char *data, size_t size);

	/**
	 * Remove (discard) #size bytes from the beginning of the buffer.
	 *
	 * @return actual number of bytes that were removed.
	 */
	size_t remove(size_t size);

	struct CArrayRef {
		const char *data;
		size_t size;
	};

	/**
	 * @returns a pointer to the beginning of data inside
	 * the buffer and the size of the continuos segment
	 * starting there. This can be less then #size() if
	 * the data "wraps around" in the buffer.
	 */
	CArrayRef front() const;

	struct ArrayRef {
		char *data;
		size_t size;
	};

	/**
	 * @return a pointer to a place in the buffer
	 * where new data would/can be added and
	 * the size of the continuos segment available
	 * for data there.
	 *
	 * Possible usage exmple:
	 *
	 *     auto ref = ringBuffer.next();
	 *     ssize_t bRead = read(fd, ref.data, ref.size)
	 *     if (bRead > 0)
	 *         ringBuffer.added(bRead)
	 *
	 * @note Although this method does not modify the buffer,
	 * it is not const because it only makes sense to use it
	 * in contexts where the buffer will be modified.
	 *
	 * @see added
	 */
	ArrayRef next();

	/**
	 * Enlarge the buffer to also hold the next #size bytes
	 * alredy present in the free space (presumably filled in using
	 * the information from a next() call).
	 *
	 * @see next
	 */
	void added(size_t size);

	/**
	 * @returns the size of data in the buffer.
	 */
	size_t size() const;

	/**
	 * @returns the capacity of the buffer.
	 */
	size_t capacity() const;

	/**
	 * @returns the size of the empty space in the buffer.
	 */
	size_t free() const;

	/**
	 * @returns true if the buffer is empty.
	 */
	bool isEmpty() const { return size() == 0; }

	/**
	 * @returns true if the buffer is full.
	 */
	bool isFull() const { return size() == capacity(); }
};

#endif//HAVE_RINGBUFFER_H
/* vim: set sw=4 sts=4 ts=4 noet : */
