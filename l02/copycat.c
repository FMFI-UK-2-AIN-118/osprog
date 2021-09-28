#include "copy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char* argv[])
{
	int infd = 0; // standard input
	int outfd = 1; // standard output

	if (argc > 1) {
		if (argc != 3) {
			fprintf(stderr, "Error: copycat needs either no or exactly two arguments!\n");
			// This is basically the same as `return EXIT_FAILURE` here,
			// since we are in main.
			exit(EXIT_FAILURE);
		}

		if (strcmp("-", argv[1])) {
			// TODO implement me: open input file for reading
			// infd = ...
			assert(0);
		}
		if (strcmp("-", argv[2])) {
			// TODO implement me: open output file for writing
			// outfd = ...
			assert(0);
		}
	}

	if (copyFds(infd, outfd) == -1) {
		perror("Error copying");
		return EXIT_FAILURE;
	}

	// TODO close files (if not stdin/stdout)
	return EXIT_SUCCESS;
}
