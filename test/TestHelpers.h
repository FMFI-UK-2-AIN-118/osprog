#ifndef HAVE_TESTHELPERS_H
#define HAVE_TESTHELPERS_H 1

#include <atomic>
#include <catch.hpp>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string.h>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

/**
 * A scoped helper to initialize and close a pipe.
 *
 *     Pipe p;
 *     auto ret = read(p.source, buf, len);
 */
struct Pipe {
	int source, sink;
	Pipe()
	{
		int pipefd[2];
		if (pipe(pipefd) == -1) {
			throw std::runtime_error(std::string("Can initialize a pipe: ") + strerror(errno));
		}
		source = pipefd[0];
		sink = pipefd[1];
	}

	~Pipe()
	{
		close(source);
		close(sink);
	}
};

/**
 * A scoped helper to terminate the program after a certain timeout
 * if it doesn't go out of scope.
 *
 *     using namespace std::chrono_literals;
 *     T1000 terminateAfter(10000ms);
 */
struct T1000 {
	template<class Rep, class Period>
	T1000(const std::chrono::duration<Rep, Period> &timeout, const std::string &msg = "")
		: disarmed(false)
	{
		t = std::thread([timeout, msg, this]() {
			std::unique_lock<std::mutex> lock(cv_m);
			if (!cv.wait_for(lock, timeout, [this]() { return bool(disarmed); })) {
				INFO("Timeout reached" << (msg.empty() ? "" : ": ") << msg);
				std::terminate();
			}
		});
	}

	~T1000()
	{
		disarmed = true;
		cv.notify_all();
		t.join();
	}

private:
	std::condition_variable cv;
	std::mutex cv_m;
	std::atomic<bool> disarmed;
	std::thread t;
};

/**
 * Read #count bytes from the filedescriptor as a string.
 * Note: can return a shorter string in case of errors/eof.
 */
inline std::string readBytes(int fd, size_t count)
{
	std::string str(count, '\0');
	size_t pos = 0;
	while (pos < count) {
		auto ret = read(fd, str.data() + pos, count - pos);
		if (ret <= 0) {
			// error or eof
			INFO("readBytes: " << ret);
			str.resize(pos);
			return str;
		}
		pos += ret;
	}
	return str;
}

/**
 * Read all the currently available data from a fd (pipe) as a string.
 */
inline std::string readAvailable(int fd)
{
	int bytesAvailable;
	if (ioctl(fd, FIONREAD, &bytesAvailable) == -1)
		throw std::runtime_error("can't determine number of available bytes in fd " + std::to_string(fd));
	if (bytesAvailable ==0)
		return std::string();

	return readBytes(fd, bytesAvailable);
}

#endif //HAVE_TESTHELPERS_H
