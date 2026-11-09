/**
 * Tests provided by claude.
 * All hail our new AI overlords.
 */
#include <catch.hpp>
#include <TestHelpers.h>
#include <TestNet.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <dirent.h>
#include <functional>
#include <string>
#include <thread>
#include <vector>

/*
 * Black-box tests of the chat server: they start `./chat PORT` as a separate
 * process and talk to it over TCP, so they do not depend on how the
 * Client/Server/Reader/Writer classes are structured.
 *
 * Tags:
 *   [big]    -- transfers a lot of data (skipped for the slow valgrind runs)
 *   [.bonus] -- bonus assignments, not run by default: `./TestChat "[bonus]"`
 */

using namespace std::chrono_literals;

namespace {

using ms = std::chrono::milliseconds;

/** A timeout stretched by TEST_SLOWDOWN (for running under valgrind). */
ms slow(int millis) { return ms(millis * testSlowdown()); }

/** Deterministic, non-repeating-ish data of the given size. */
std::string testData(size_t size, unsigned seed = 1)
{
	std::string data(size, '\0');
	uint32_t x = seed;
	for (auto &c : data) {
		x = x * 1664525u + 1013904223u; // LCG
		c = char(x >> 24);
	}
	return data;
}

/** A thread that is joined (instead of terminating the test program) if a test fails. */
struct JoiningThread : std::thread {
	using std::thread::thread;
	JoiningThread(JoiningThread &&) = default;
	~JoiningThread() { if (joinable()) join(); }
};

/**
 * Sends data from a client in the background (so that the test can read the
 * replies at the same time). When it goes out of scope, it makes sure the
 * sending thread finishes even if the test failed half way (i.e. the server
 * stopped reading).
 */
class Sender {
public:
	Sender(ClientSocket &client, std::string data)
		: client(client), data(std::move(data))
		, thread([this]() { this->client.sendAll(this->data); })
	{}
	~Sender()
	{
		if (thread.joinable()) {
			shutdown(client.fd(), SHUT_RDWR);
			thread.join();
		}
	}
	void join() { thread.join(); }
private:
	ClientSocket &client;
	std::string data;
	JoiningThread thread;
};

/** The server to test (`make test-helgrind` runs it under valgrind). */
class Chat : public ServerProcess {
public:
	Chat() : ServerProcess(program()) {}
private:
	static std::string program()
	{
		const char *env = getenv("CHAT_BINARY");
		return env ? env : "./chat";
	}
};

/**
 * Clients connected to a server.
 *
 * Connecting only means that the kernel has queued the connection, the server
 * might not have accepted it (and started to broadcast to it) yet. #join waits
 * until the client is really part of the chat: it sends a marker byte and
 * waits until it comes back to all the clients (so there is nothing
 * unexpected left in any of the streams afterwards).
 */
struct Room {
	explicit Room(ServerProcess &server) : server(server) {}

	ClientSocket &join()
	{
		clients.push_back(server.connect());
		REQUIRE(clients.back().sendAll("\x01"));
		for (auto &c : clients)
			REQUIRE(c.recvBytes(1) == "\x01");
		return clients.back();
	}

	void join(size_t count)
	{
		while (count--)
			join();
	}

	ClientSocket &operator[](size_t i) { return clients[i]; }
	size_t size() const { return clients.size(); }

