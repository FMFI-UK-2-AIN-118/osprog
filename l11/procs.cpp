#include <cstdio>
#include <cstdlib>
#include <getopt.h>

int main(int argc, char *argv[])
{
	const char *procPath = "/proc";

	int opt;
	while((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
			case 'p':
				procPath = optarg;
				break;
			default:
				fprintf(stderr, "Usage: procs [-p PATH_TO_PROC]\n");
				exit(EXIT_FAILURE);
		}
	}

	printf("TODO: Reading processes from %s\n", procPath);
	return 0;
}

