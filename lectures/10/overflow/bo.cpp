#include <stdio.h>
#include <numeric>
#include <cstdint>
#include <stdlib.h>

#define printAddr(v) printf("%3s: %p\n", #v, &v)

int main(int, char**);

void privateFunc(void)
{
	printf("This should not be called!\n");
}

void foo(int x, int y)
{
	int a,b;
	char buf[8];
	int c,d;

	*((uint32_t*)buf) = 0xdeadbeef;
	*((uint32_t*)buf+1) = 0xdeadbeef;

	a = (x | 0xf) & 0xfffffffa;
	b = (x | 0xf) & 0xfffffffb;
	c = (y | 0xf) & 0xfffffffc;
	d = (y | 0xf) & 0xfffffffd;
	printAddr(buf);
	printAddr(a);
	printAddr(b);
	printAddr(c);
	printAddr(d);
	printAddr(main);
	printAddr(foo);
	printAddr(privateFunc);
	printAddr(printf);
//	printAddr(system);

	printf("0x%x 0x%x 0x%x 0x%x\n", a, b, c, d);
	fgets(buf, 64, stdin);
	printf("%s0x%x 0x%x 0x%x 0x%x\n", buf, a, b, c, d);

}

int  main(int argc, char **argv)
{
	printf("start\n");
	foo(0x11111111,0x22222222);
	printf("end\n");
	return 0;
}

/* vim: set sw=4 sts=4 ts=4 noet : */