	ServerProcess &server;
	std::deque<ClientSocket> clients; // references stay valid when clients are added
};

/** Split a stream of fixed-sized lines and sort them (to ignore the order). */
std::vector<std::string> sortedLines(const std::string &data, size_t lineSize)
{
	std::vector<std::string> lines;
	for (size_t pos = 0; pos + lineSize <= data.size(); pos += lineSize)
		lines.push_back(data.substr(pos, lineSize));
	std::sort(lines.begin(), lines.end());
	return lines;
}

/** Fixed size (10 bytes) message from the client number #i. */
std::string line(size_t i)
{
	std::string s = "client " + std::to_string(i % 100);
	s.resize(9, ' ');
	return s + "\n";
}

/* Resources used by a process, from /proc */
struct Resources {
	size_t threads, fds;
	bool operator==(const Resources &o) const { return threads == o.threads && fds == o.fds; }
	bool operator<=(const Resources &o) const { return threads <= o.threads && fds <= o.fds; }
};

std::ostream &operator<<(std::ostream &os, const Resources &r)
{
	return os << r.threads << " threads, " << r.fds << " open file descriptors";
}

size_t countEntries(const std::string &dir)
{
	size_t count = 0;
	if (DIR *d = opendir(dir.c_str())) {
		while (auto *e = readdir(d))
			if (e->d_name[0] != '.')
				count++;
		closedir(d);
	}
	return count;
}

Resources resourcesOf(pid_t pid)
{
	auto proc = "/proc/" + std::to_string(pid);
	return {countEntries(proc + "/task"), countEntries(proc + "/fd")};
}

/** Poll until #pred holds (or the timeout expires); returns the last value of #pred. */
bool waitUntil(const std::function<bool()> &pred, ms timeout = slow(5000))
{
	auto deadline = std::chrono::steady_clock::now() + timeout;
	while (!pred()) {
		if (std::chrono::steady_clock::now() > deadline)
			return false;
		std::this_thread::sleep_for(5ms);
	}
	return true;
}

/**
 * Resources of an idle server (after the connection used to wait for it to
 * start listening is gone and everything has settled).
 */
Resources idleResources(ServerProcess &server)
{
	{
		Room room(server);
		room.join();
	}
	auto prev = resourcesOf(server.pid());
	int same = 0;
	auto deadline = std::chrono::steady_clock::now() + slow(5000);
	while (same < 10 && std::chrono::steady_clock::now() < deadline) {
		std::this_thread::sleep_for(10ms);
		auto cur = resourcesOf(server.pid());
		same = (cur == prev) ? same + 1 : 0;
		prev = cur;
	}
	return prev;
}

}

