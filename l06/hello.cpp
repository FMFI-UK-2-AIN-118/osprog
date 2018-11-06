#include <arpa/inet.h> // inet_ntoa
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void err(const char *msg) { perror(msg); exit(EXIT_FAILURE); }

void handleClientBlocking(int fd)
{
	// Following is basically the following "pseudocode" in raw C ;)
	//     name = socket.readUntil("\n", maxSize = 60);
	//     socket.write("Hello " + name + "!\n");
	//     socket.close();

	char buffer[6 + 60 + 2] = {"Hello "};
	char* start = buffer + 6; // read new data after hello 
	char* end = start;
	char* max = start + 60;

	while (end < max) {
		ssize_t bytesRead = read(fd, end, max - end);
		if (bytesRead == -1) // Ignoring EINTR / EPIPE for now
			err("Reading from client");

		char* newlinePosition = NULL;
		if ((newlinePosition = (char*)memchr(end, '\n', bytesRead)) != NULL) {
			end = newlinePosition;
			break;
		}
		end += bytesRead;
	}
	start = buffer; // print from beginning
	// append "!\n";
	*end++ = '!';
	*end++ = '\n';
	while (start < end) {
		ssize_t bytesWritten = write(fd, start, end - start);
		if (bytesWritten == -1)
			err("Writing to client");
		start += bytesWritten;		
	}

	close(fd);
}

int main(int argc, char* argv[])
{
	int serverSock;

	if (argc < 2) {
		fprintf(stderr, "USAGE:\n");
		fprintf(stderr, "  hello PORT_NUMBER\n");
		exit(EXIT_FAILURE);
	}

	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
		err("socket");

	uint16_t portno = atoi(argv[1]);  // TODO strtol to detect errors

	struct sockaddr_in addr; // ipv4 address and port
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; // listen on any address / interface
	addr.sin_port = htons(portno); // convert port number to network byte order

	if (bind(serverSock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		err("bind");

	if (listen(serverSock, 10) == -1)
		err("listen");

	while (1) {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		// wait for new connections
		int clientSock = accept(serverSock, (struct sockaddr *) &clientAddr,
				&clientAddrSize);
		if (clientSock == -1 && (errno != EAGAIN || errno != EINTR))
			err("accept");

		if (clientSock != -1) {
			// we have a new connection
			fprintf(stderr, "New connection from %s:%d\n",
					inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			// TODO handle connection
			// The example here is blocking, a correct solution needs to
			// ensure that other connections will be accepted, even if
			// this client doesn't send anything... 
			handleClientBlocking(clientSock);
		}
	}
}

/* vim: set sw=4 sts=4 ts=4 noet : */
