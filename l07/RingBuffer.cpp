#include "RingBuffer.h"

RingBuffer::RingBuffer(size_t capacity)
{
}

RingBuffer::~RingBuffer()
{
}

size_t RingBuffer::add(const char *data, size_t size)
{
	return 0;
}


size_t RingBuffer::take(char *data, size_t size)
{
	return 0;
}


RingBuffer::CArrayRef RingBuffer::front() const
{
	return CArrayRef{0,0};
}

size_t RingBuffer::remove(size_t size)
{
	return 0;
}

RingBuffer::ArrayRef RingBuffer::next()
{
	return ArrayRef{0,0};
}

void RingBuffer::added(size_t size)
{
}

size_t RingBuffer::size() const
{
	return 0;
}


size_t RingBuffer::capacity() const
{
	return 0;
}


size_t RingBuffer::free() const
{
	return 0;
}

/* vim: set sw=4 sts=4 ts=4 noet : */
