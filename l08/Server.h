#ifndef HAVE_SERVER_H
#define HAVE_SERVER_H 1

#include <stdint.h>
#include <memory>

class Client;

class Server {
public:
	/**
	 * Creates the TCP chat server.
	 */
	Server();

	~Server();

	/**
	 * Starts listening for connectiosn on #port.
	 *
	 * This should block and wait for connections
	 * (and accept them).
	 *
	 * @returns -1 on error
	 */
	int listen(uint16_t port);

	/**
	 * Adds a new client.
	 */
	void add(std::unique_ptr<Client> client);

	/**
	 * Removes a client from the collection
	 *
	 * @param client a (raw) pointer to the client to be removed.
	 *
	 * TODO: feel free to use the socket instead to identify a client.
	 */
	void remove(Client *client);

	/**
	 * Append the data specified by the arguments to each clients
	 * ring buffer.
	 */
	void appendData(const char *data, size_t len);

private:
	int sock;
};

#endif//HAVE_SERVER_H
