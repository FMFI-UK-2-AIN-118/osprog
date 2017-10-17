#ifndef HAVE__BUFFER_H
#define HAVE__BUFFER_H 1

#include <cstddef>

class Buffer {
protected:
	struct Data;
	Data *p_d;

public:
	/**
	 * Creates a new buffer of the given size.
	 * Note that the content will be "uninitialized".
	 *
	 * @param size size of the buffer
	 */
	Buffer(std::size_t size = 0);

	/**
	 * Copy constructor that should just create a new reference.
	 */
	Buffer(const Buffer &other);

	/**
	 * Destructor that should decrease the reference count
	 * and free the resources if it reaches zero.
	 */
	~Buffer();

	/**
	 * Assignment operator.
	 * Makes this a reference to other.
	 *
	 * Note that the argument is not a reference, thus it will already be
	 * copy-constructed from the actual other object by the copy constructor
	 * (which is what we want), and when it goes out of scope, it's destructor
	 * will be called (which isn't exactly what we want, though we would like
	 * do de-ref "this")
	 */
	Buffer& operator=(Buffer other);

	/**
	 * Return a (const) pointer to the actual data
	 * that can be used only for "reading".
	 */
	const char* cdata() const;

	/**
	 * Return a pointer to the actual data
	 * that can be used to modify it.
	 */
	char * data();

	/**
	 * Return the size of the buffer.
	 */
	std::size_t size() const;

	/**
	 * Change the size of the buffer.
	 *
	 * If newSize is greater than current size, the new
	 * bytes at the end are "uninitialized".
	 *
	 * If newSize is greater than the current capacity, reallocation
	 * will occur.
	 *
	 * @param newSize new size of the buffer
	 *
	 * @throws std::bad_alloc if allocating the new memory wasn't successfull
	 */
	void resize(std::size_t newSize);

	/**
	 * Return the current capacity of the buffer.
	 *
	 * Capacity is the size that this buffer can grow to
	 * without actually needing to re-allocate memory.
	 */
	std::size_t capacity() const;

	/**
	 * Allocate memory for at least newSize bytes.
	 * The real size of the buffer (used data) doesn't change
	 * but calling this method in advance makes sure that small
	 * "appends" up to this size won't make re-allocations.
	 *
	 * This can't lower the capacity below actual size.
	 *
	 * @param newSize the new capacity
	 */
	void reserve(std::size_t newSize);

	/**
	 * "Release" memory not needed to store the data
	 * (i.e. set capacity to size).
	 */
	void squeeze();

	/**
	 * Add new data to the end of the buffer.
	 *
	 * @param data pointer to the data to add
	 * @param len length of the data.
	 *
	 * If len == -1 then data is assumed to be a null-terminated string
	 * (as if strlen was used to determine the actual length).
	 */
	void append(const char *data, int len = -1);

	/**
	 * Remove size bytes from the end of the buffer.
	 *
	 * @param size number of bytes to remove
	 */
	void chop(std::size_t size);

	/**
	 * Remove size bytes from the buffer starting at pos.
	 *
	 * If pos is out of range, nothing happens.
	 *
	 * @param pos position to start removing from
	 * @param size number of bytes to remove
	 */
	void remove(std::size_t pos, std::size_t size);

	/**
	 * Return the number of references.
	 * This breaks encapsulation and is here only to make
	 * testing easier.
	 */
	int refs() const;
};

#endif //HAVE__BUFFER_H
