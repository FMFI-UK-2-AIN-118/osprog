#include <catch.hpp>
#include <TestHelpers.h>
#include <unistd.h>
#include <numeric>

#include "Writer.h"
#include "TestWorkerThread.h"

using Catch::Matchers::Equals;
using namespace std::chrono_literals;

SCENARIO("WriterSmall", "[writer]") {
	T1000 terminateAfter(1000ms);
	GIVEN("A Writer over a pipe with a small buffer") {
		Pipe p;
		Writer w(p.sink, 16);

		THEN("data can be added to the ring buffer") {
			REQUIRE(w.add("abcdefghijklmnopqrstuwvxyzABCDEF", 32) == 16);
		}
		GIVEN("a thread executing run") {
			auto wt = WorkerThread(w);
			std::this_thread::sleep_for(10ms); // give it time to start and process

			THEN("writer will stop when quit is called and it's waiting on the empty buffer condition") {
				wt.quitAndWait();
			}

			WHEN("data is added") {
				REQUIRE(w.add("abcd", 4) == 4);
				std::this_thread::sleep_for(10ms);

				THEN("data is written to the pipe") {
					auto str = readBytes(p.source, 4);
					REQUIRE_THAT(str, Equals("abcd"));
					std::this_thread::sleep_for(10ms);
					wt.quitAndWait();
				}
			}
		}
	}
}

SCENARIO("WriterBig", "[writer]") {
	T1000 terminateAfter(1000ms);
	INFO("A Writer over a pipe with a big buffer");
	Pipe p;
	Writer w(p.sink, 256*256);

	INFO("thread executes run")
	auto wt = WorkerThread(w);
	std::this_thread::sleep_for(10ms); // give it time to start and process

	INFO("the buffer is  filled and no new data is accepted");
	std::string bigStr(32, '\0');
	std::iota(bigStr.begin(), bigStr.end(), char('A'));
	size_t written = 0, added = 0;
	while ((added = w.add(bigStr.data(), bigStr.size())) != 0) { written += added; }
	std::this_thread::sleep_for(10ms); // give it time to fill the pipe
	while ((added = w.add(bigStr.data(), bigStr.size())) != 0) { written += added; }
	std::this_thread::sleep_for(10ms); // give it time to fill the pipe
	CAPTURE(written);

	INFO("then same data can be read from the pipe");
	auto str = readBytes(p.source, written);
	auto unexpected = readAvailable(p.source);
	CAPTURE(str.size());
	CAPTURE(unexpected.size());
	REQUIRE(str.size() == written);
	std::this_thread::sleep_for(100ms);
	wt.sendQuit();

	for (size_t checked = 0; checked < written; checked += bigStr.size()) {
		CAPTURE(checked);
		auto part = std::string(str, checked, bigStr.size());
		REQUIRE(part == std::string(bigStr, 0, part.size()));
	}
}
