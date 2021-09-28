#include <unistd.h>

struct ExpectedReads {
	int fd;
	const char *data;
	const char *dataEnd;
	ssize_t *sizes; // zero terminated, negative means -1 and set corresponsing errno
};

struct ExpectedWrites {
	int fd;
	char *data;
	char *dataEnd;
	ssize_t *sizes;
};

extern struct ExpectedReads *expectedReads;
extern struct ExpectedWrites *expectedWrites;
