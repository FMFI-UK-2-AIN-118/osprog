#include <stdio.h>
#include <numeric>
#include <cstdint>
#include <stdlib.h>

#define printAddr(v) printf("%13s: %p\n", #v, &v)

int main(int, char**);

void privateFunc(void)
{
	printf("This should not be called!\n");
}

void foo(int x, int y)
{
	int a,b;
	char buf[8];
	int i,j;

	*((uint32_t*)buf) = 0xdeadbeef;
	*((uint32_t*)buf+1) = 0xdeadbeef;

	a = x + 1; b = x + 2;
	i = y + 1; j = y + 2;
	printAddr(buf);
	printAddr(a); printAddr(b);
	printAddr(i); printAddr(j);
	printAddr(main); printAddr(foo);
	printAddr(privateFunc); printAddr(printf);
	printf("x 0x%x y 0x%x a 0x%x b 0x%x i 0x%x j 0x%x\n", x, y, a, b, i, j);

	printf("----\n");
//	gets(buf);
	fgets(buf, 64, stdin);
	printf("\n----\n");
	printf("%s\n", buf);
	printf("x 0x%x y 0x%x a 0x%x b 0x%x i 0x%x j 0x%x\n", x, y, a, b, i, j);

}

int  main(int argc, char **argv)
{
	printf("start\n");
	foo(0x11111111,0x22222222);
	printf("end\n");
	return 0;
}

/* vim: set sw=4 sts=4 ts=4 noet : */
