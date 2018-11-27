#include "Server.h"
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_ntoa

Server::Server()
	: sock(-1)
{
}

Server::~Server()
{
	if (sock != -1)
		close(sock);
}

int Server::listen(uint16_t port)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return -1;
	}

	if (::listen(sock, 10) == -1) {
		perror("listen");
		return -1;
	}

	fprintf(stderr, "Listenning on port %d\n", port);

	while (1) {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		// wait for new connctions
		int clientSock = accept(sock, (struct sockaddr *) &clientAddr,
				&clientAddrSize);
		if (clientSock == -1 && (errno != EAGAIN || errno != EINTR)) {
			perror("accept");
			return -1;
		}

		if (clientSock != -1) {
			// we have a new connection
			fprintf(stderr, "New connection from %s:%d\n",
					inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			// TODO handle client
			// add(std::make_unique<Client>(this, clientSock);
		}
	}

	return 0;
}