SCENARIO("Chat server", "[chat]") {
	T1000 terminateAfter(slow(20000), "chat test took too long");
	Chat server;
	Room room(server);

	GIVEN("a single client") {
		auto &c = room.join();

		WHEN("it sends a line") {
			REQUIRE(c.sendAll("hello\n"));
			THEN("it gets it back (the sender is also a recipient)") {
				REQUIRE(c.recvBytes(6) == "hello\n");
			}
		}

		WHEN("it sends several lines one by one") {
			THEN("it gets each one back, in order") {
				for (int i = 0; i < 10; i++) {
					std::string msg = "line " + std::to_string(i) + "\n";
					REQUIRE(c.sendAll(msg));
					REQUIRE(c.recvBytes(msg.size()) == msg);
				}
			}
		}

		WHEN("it sends data without a newline") {
			REQUIRE(c.sendAll("abc"));
			THEN("it gets it back anyway") {
				REQUIRE(c.recvBytes(3) == "abc");
			}
		}

		WHEN("it sends binary data including zero bytes") {
			std::string data("\0\1\2\0\xff\xfe\0", 7);
			REQUIRE(c.sendAll(data));
			THEN("it gets exactly the same bytes back") {
				REQUIRE(c.recvBytes(data.size()) == data);
			}
		}

		WHEN("it sends a lot of data at once") {
			auto data = testData(1024 * 1024);
			Sender sender(c, data);
			auto received = c.recvBytes(data.size());
			THEN("it gets all of it back, unchanged") {
				REQUIRE(received.size() == data.size());
				REQUIRE(received == data);
			}
		}
	}

	GIVEN("several clients") {
		room.join(5);

		WHEN("one of them sends a line") {
			REQUIRE(room[2].sendAll("hello everyone\n"));
			THEN("all of them (including the sender) receive it") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					REQUIRE(room[i].recvBytes(15) == "hello everyone\n");
				}
			}
		}

		WHEN("one of them sends binary data") {
			auto data = testData(100 * 1024, 3);
			REQUIRE(room[4].sendAll(data));
			THEN("all of them receive exactly the same bytes") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					REQUIRE(room[i].recvBytes(data.size()) == data);
				}
			}
		}

		WHEN("one of them sends several lines") {
			for (int i = 0; i < 20; i++)
				REQUIRE(room[1].sendAll("message " + std::to_string(100 + i) + "\n"));
			THEN("all of them receive them in the same order") {
				std::string expected;
				for (int i = 0; i < 20; i++)
					expected += "message " + std::to_string(100 + i) + "\n";
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					REQUIRE(room[i].recvBytes(expected.size()) == expected);
				}
			}
		}

		WHEN("all of them send a line") {
			std::vector<std::string> expected;
			for (size_t i = 0; i < room.size(); i++) {
				REQUIRE(room[i].sendAll(line(i)));
				expected.push_back(line(i));
			}
			THEN("all of them receive all the lines") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					auto received = room[i].recvBytes(10 * room.size());
					REQUIRE(sortedLines(received, 10) == expected);
				}
			}
		}

		WHEN("another client joins") {
			room.join();
			REQUIRE(room[0].sendAll("welcome\n"));
			THEN("it receives only what is sent after it joined") {
				REQUIRE(room[5].recvBytes(8) == "welcome\n");
			}
		}
	}

	GIVEN("a client that was there before") {
		auto &old = room.join();
		REQUIRE(old.sendAll("old news\n"));
		REQUIRE(old.recvBytes(9) == "old news\n");

		WHEN("a new client joins") {
			auto &fresh = room.join();
			REQUIRE(old.sendAll("new news\n"));
			THEN("the new client does not get the data sent before it joined") {
				REQUIRE(fresh.recvBytes(9) == "new news\n");
			}
		}
	}

	GIVEN("a lot of clients connected at the same time") {
		const size_t count = 50;
		// connect them all first (more than the listen backlog)
		for (size_t i = 0; i < count - 1; i++)
			room.clients.push_back(server.connect());
		// The server registers the clients in the order they connected, so
		// when the last one is part of the chat, all of them are.
		room.join();

		WHEN("they all send a line at the same time") {
			std::vector<JoiningThread> senders;
			for (size_t i = 0; i < count; i++)
				senders.emplace_back([&room, i]() { room[i].sendAll(line(i)); });
			for (auto &t : senders)
				t.join();

			THEN("each of them receives all the lines") {
				std::vector<std::string> expected;
				for (size_t i = 0; i < count; i++)
					expected.push_back(line(i));
				std::sort(expected.begin(), expected.end());

				for (size_t i = 0; i < count; i++) {
					INFO("client " << i);
					auto received = room[i].recvBytes(10 * count);
					REQUIRE(sortedLines(received, 10) == expected);
				}
			}
		}
	}

	GIVEN("several clients sending a lot of data at the same time") {
		const size_t count = 4, size = 256 * 1024;
		room.join(count);

		WHEN("they all send") {
			// everybody sends its "own" byte, so we can check that nothing was lost
			std::vector<JoiningThread> senders;
			for (size_t i = 0; i < count; i++)
				senders.emplace_back([&room, i]() {
					room[i].sendAll(std::string(size, char('a' + i)));
				});

			THEN("each of them receives everything") {
				for (size_t i = 0; i < count; i++) {
					INFO("client " << i);
					auto received = room[i].recvBytes(count * size);
					REQUIRE(received.size() == count * size);
					for (size_t sender = 0; sender < count; sender++)
						CHECK(std::count(received.begin(), received.end(), char('a' + sender)) == size);
				}
			}

			for (auto &t : senders)
				t.join();
		}
	}
}

