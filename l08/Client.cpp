#include "Client.h"
#include "Server.h"

Client::Client(Server *server, int fd)
	: m_server(server)
{
}

void Client::start()
{
	// TODO
}
