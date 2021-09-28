#ifdef __linux__
#define _GNU_SOURCE 1
#endif
#include "mock.h"
#include <dlfcn.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;
static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;

struct ExpectedReads *expectedReads;
struct ExpectedWrites *expectedWrites;

ssize_t read(int fd, void *buf, size_t count)
{
	if (!real_read) {
		real_read = dlsym(RTLD_NEXT, "read");
	}

	if (fd != expectedReads->fd)
		return real_read(fd, buf, count);

	struct ExpectedReads *e = expectedReads;
	ssize_t toReturn = 0;
	if (*e->sizes != 0) {
		if (*e->sizes < 0) {
			toReturn = -1;
			errno = -*e->sizes;
			e->sizes++;
		} else {
			toReturn = count;
			if (*e->sizes < toReturn)
				toReturn = *e->sizes;
			if (e->dataEnd - e->data < toReturn)
				toReturn = e->dataEnd - e->data;
			memmove(buf, e->data, toReturn);
			e->data += toReturn;
			*e->sizes -= toReturn;
			if (!*e->sizes)
				e->sizes++;
		}
	}
	if (toReturn == -1) {
		fprintf(stderr, " read  fd %3d %7lu -> %7ld errno %d\n", fd, count, toReturn, errno);
	} else {
		fprintf(stderr, " read  fd %3d %7lu -> %7ld\n", fd, count, toReturn);
	}
	return toReturn;
}

ssize_t write(int fd, const void *buf, size_t count)
{
	if (!real_write) {
		real_write = dlsym(RTLD_NEXT, "write");
	}

	if (fd != expectedWrites->fd)
		return real_write(fd, buf, count);

	struct ExpectedWrites *e = expectedWrites;
	ssize_t toReturn = 0;
	if (*e->sizes != 0) {
		if (*e->sizes < 0) {
			toReturn = -1;
			errno = -*e->sizes;
			e->sizes++;
		} else {
			toReturn = count;
			if (*e->sizes < toReturn)
				toReturn = *e->sizes;
			if (e->dataEnd - e->data < toReturn) {
				toReturn = e->dataEnd - e->data;
				if (toReturn == 0) { // out of buffer space, this should not happen as we control the sizes in tests
					errno = ENOSPC;
					toReturn = -1;
				}
			}
			if (toReturn) {
				memmove(e->data, buf, toReturn);
				e->data += toReturn;
				*e->sizes -= toReturn;
				if (!*e->sizes)
					e->sizes++;
			}
		}
	}
	if (toReturn == -1) {
		fprintf(stderr, " write fd %3d %7lu -> %7ld errno %d\n", fd, count, toReturn, errno);
	} else {
		fprintf(stderr, " write fd %3d %7lu -> %7ld\n", fd, count, toReturn);
	}
	return toReturn;
}