SCENARIO("Chat server with clients leaving", "[chat][leaving]") {
	T1000 terminateAfter(slow(30000), "chat test took too long");
	Chat server;
	Room room(server);

	GIVEN("several clients") {
		room.join(4);

		WHEN("one of them disconnects") {
			room.clients.erase(room.clients.begin() + 1);
			std::this_thread::sleep_for(slow(50));

			THEN("the others still work") {
				for (int round = 0; round < 3; round++) {
					REQUIRE(room[0].sendAll("still here\n"));
					for (size_t i = 0; i < room.size(); i++) {
						INFO("client " << i << ", round " << round);
						REQUIRE(room[i].recvBytes(11) == "still here\n");
					}
				}
			}
		}

		WHEN("one of them disconnects abruptly (connection reset)") {
			room.clients[1].reset();
			room.clients.erase(room.clients.begin() + 1);
			std::this_thread::sleep_for(slow(50));

			THEN("the others still work") {
				for (int round = 0; round < 3; round++) {
					REQUIRE(room[0].sendAll("still here\n"));
					for (size_t i = 0; i < room.size(); i++) {
						INFO("client " << i << ", round " << round);
						REQUIRE(room[i].recvBytes(11) == "still here\n");
					}
				}
			}
		}

		WHEN("one of them sends a message and disconnects immediately") {
			REQUIRE(room[3].sendAll("goodbye\n"));
			room.clients.pop_back();

			THEN("the others still receive the message") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					REQUIRE(room[i].recvBytes(8) == "goodbye\n");
				}
			}
		}

		WHEN("one of them closes only its sending side") {
			room[3].shutdownWrite();
			std::this_thread::sleep_for(slow(50));

			THEN("the others still work") {
				REQUIRE(room[0].sendAll("anyone?\n"));
				for (size_t i = 0; i < 3; i++) {
					INFO("client " << i);
					REQUIRE(room[i].recvBytes(8) == "anyone?\n");
				}
			}
		}

		WHEN("one of them disconnects while others are sending a lot of data") {
			// Writing to the disconnected client fails (EPIPE/ECONNRESET); the
			// server must survive that (i.e. not die of SIGPIPE).
			auto data = testData(1024 * 1024, 7);
			Sender sender(room[0], data);
			room.clients[3].reset();
			room.clients.pop_back();

			THEN("the others get all the data") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					auto received = room[i].recvBytes(data.size());
					REQUIRE(received.size() == data.size());
					REQUIRE(received == data);
				}
			}
		}

		WHEN("all of them disconnect") {
			room.clients.clear();
			std::this_thread::sleep_for(slow(50));

			THEN("a new client can still join and chat") {
				auto &c = room.join();
				REQUIRE(c.sendAll("anybody here?\n"));
				REQUIRE(c.recvBytes(14) == "anybody here?\n");
			}
		}
	}

	GIVEN("clients which keep leaving while another one sends") {
		room.join(8);
		auto data = testData(1024 * 1024, 11);

		WHEN("the others leave at random times") {
			Sender sender(room[0], data);
			for (size_t i = 7; i >= 3; i--) {
				std::this_thread::sleep_for(slow(5));
				if (i % 2)
					room.clients[i].reset();
				room.clients.pop_back();
			}

			THEN("the remaining ones get all the data") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					auto received = room[i].recvBytes(data.size());
					REQUIRE(received.size() == data.size());
					REQUIRE(received == data);
				}
			}
		}
	}
}

