#include "copy.h"
#include "mock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define RDFD 10
#define WRFD 11

char wBuffer[100*1024];
char rBuffer[100*1024];
static struct ExpectedWrites ew;
static struct ExpectedReads er;

void erSet(ssize_t *erSizes)
{
	er.fd = RDFD;
	er.data = rBuffer;
	er.dataEnd = rBuffer + sizeof(rBuffer);
	er.sizes = erSizes;
	expectedReads = &er;
}

void ewSet(ssize_t *ewSizes)
{
	ew.fd = WRFD;
	ew.data = wBuffer;
	ew.dataEnd = wBuffer + sizeof(wBuffer);
	ew.sizes = ewSizes;
	expectedWrites = &ew;
}

void fill(size_t n)
{
	for(size_t i = 0 ; i < n; ++i)
		rBuffer[i] = 'a' + i % 26;
	memset(rBuffer + n, '_', sizeof(rBuffer) - n);
	memset(wBuffer, '#', sizeof(wBuffer));
}

#define COMPARE(a, b) if ((a) != (b)) { \
	fprintf(stderr, "  expected: %3d\n", (b)); \
	fprintf(stderr, "    actual: %3d\n", (a)); \
	assert(a == b); }
#define COMPARE_S(a, b) if ((a) != (b)) { \
	fprintf(stderr, "  expected: %3d\n", (b)); \
	fprintf(stderr, "    actual: %3lu\n", (a)); \
	assert(a == b); }
#define COMPARE_STR(a, b, n)  if (strncmp((a),(b),(n))) { \
	fprintf(stderr, "   input: %.*s\n", (n)+2, (a)); \
	fprintf(stderr, "  output: %.*s\n", (n)+2, (b)); \
	fprintf(stderr, "    _ - input padding,  # - output padding (shoudl be exactly 2 chars)\n"); \
	assert(!strncmp(a,b,n)); }

#define RS(x...) {x}
#define WS(x...) {x}
#define TEST_OK(n, _rs, _ws, expRet) \
	fprintf(stderr, "%s\n", __FUNCTION__); \
	fill(n); \
	ssize_t rs[] = _rs; \
	ssize_t ws[] = _ws; \
	erSet(rs); \
	ewSet(ws); \
	ssize_t ret = copyFds(er.fd, ew.fd); \
	COMPARE_S(ret, expRet); \
	COMPARE_STR(rBuffer, wBuffer, n); \
	fprintf(stderr, "OK\n\n");

#define TEST_ERROR(n, _rs, _ws, expErrno) \
	fprintf(stderr, "%s\n", __FUNCTION__); \
	fill(n); \
	ssize_t rs[] = _rs; \
	ssize_t ws[] = _ws; \
	erSet(rs); \
	ewSet(ws); \
	ssize_t ret = copyFds(er.fd, ew.fd); \
	COMPARE_S(ret, -1); \
	COMPARE(errno, expErrno) \
	COMPARE_STR(rBuffer, wBuffer, n); \
	fprintf(stderr, "OK\n\n")


/**
 * Simulates that a first read would return that it read 10 bytes and the next
 * one (and all others) would return 0 (i.e. signalling an EOF.)
 * Similarly simulates that the first write would return that it wrote 10 bytes.
 *
 * The -EACCES (negative number means -1 would be returned and the errno would
 * be set to the abs value) at the end means that the EACCES error would be
 * returned if the program under test tries to write more data...
 *
 * Note: these tests assume that the code under test will try to write as much
 * as it correctly can i.e. after reading 4 bytes, it would try to write all 4
 * bytes and not do two writes of two bytes. While that's a correct behaviour,
 * it would complicate the tests to allow that ;) (pull requests welcome...)
 */
void testSimple()
{
	TEST_OK(10,
		RS(10, 0),
		WS(10, -EACCES, 0),
		10);
}

void testErrorRead()
{
	TEST_ERROR(4,
		RS(1,1,1,1, -EPERM, 0),
		WS(4, -EACCES, 0),
		EPERM);
}

void testWritePartial()
{
	TEST_OK(10,
		RS(10, 0),
		WS(1,1,1,1,1,1,1,1,1,1,-EACCES, 0),
		10);
}

void testWriteHalf()
{
	TEST_OK(10,
		RS(10, 0),
		WS(5, 5, -EACCES, 0),
		10);
}

void testWritePartialError()
{
	TEST_ERROR(10,
		RS(10,0),
		WS(5, -EPERM, 0),
		EPERM);
}

void testEintrRead()
{
	TEST_OK(10,
		RS(1, -EINTR, 1, -EINTR, -EINTR, -EINTR, 8, 0),
		WS(10, -EACCES, 0),
		10);
}

void testEintrWrite()
{
	TEST_OK(10,
		RS(10, 0),
		WS(1, -EINTR, 1, -EINTR, -EINTR, -EINTR, 8, -EACCES, 0),
		10);
}

int main(int argc, char* argv[])
{
	testSimple();
	testErrorRead();
	testWritePartial();
	testWriteHalf();
	testWritePartial();
	testEintrRead();
	testEintrWrite();
	return 0;
}
