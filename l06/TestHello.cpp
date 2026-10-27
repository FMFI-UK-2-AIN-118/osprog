#include <catch.hpp>
#include <TestHelpers.h>
#include <TestNet.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

SCENARIO("Hello server", "[hello]") {
	T1000 terminateAfter(10000ms, "hello test took too long");
	ServerProcess server("./hello");

	GIVEN("a client") {
		auto c = server.connect();

		WHEN("it sends a name") {
			REQUIRE(c.sendAll("YoYo\n"));
			THEN("it gets a greeting and the connection is closed") {
				REQUIRE(c.recvUntilEof() == "Hello YoYo!\n");
			}
		}

		WHEN("it sends a name with spaces") {
			REQUIRE(c.sendAll("Alexander the Great\n"));
			THEN("the whole name is in the greeting") {
				REQUIRE(c.recvUntilEof() == "Hello Alexander the Great!\n");
			}
		}

		WHEN("it sends the name in multiple parts") {
			REQUIRE(c.sendAll("Alex"));
			std::this_thread::sleep_for(50ms);
			REQUIRE(c.sendAll("ander"));
			std::this_thread::sleep_for(50ms);
			REQUIRE(c.sendAll("\n"));
			THEN("it gets a greeting for the whole name") {
				REQUIRE(c.recvUntilEof() == "Hello Alexander!\n");
			}
		}

		WHEN("it sends more data after the newline") {
			REQUIRE(c.sendAll("YoYo\nsomething else\n"));
			THEN("only the first line is used") {
				REQUIRE(c.recvUntilEof() == "Hello YoYo!\n");
			}
		}
	}

	GIVEN("several clients connected one after another") {
		for (int i = 0; i < 5; i++) {
			auto c = server.connect();
			std::string name = "client" + std::to_string(i);
			REQUIRE(c.sendAll(name + "\n"));
			REQUIRE(c.recvUntilEof() == "Hello " + name + "!\n");
		}
	}

	GIVEN("several clients connected at the same time") {
		std::vector<ClientSocket> clients;
		for (int i = 0; i < 10; i++)
			clients.push_back(server.connect());

		WHEN("they send their names in reverse order") {
			for (int i = clients.size() - 1; i >= 0; i--)
				REQUIRE(clients[i].sendAll("client" + std::to_string(i) + "\n"));

			THEN("each one gets its own greeting") {
				for (size_t i = 0; i < clients.size(); i++)
					REQUIRE(clients[i].recvUntilEof() == "Hello client" + std::to_string(i) + "!\n");
			}
		}
	}

	GIVEN("an uncooperative client that connected, sent an incomplete name and stopped") {
		auto slow = server.connect();
		REQUIRE(slow.sendAll("first"));
		std::this_thread::sleep_for(50ms);

		WHEN("another client connects") {
			auto c = server.connect();
			REQUIRE(c.sendAll("YoYo\n"));

			THEN("it still gets a timely response") {
				REQUIRE(c.recvUntilEof() == "Hello YoYo!\n");
			}

			AND_WHEN("the uncooperative client finally finishes its name") {
				REQUIRE(slow.sendAll("last\n"));
				THEN("it gets a greeting too") {
					REQUIRE(slow.recvUntilEof() == "Hello firstlast!\n");
				}
			}
		}
	}

	GIVEN("a client that connected and sent nothing") {
		auto silent = server.connect();

		WHEN("another client connects") {
			auto c = server.connect();
			REQUIRE(c.sendAll("YoYo\n"));

			THEN("it still gets a timely response") {
				REQUIRE(c.recvUntilEof() == "Hello YoYo!\n");
			}
		}
	}
}

/* vim: set sw=4 sts=4 ts=4 noet : */
