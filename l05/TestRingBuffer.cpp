#include "catch.hpp"

#include "RingBuffer.h"
#include <random>
#include <algorithm>
#include <cstring>
#include <iostream>

bool beq(const char *a, const char *b, size_t s) {
	return memcmp(a, b, s) == 0;
}

SCENARIO("RingBuffer operations", "[ringbuffer]") {
	// some (pseudo) random data
//	char buf[2048];
//	std::generate_n(buf, 2048, std::mt19937{std::random_device{}()});
	const char textBuf[] = "abcdefghijklmnopqrstuvwxyzABCDEF";

	RingBuffer b(16);
	GIVEN("An empty RingBuffer") {
		REQUIRE(b.capacity() == 16);
		REQUIRE(b.size() == 0);
		REQUIRE(b.free() == 16);
		REQUIRE(b.isEmpty());
		REQUIRE(!b.isFull());

		THEN("remove(0) will do nothing") {
			REQUIRE(b.remove(0) == 0);
			REQUIRE(b.capacity() == 16);
			REQUIRE(b.size() == 0);
			REQUIRE(b.free() == 16);
			REQUIRE(b.isEmpty());
			REQUIRE(!b.isFull());
		}

		WHEN("data is added") {
			size_t added = b.add(textBuf, 5);
			THEN("sizes change accordingly") {
				CHECK(added == 5);
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 16);
				REQUIRE(b.free() == 16 - 5);
			}
			THEN("remove(0) will do nothing") {
				REQUIRE(b.remove(0) == 0);
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() == 16);
				REQUIRE(b.free() == 16 - 5);
			}
			THEN("data is contiguous") {
				REQUIRE(b.front().size == b.size());
			}
			THEN("correct data is there") {
				INFO("data  : " << std::string(b.front().data, 5));
				INFO("source: " << std::string(textBuf, 5));
				REQUIRE(beq(b.front().data, textBuf, 5));
			}
			THEN("correct data can be taken out") {
				char buf[32];
				size_t s = b.size();
				size_t taken = b.take(buf, 32);
				INFO("buf   : " << std::string(buf, s));
				INFO("source: " << std::string(textBuf, s));
				REQUIRE(taken == s);
				REQUIRE(b.size() == 0);
				REQUIRE(b.isEmpty());
				REQUIRE(beq(buf, textBuf, s));
			}
		}
		WHEN("next() is called") {
			auto ref = b.next();
			THEN("next() points to the whole buffer") {
				REQUIRE(ref.size == 16);
			}

			AND_WHEN("Data is added through it") {
				strncpy(ref.data, textBuf, 10);
				b.added(10);
				THEN("the buffer contains the new data") {
					REQUIRE(b.size() == 10);
					REQUIRE(b.front().size == 10);
					REQUIRE(beq(b.front().data, textBuf, b.front().size));
				}
			}
		}
	}

	GIVEN("A RingBuffer that will wrap") {
		const char *tbS = textBuf;
		const char *tbE = textBuf;
		auto added1 = b.add(tbE, 12); tbE += 12;
		auto removed = b.remove(8); tbS += 8;
		REQUIRE(added1 == 12);
		REQUIRE(removed == 8);
		REQUIRE(b.capacity() == 16);
		REQUIRE(b.size() == 4);
		REQUIRE(b.free() == 16 - 4);

		WHEN("data that wraps is added") {
			size_t added = b.add(tbE, 8); tbE += 8;
			THEN("sizes change accordingly") {
				CHECK(added == 8);
				REQUIRE(b.capacity() == 16);
				REQUIRE(b.size() == 12);
				REQUIRE(b.free() == 16 - 12);
			}
			THEN("data is not contiguous") {
				REQUIRE(b.front().size == 8);
				REQUIRE(b.remove(8) == 8);
				REQUIRE(b.front().size == 4);
			}
			THEN("correct data is there") {
				INFO("data  : "
					<< std::string(b.front().data, b.front().size));
				INFO("source: "
					<< std::string(tbS, b.front().size));
				REQUIRE(beq(
					b.front().data,
					tbS,
					b.front().size
					));
			THEN("correct data is after wrap")
				b.remove(8); tbS += 8;
				INFO("data  : "
					<< std::string(b.front().data, b.front().size));
				INFO("source: "
					<< std::string(tbS, b.front().size));
				REQUIRE(beq(
					b.front().data,
					tbS,
					b.front().size
				));
			}
			THEN("correct data can be taken out") {
				char buf[32];
				size_t s = b.size();
				size_t taken = b.take(buf, 32);
				INFO("buf   : " << std::string(buf, s));
				INFO("source: " << std::string(tbS, s));
				REQUIRE(taken == s);
				REQUIRE(s == size_t(tbE - tbS));
				REQUIRE(b.size() == 0);
				REQUIRE(b.isEmpty());
				REQUIRE(beq(buf, tbS, s));
			}

			AND_WHEN("removing more than size") {
				REQUIRE(b.remove(16) == 12); tbS += 12;
				THEN("buffer becomes empty") {
					REQUIRE(b.size() == 0);
					REQUIRE(b.isEmpty());
				}
			}

			AND_WHEN("appending more data") {
				REQUIRE(b.add(tbE, 4) == 4); tbE += 4;
				THEN("buffer is full") {
					REQUIRE(b.free() == 0);
					REQUIRE(b.isFull());
				}
				THEN("correct data can be taken out") {
					char buf[32];
					size_t s = b.size();
					size_t taken = b.take(buf, 32);
					INFO("buf   : " << std::string(buf, s));
					INFO("source: " << std::string(tbS, s));
					REQUIRE(taken == s);
					REQUIRE(s == size_t(tbE - tbS));
					REQUIRE(b.size() == 0);
					REQUIRE(b.isEmpty());
					REQUIRE(beq(buf, tbS, s));
				}
			}

			AND_WHEN("appending more data than free") {
				size_t added2 = b.add(textBuf + 20, 8); tbE += 4;
				THEN("only free() bytes are appended") {
					REQUIRE(added2 == 4);
				}
				THEN("data is correct") {
					char buf[32];
					size_t s = b.size();
					size_t taken = b.take(buf, 32);
					INFO("buf   : " << std::string(buf, s));
					INFO("source: " << std::string(tbS + 8, s));
					REQUIRE(taken == s);
					REQUIRE(s == size_t(tbE - tbS));
					REQUIRE(b.size() == 0);
					REQUIRE(b.isEmpty());
					REQUIRE(beq(buf, tbS, s));
				}
			}
		}
		WHEN("next() is called") {
			auto ref = b.next();
			THEN("next() points to the part at the end") {
				REQUIRE(ref.size == 16 - added1);

				AND_WHEN("it just wraps") {
					b.add("abcd", 4);
					THEN("next() points to whole empty space that's left") {
						REQUIRE(b.next().size == removed);
					}
				}
			}
		}
	}
}

/* vim: set sw=4 sts=4 ts=4 noet : */
