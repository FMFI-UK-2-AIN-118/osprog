#include <catch.hpp>
#include <TestHelpers.h>
#include <chrono>
#include <numeric>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "Reader.h"
#include "TestWorkerThread.h"


using Catch::Matchers::Equals;
using namespace std::chrono_literals;

SCENARIO("Reader", "[reader]") {
	GIVEN("A reader over a pipe") {
		T1000 terminateAfter(1000ms);
		Pipe p;
		std::string str;
		auto r = Reader(p.source, [&str](const char *data, size_t size) { str.append(data, size); });

		GIVEN("a thread executing Reader::run") {
			auto rt = WorkerThread(r);

			WHEN("data is read from the pipe") {
				write(p.sink, "abcd", 4);
				std::this_thread::sleep_for(10ms); // give it time to start and process

				THEN("reader will stop when quit is received") {
					rt.sendQuit();
					std::this_thread::sleep_for(10ms); // give it some time
					// make the blocked read finish
					write(p.sink, "efgh", 4); // this should however be ignored by the reader
					{
						INFO("Waiting for Reader thread to finish");
						rt.wait();
					}
					REQUIRE_THAT(str, Equals("abcd"));
				}

				THEN("reader will stop when eof is reached") {
					close(p.sink);
					std::this_thread::sleep_for(10ms); // give it some time
					{
						INFO("Waiting for Reader thread to finish");
						rt.wait();
					}
					REQUIRE_THAT(str, Equals("abcd"));
				}
			}

			THEN("data must be read correctly") {
				auto data = std::vector<std::string>{
					"abcdefgh", "ijkl", "mnopqrst", "opq", "rstuvwxyz"
				};

				for (const auto &s : data) {
					write(p.sink, s.data(), s.size());
					std::this_thread::sleep_for(10ms);
				}
				close(p.sink);
				std::this_thread::sleep_for(10ms); // give it some time

				{
					INFO("Waiting for Reader thread to finish");
					rt.wait();
				}

				auto expected = std::accumulate(data.begin(), data.end(), std::string());
				REQUIRE_THAT(str, Equals(expected));
			}
		}
	}
}