SCENARIO("Chat server releases the resources of disconnected clients", "[chat][leaving][resources]") {
	T1000 terminateAfter(slow(30000), "chat test took too long");
	Chat server;
	auto idle = idleResources(server);
	INFO("idle server: " << idle);

	GIVEN("a client that connected and then disconnected") {
		{
			Room room(server);
			room.join(2);
			// the server needs something (at least the sockets) for them
			CHECK(!(resourcesOf(server.pid()) <= idle));
		}

		THEN("the server frees its threads and file descriptors") {
			Resources now;
			bool ok = waitUntil([&]() { return (now = resourcesOf(server.pid())) <= idle; });
			INFO("now: " << now);
			REQUIRE(ok);
		}
	}

	GIVEN("a lot of clients that came and went") {
		for (int batch = 0; batch < 10; batch++) {
			Room room(server);
			room.join(10);
			if (batch % 2) {
				for (auto &c : room.clients)
					c.reset();
			}
			// closes the connections (the already reset ones are no-ops)
		}

		THEN("the server frees all of their threads and file descriptors") {
			Resources now;
			bool ok = waitUntil([&]() { return (now = resourcesOf(server.pid())) <= idle; });
			INFO("now: " << now);
			REQUIRE(ok);
		}

		THEN("it still works") {
			Room room(server);
			auto &c = room.join();
			REQUIRE(c.sendAll("alive\n"));
			REQUIRE(c.recvBytes(6) == "alive\n");
		}
	}

	GIVEN("clients which join and leave while others are chatting") {
		Room room(server);
		room.join(2);
		const Resources withTwo = resourcesOf(server.pid());

		for (int i = 0; i < 20; i++) {
			{
				Room visitor(server);
				visitor.join();
				// the permanent members get the marker byte of the visitor too
				REQUIRE(room[0].recvBytes(1) == "\x01");
				REQUIRE(room[1].recvBytes(1) == "\x01");
				REQUIRE(room[0].sendAll("hi\n"));
				REQUIRE(visitor[0].recvBytes(3) == "hi\n");
				REQUIRE(room[0].recvBytes(3) == "hi\n");
				REQUIRE(room[1].recvBytes(3) == "hi\n");
			}
		}

		THEN("only the resources for the remaining clients are in use") {
			// The two remaining clients need no more than they did before.
			Resources now;
			bool ok = waitUntil([&]() { return (now = resourcesOf(server.pid())) <= withTwo; });
			INFO("now: " << now << ", expected at most: " << withTwo);
			REQUIRE(ok);
			room.clients.clear();
			ok = waitUntil([&]() { return (now = resourcesOf(server.pid())) <= idle; });
			INFO("now: " << now);
			REQUIRE(ok);
		}
	}
}

SCENARIO("Chat server with slow clients", "[chat][slow][big]") {
	T1000 terminateAfter(slow(60000), "chat test took too long");
	Chat server;
	Room room(server);

	GIVEN("a client that does not read anything and an active one") {
		auto &lazy = room.join();
		auto &active = room.join();
		lazy.setRecvTimeout(slow(500));
		active.setRecvTimeout(slow(5000));

		// Much more than all the buffers (2MB in the server + whatever the
		// kernel buffers on both ends) can hold.
		const size_t chunk = 64 * 1024, total = 32 * 1024 * 1024;
		auto data = testData(total, 5);

		WHEN("the active one sends and reads a lot of data") {
			// Sending and reading in lockstep makes sure the server never
			// has to drop anything for the active client.
			bool ok = true;
			for (size_t pos = 0; ok && pos < total; pos += chunk) {
				ok = active.sendAll(data.data() + pos, chunk);
				auto received = active.recvBytes(chunk);
				if (received != data.substr(pos, chunk)) {
					ok = false;
					FAIL_CHECK("wrong data received at offset " << pos << " (got " << received.size() << " bytes)");
				}
			}
			REQUIRE(ok);

			THEN("the lazy client got the beginning, but data that did not fit was dropped") {
				auto received = lazy.recvUntilEof(); // until the timeout
				INFO("lazy client received " << received.size() << " bytes");
				// 2MB buffer in the server, plus kernel buffers
				REQUIRE(received.size() >= 1024 * 1024);
				REQUIRE(received.size() < total);
				REQUIRE(received.substr(0, 1024 * 1024) == data.substr(0, 1024 * 1024));

				AND_THEN("it takes part in the chat again") {
					REQUIRE(active.sendAll("welcome back\n"));
					REQUIRE(active.recvBytes(13) == "welcome back\n");
					REQUIRE(lazy.recvBytes(13) == "welcome back\n");
				}
			}
		}
	}

	GIVEN("a client that does not read and an active one sending a bit") {
		auto &lazy = room.join();
		auto &active = room.join();

		WHEN("the server has to buffer the data for the lazy one") {
			auto data = testData(1024 * 1024, 9);
			REQUIRE(active.sendAll(data));
			REQUIRE(active.recvBytes(data.size()) == data);

			THEN("the lazy one still gets everything once it starts reading") {
				auto received = lazy.recvBytes(data.size());
				REQUIRE(received.size() == data.size());
				REQUIRE(received == data);
			}
		}
	}

	GIVEN("a lazy client that goes away without reading anything") {
		auto &lazy = room.join();
		auto &active = room.join();

		const size_t chunk = 64 * 1024, total = 16 * 1024 * 1024;
		auto data = testData(total, 6);
		for (size_t pos = 0; pos < total / 2; pos += chunk) {
			REQUIRE(active.sendAll(data.data() + pos, chunk));
			REQUIRE(active.recvBytes(chunk) == data.substr(pos, chunk));
		}

		WHEN("it disconnects while the server is blocked writing to it") {
			lazy.reset();
			room.clients.erase(room.clients.begin());

			THEN("the active one is not affected") {
				for (size_t pos = total / 2; pos < total; pos += chunk) {
					REQUIRE(active.sendAll(data.data() + pos, chunk));
					REQUIRE(active.recvBytes(chunk) == data.substr(pos, chunk));
				}
			}
		}
	}
}

