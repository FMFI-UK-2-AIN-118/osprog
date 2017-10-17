#include "cbuffer.h"
#include <stdlib.h>
#include <stdio.h>

struct Buffer {
	int refc;
	size_t size;
	char data[];
};


Buffer *buffer_new(size_t size)
{
	Buffer* buf = (Buffer*)malloc(size+sizeof(Buffer));
	if (buf) {
		buf->refc = 1;
		buf->size = size;
	}
	return buf;
}

void buffer_ref(Buffer *buf)
{
	buf->refc++;
}

void buffer_unref(Buffer *buf)
{
	buf->refc--;
	if (buf->refc <= 0) {
		free(buf);
	}
}

int buffer_refs(Buffer *buf)
{
	return buf->refc;
}

char * buffer_data(Buffer *buf)
{
	return buf->data;
}

size_t buffer_size(Buffer *buf)
{
	return buf->size;
}
