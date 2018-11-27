#include <cstdio>
#include <cstdlib>
#include "Server.h"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "USAGE:\n");
		fprintf(stderr, "chat PORT_NUMBER\n");
		exit(EXIT_FAILURE);
	}

	Server server;
	return server.listen(atoi(argv[1]));
}
/* vim: set sw=4 sts=4 ts=4 noet : */
