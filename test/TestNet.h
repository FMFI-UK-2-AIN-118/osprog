#ifndef HAVE_TESTNET_H
#define HAVE_TESTNET_H 1

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <signal.h>
#include <sys/prctl.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <utility>

#include "TestHelpers.h"

/**
 * Ask the kernel for a TCP port that is currently free.
 * (There is an unavoidable small race between this returning and the port
 * being used, but it's good enough for tests.)
 */
inline uint16_t freePort()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		throw std::runtime_error(std::string("socket: ") + strerror(errno));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;
	socklen_t len = sizeof(addr);
	if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == -1 || getsockname(fd, (sockaddr *)&addr, &len) == -1) {
		int e = errno;
		close(fd);
		throw std::runtime_error(std::string("can't find a free port: ") + strerror(e));
	}
	close(fd);
	return ntohs(addr.sin_port);
}

/**
 * A scoped TCP client socket (blocking, with a receive timeout so a
 * misbehaving server fails a test instead of hanging it).
 *
 *     ClientSocket c(port);
 *     c.sendAll("hi\n");
 *     auto reply = c.recvBytes(3);
 */
class ClientSocket {
public:
	using ms = std::chrono::milliseconds;

	/** Connect to localhost:port; throws on failure. */
	explicit ClientSocket(uint16_t port, ms recvTimeout = ms(2000))
		: fd_(socket(AF_INET, SOCK_STREAM, 0))
	{
		if (fd_ == -1)
			throw std::runtime_error(std::string("socket: ") + strerror(errno));

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		addr.sin_port = htons(port);
		if (connect(fd_, (sockaddr *)&addr, sizeof(addr)) == -1) {
			int e = errno;
			close(fd_);
			throw std::system_error(e, std::generic_category(), "connect");
		}
		setRecvTimeout(recvTimeout);
	}

	ClientSocket(ClientSocket &&o) noexcept : fd_(o.fd_) { o.fd_ = -1; }
	ClientSocket &operator=(ClientSocket &&o) noexcept
	{
		std::swap(fd_, o.fd_);
		return *this;
	}
	ClientSocket(const ClientSocket &) = delete;
	ClientSocket &operator=(const ClientSocket &) = delete;
	~ClientSocket() { if (fd_ != -1) close(fd_); }

	int fd() const { return fd_; }

	void setRecvTimeout(ms t)
	{
		timeval tv;
		tv.tv_sec = t.count() / 1000;
		tv.tv_usec = (t.count() % 1000) * 1000;
		setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	}

	/** Send everything; returns false if the connection broke. */
	bool sendAll(const std::string &data) { return sendAll(data.data(), data.size()); }
	bool sendAll(const char *data, size_t size)
	{
		size_t pos = 0;
		while (pos < size) {
			auto ret = send(fd_, data + pos, size - pos, MSG_NOSIGNAL);
			if (ret < 0 && errno == EINTR)
				continue;
			if (ret <= 0)
				return false;
			pos += ret;
		}
		return true;
	}

	/** Receive exactly #count bytes (shorter on eof / error / timeout). */
	std::string recvBytes(size_t count) { return readBytes(fd_, count); }

	/** Receive until the server closes the connection (or error / timeout). */
	std::string recvUntilEof()
	{
		std::string result;
		char buf[4096];
		ssize_t ret;
		while ((ret = read(fd_, buf, sizeof(buf))) > 0 || (ret < 0 && errno == EINTR))
			if (ret > 0)
				result.append(buf, ret);
		return result;
	}

	/** Receive until (and including) the first newline (or eof / timeout). */
	std::string recvLine()
	{
		std::string result;
		char c;
		while (read(fd_, &c, 1) == 1) {
			result += c;
			if (c == '\n')
				break;
		}
		return result;
	}

	/** Signal "no more data from me", but keep reading. */
	void shutdownWrite() { shutdown(fd_, SHUT_WR); }

private:
	int fd_;
};

/**
 * A scoped helper that starts a server program (`./program PORT`) and kills
 * it when it goes out of scope.
 *
 *     ServerProcess server("./echo");
 *     ClientSocket c = server.connect();
 */
class ServerProcess {
public:
	explicit ServerProcess(const std::string &program)
		: program_(program), port_(freePort())
	{
		pid_ = fork();
		if (pid_ == -1)
			throw std::runtime_error(std::string("fork: ") + strerror(errno));
		if (pid_ == 0) {
			prctl(PR_SET_PDEATHSIG, SIGKILL); // don't outlive the test program
			std::string portStr = std::to_string(port_);
			execl(program.c_str(), program.c_str(), portStr.c_str(), (char *)nullptr);
			perror(("exec " + program).c_str());
			_exit(127);
		}
	}

	ServerProcess(const ServerProcess &) = delete;
	ServerProcess &operator=(const ServerProcess &) = delete;

	~ServerProcess()
	{
		if (running()) {
			kill(pid_, SIGTERM);
			waitpid(pid_, nullptr, 0);
		} else if (pid_ != 0) {
			// a server should run until we kill it
			FAIL_CHECK(program_ << " terminated unexpectedly: " << describe(status_));
		}
	}

	uint16_t port() const { return port_; }

	/** True as long as the server process has not terminated. */
	bool running()
	{
		if (pid_ <= 0)
			return false;
		if (waitpid(pid_, &status_, WNOHANG) == 0)
			return true;
		pid_ = -1;
		return false;
	}

	/**
	 * Connect to the server. Retries for a while, since the server might
	 * not have started listening yet.
	 */
	ClientSocket connect(std::chrono::milliseconds timeout = std::chrono::milliseconds(3000))
	{
		auto deadline = std::chrono::steady_clock::now() + timeout;
		while (true) {
			try {
				return ClientSocket(port_);
			} catch (const std::system_error &e) {
				if (!running())
					throw std::runtime_error(program_ + " terminated before accepting connections");
				if (e.code().value() != ECONNREFUSED || std::chrono::steady_clock::now() > deadline)
					throw;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

private:
	static std::string describe(int status)
	{
		if (WIFSIGNALED(status))
			return std::string("killed by signal ") + std::to_string(WTERMSIG(status)) + " (" + strsignal(WTERMSIG(status)) + ")";
		return "exited with status " + std::to_string(WEXITSTATUS(status));
	}

	std::string program_;
	uint16_t port_;
	pid_t pid_;
	int status_ = 0;
};

#endif //HAVE_TESTNET_H