/*
 * Bonus: T-1000 (graceful termination on SIGTERM).
 * Hidden from the default test run, use `make test-bonus`.
 */

SCENARIO("Chat server terminates gracefully on SIGTERM", "[.bonus][sigterm]") {
	T1000 terminateAfter(slow(30000), "bonus test took too long");
	Chat server;
	Room room(server);
	const auto timeout = slow(5000);

	// A server killed by the (default) SIGTERM action did not exit by itself.
	// Under AddressSanitizer an unclean exit (i.e. memory leaks, threads
	// still running when `main` returns...) also results in a nonzero status.
	auto requireCleanExit = [&]() {
		REQUIRE(server.terminate(timeout));
		INFO("exit status: " << server.exitStatus());
		REQUIRE(server.exitedNormally());
		REQUIRE(server.exitStatus() == 0);
	};

	GIVEN("a server without any clients") {
		room.join(); // wait for the server to start
		room.clients.clear();

		THEN("it exits with status 0") {
			requireCleanExit();
		}
	}

	GIVEN("a server with some clients") {
		room.join(5);
		REQUIRE(room[0].sendAll("some data\n"));
		for (size_t i = 0; i < room.size(); i++)
			REQUIRE(room[i].recvBytes(10) == "some data\n");

		WHEN("it receives SIGTERM") {
			REQUIRE(server.running());
			kill(server.pid(), SIGTERM);
			THEN("it closes all the connections") {
				for (size_t i = 0; i < room.size(); i++) {
					INFO("client " << i);
					REQUIRE(room[i].waitForEof());
				}
			}
			THEN("it exits with status 0") {
				requireCleanExit();
			}
			THEN("it does not listen anymore") {
				REQUIRE(server.waitForExit(timeout));
				REQUIRE_THROWS_AS(ClientSocket(server.port()), std::system_error);
			}
		}
	}

	GIVEN("a server with clients that left") {
		room.join(3);
		room.clients[1].reset();
		room.clients.clear();
		room.join(2);

		THEN("it exits with status 0") {
			requireCleanExit();
		}
	}

	GIVEN("a server which is blocked writing to a client that does not read") {
		auto &lazy = room.join();
		auto &active = room.join();
		lazy.setRecvTimeout(slow(100));

		const size_t chunk = 64 * 1024, total = 16 * 1024 * 1024;
		auto data = testData(total, 8);
		for (size_t pos = 0; pos < total; pos += chunk) {
			REQUIRE(active.sendAll(data.data() + pos, chunk));
			REQUIRE(active.recvBytes(chunk) == data.substr(pos, chunk));
		}

		THEN("it still exits with status 0") {
			requireCleanExit();
		}
	}

	GIVEN("a server which is busy") {
		room.join(3);
		auto data = testData(1024 * 1024, 4);
		Sender sender(room[0], data);

		THEN("it still exits with status 0") {
			std::this_thread::sleep_for(slow(10));
			requireCleanExit();
		}
	}
}

/* vim: set sw=4 sts=4 ts=4 noet : */
