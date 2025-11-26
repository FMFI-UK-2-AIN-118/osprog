#include <cstdio>
#include <unistd.h>
#define printAddr(v) printf("%3s: %p\n", #v, &v)

int main()
{
	printAddr(write);
	return 0;
}
