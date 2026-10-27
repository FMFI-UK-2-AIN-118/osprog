#include <catch.hpp>
#include <TestHelpers.h>
#include <TestNet.h>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {

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

}

SCENARIO("Echo server", "[echo]") {
	T1000 terminateAfter(10000ms, "echo test took too long");
	ServerProcess server("./echo");

	GIVEN("a client") {
		auto c = server.connect();

		WHEN("it sends a line") {
			REQUIRE(c.sendAll("hello\n"));
			THEN("it gets the same line back") {
				REQUIRE(c.recvBytes(6) == "hello\n");
			}
		}

		WHEN("it sends several lines one by one") {
			THEN("it gets each one back before it sends the next") {
				for (int i = 0; i < 10; i++) {
					std::string line = "line " + std::to_string(i) + "\n";
					REQUIRE(c.sendAll(line));
					REQUIRE(c.recvBytes(line.size()) == line);
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

		WHEN("it closes its sending side after sending data") {
			REQUIRE(c.sendAll("bye\n"));
			c.shutdownWrite();
			THEN("it gets the data and then the server closes the connection") {
				REQUIRE(c.recvUntilEof() == "bye\n");
			}
		}

		WHEN("it sends nothing and closes the sending side") {
			c.shutdownWrite();
			THEN("the server closes the connection") {
				REQUIRE(c.recvUntilEof() == "");
			}
		}
	}

	GIVEN("a client sending a lot of data") {
		auto c = server.connect();
		T1000 terminateAfter(10000ms, "big transfer took too long");
		const size_t size = 8 * 1024 * 1024;
		auto data = testData(size);

		WHEN("it sends it all (in parallel with reading) and closes its sending side") {
			std::thread sender([&]() {
				c.sendAll(data);
				c.shutdownWrite();
			});
			auto received = c.recvUntilEof();
			sender.join();

			THEN("it receives exactly the same data") {
				REQUIRE(received.size() == data.size());
				REQUIRE(received == data);
			}
		}
	}

	GIVEN("several clients connected at the same time") {
		std::vector<ClientSocket> clients;
		for (int i = 0; i < 10; i++)
			clients.push_back(server.connect());

		WHEN("they all send something") {
			for (size_t i = 0; i < clients.size(); i++)
				REQUIRE(clients[i].sendAll("client " + std::to_string(i) + "\n"));

			THEN("each one gets its own data back") {
				for (size_t i = 0; i < clients.size(); i++) {
					std::string expected = "client " + std::to_string(i) + "\n";
					REQUIRE(clients[i].recvBytes(expected.size()) == expected);
				}
			}
		}

		WHEN("one of them disconnects") {
			clients.erase(clients.begin());
			std::this_thread::sleep_for(50ms);

			THEN("the others still work") {
				for (size_t i = 0; i < clients.size(); i++) {
					REQUIRE(clients[i].sendAll("still here\n"));
					REQUIRE(clients[i].recvBytes(11) == "still here\n");
				}
			}
		}
	}

	GIVEN("a client that sends a lot of data but does not read the replies") {
		auto busy = server.connect();
		T1000 terminateAfter(10000ms, "slow reader test took too long");
		const size_t size = 16 * 1024 * 1024; // more than any socket buffers
		auto data = testData(size, 2);

		// Will block once all the buffers (client, kernel, server) are full
		// and the server (correctly) stops reading from this client.
		std::thread sender([&]() { busy.sendAll(data); });
		std::this_thread::sleep_for(200ms);

		WHEN("another client connects") {
			auto c = server.connect();

			THEN("it is still served without delay") {
				for (int i = 0; i < 5; i++) {
					std::string line = "ping " + std::to_string(i) + "\n";
					REQUIRE(c.sendAll(line));
					REQUIRE(c.recvBytes(line.size()) == line);
				}
			}
		}

		WHEN("the slow client finally starts reading") {
			auto received = busy.recvBytes(size);
			THEN("it gets all of its data back, in order") {
				REQUIRE(received.size() == data.size());
				REQUIRE(received == data);
			}
		}

		// make sure the sender is unblocked even if the checks above failed
		shutdown(busy.fd(), SHUT_RDWR);
		sender.join();
	}
}

/* vim: set sw=4 sts=4 ts=4 noet : */
